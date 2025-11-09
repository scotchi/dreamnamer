#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QThreadPool>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QJsonValue>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "Index.h"
#include "NoLockFactory.h"

static constexpr auto CHARACTERS_TO_STRIP = "-";
static constexpr auto MAX_RESULTS = 20;

namespace
{
    QByteArray decompress(const QByteArray &compressed)
    {
        std::stringstream input(compressed.toStdString());
        std::stringstream output;
        boost::iostreams::filtering_streambuf<boost::iostreams::input> filter;
        filter.push(boost::iostreams::gzip_decompressor());
        filter.push(input);
        boost::iostreams::copy(filter, output);

        return QByteArray::fromStdString(
            std::string(std::istreambuf_iterator<char>(output), {}));
    }
}

Index::Index(const QString &name, const QString &titleKey) :
    m_name(name),
    m_titleKey(titleKey),
    m_strippedKey(QString(titleKey).replace("original_", "stripped_")),
    m_networkManager(this),
    m_indexPath(cacheDir() + "/indexes/" + name),
    m_analyzer(Lucene::newLucene<Lucene::StandardAnalyzer>(Lucene::LuceneVersion::LUCENE_CURRENT))
{
    QDir().mkpath(m_indexPath);

    m_indexDirectory = Lucene::FSDirectory::open(
        m_indexPath.toStdWString(), Lucene::NoLockFactory::getNoLockFactory());

    if(needsSync())
    {
        sync();
    }
    else
    {
        m_reader = Lucene::IndexReader::open(m_indexDirectory);
        emit ready();
    }
}

bool Index::isReady() const
{
    return m_reader && m_indexDirectory;
}

QList<Index::Score> Index::search(const QString &file)
{
    QReadLocker locker(&m_lock);

    Q_ASSERT(m_reader);

    QList<Score> scores;

    auto searcher = Lucene::newLucene<Lucene::IndexSearcher>(m_reader);

    auto fields = Lucene::newCollection<Lucene::String>(
        m_titleKey.toStdWString(), m_strippedKey.toStdWString());
    auto parser = Lucene::newLucene<Lucene::MultiFieldQueryParser>(
        Lucene::LuceneVersion::LUCENE_CURRENT, fields, m_analyzer);

    qDebug() << query(file);

    auto q = parser->parse(query(file).toStdWString());
    auto results = searcher->search(q, MAX_RESULTS);

    for(auto score : results->scoreDocs)
    {
        auto id = m_reader->document(score->doc)->get(L"id");
        auto name = m_reader->document(score->doc)->get(m_titleKey.toStdWString());
        scores.append(Score { .id = QString::fromStdWString(id).toInt(),
                              .name = QString::fromStdWString(name),
                              .score = score->score });
    }

    return scores;
}

QString Index::cacheDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

QString Index::dumpFile() const
{
    return cacheDir() + "/" + m_name + "_ids.json.gz";
}

QString Index::query(const QString &file) const
{
    QFileInfo info(file);
    QString dirAndName = info.dir().dirName() + " " + info.completeBaseName();
    return dirAndName.replace(QRegularExpression("\\W"), " ");
}

void Index::sync(Update update)
{
    auto date = QDate::currentDate().addDays(-1);
    auto url = QString("http://files.tmdb.org/p/exports/" + m_name + "_ids_%1_%2_%3.json.gz")
               .arg(QString::number(date.month()), 2, QLatin1Char('0'))
               .arg(QString::number(date.day()), 2, QLatin1Char('0'))
               .arg(QString::number(date.year()));

    auto response = m_networkManager.get(QNetworkRequest(url));
    connect(response, &QNetworkReply::finished, this, [this, response] {
        auto data = response->readAll();

        QFile dump(dumpFile());

        dump.open(QIODevice::WriteOnly);
        dump.write(data);
        dump.close();

        build();
    });
}

bool Index::needsSync() const
{
    QDir dir(m_indexPath);

    if(!dir.exists() || dir.isEmpty())
    {
        return true;
    }

    QFileInfo info(dumpFile());

    return info.lastModified().addDays(7) < QDateTime::currentDateTime();
}

void Index::build()
{
    QThreadPool::globalInstance()->start([this] {
        QWriteLocker locker(&m_lock);

        QFile dump(dumpFile());

        if(!dump.open(QIODevice::ReadOnly))
        {
            return;
        }

        auto writer = Lucene::newLucene<Lucene::IndexWriter>(
            m_indexDirectory, m_analyzer, true,
            Lucene::IndexWriter::MaxFieldLengthLIMITED);

        writer->initialize();

        auto data = decompress(dump.readAll());

        for(auto line : data.split('\n'))
        {
            auto json = QJsonDocument::fromJson(line);
            auto id = json["id"];
            auto name = json[m_titleKey];
            auto popularity = json["popularity"];

            auto doc = Lucene::newLucene<Lucene::Document>();

            doc->add(Lucene::newLucene<Lucene::Field>(
                         Lucene::String(L"id"),
                         Lucene::String(QString::number(id.toInt()).toStdWString()),
                         Lucene::AbstractField::STORE_YES,
                         Lucene::AbstractField::INDEX_NOT_ANALYZED));

            doc->add(Lucene::newLucene<Lucene::Field>(
                         Lucene::String(m_titleKey.toStdWString()),
                         Lucene::String(name.toString().toStdWString()),
                         Lucene::AbstractField::STORE_YES,
                         Lucene::AbstractField::INDEX_ANALYZED));

            if(name.toString().contains(QRegularExpression(CHARACTERS_TO_STRIP)))
            {
                auto stripped = name.toString().replace(
                    QRegularExpression(CHARACTERS_TO_STRIP), QString());
                doc->add(Lucene::newLucene<Lucene::Field>(
                             Lucene::String(m_strippedKey.toStdWString()),
                             Lucene::String(stripped.toStdWString()),
                             Lucene::AbstractField::STORE_NO,
                             Lucene::AbstractField::INDEX_ANALYZED));
            }

            doc->setBoost(popularity.toDouble());
            writer->addDocument(doc);
        }

        writer->optimize();
        writer->close();

        m_reader = Lucene::IndexReader::open(m_indexDirectory);

        emit ready();
    });
}

QDebug &operator<<(QDebug &debug, const Index::Score &s)
{
    return debug << "{ id: " << s.id << ", name: " << s.name << ", score: " << s.score << " }";
}

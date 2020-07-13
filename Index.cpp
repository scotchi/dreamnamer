#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QThreadPool>
#include <QNetworkReply>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "Index.h"
#include "NoLockFactory.h"

static constexpr auto ORIGINAL_NAME = L"original_name";
static constexpr auto MAX_RESULTS = 10;

Index::Index() :
    m_networkManager(this),
    m_indexPath(cacheDir() + "/indexes/tv_series"),
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

    auto parser = Lucene::newLucene<Lucene::QueryParser>(
        Lucene::LuceneVersion::LUCENE_CURRENT, ORIGINAL_NAME, m_analyzer);

    qDebug() << query(file);

    auto q = parser->parse(query(file).toStdWString());
    auto results = searcher->search(q, MAX_RESULTS);

    for(auto score : results->scoreDocs)
    {
        auto name = m_reader->document(score->doc)->get(ORIGINAL_NAME);
        scores.append(QPair(QString::fromStdWString(name), score->score));
    }

    return scores;
}

QString Index::cacheDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

QString Index::dumpFile() const
{
    return cacheDir() + "/tv_series_ids.json.gz";
}

QByteArray Index::decompress(const QByteArray &compressed) const
{
    std::stringstream input(compressed.toStdString());
    std::stringstream output(compressed.toStdString());
    boost::iostreams::filtering_streambuf<boost::iostreams::input> filter;
    filter.push(boost::iostreams::gzip_decompressor());
    filter.push(input);
    boost::iostreams::copy(filter, output);

    return QByteArray::fromStdString(
        std::string(std::istreambuf_iterator<char>(output), {}));
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
    auto url = QString("http://files.tmdb.org/p/exports/tv_series_ids_%1_%2_%3.json.gz")
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

        emit status(tr("Building index of shows..."));

        auto writer = Lucene::newLucene<Lucene::IndexWriter>(
            m_indexDirectory, m_analyzer, true,
            Lucene::IndexWriter::MaxFieldLengthLIMITED);

        writer->initialize();

        auto data = decompress(dump.readAll());

        for(auto line : data.split('\n'))
        {
            auto json = QJsonDocument::fromJson(line);
            auto name = json["original_name"];
            auto popularity = json["popularity"];

            auto doc = Lucene::newLucene<Lucene::Document>();
            auto field = Lucene::newLucene<Lucene::Field>(
                Lucene::String(ORIGINAL_NAME),
                Lucene::String(
                    json[QString::fromStdWString(ORIGINAL_NAME)].toString().toStdWString()),
                Lucene::AbstractField::STORE_YES,
                Lucene::AbstractField::INDEX_ANALYZED);

            doc->setBoost(popularity.toDouble());
            doc->add(field);

            writer->addDocument(doc);
        }

        writer->optimize();
        writer->close();

        m_reader = Lucene::IndexReader::open(m_indexDirectory);

        emit status(tr("Finished"));
        emit ready();
    });
}

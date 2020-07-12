#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QThreadPool>

#include "Index.h"
#include "NoLockFactory.h"

static constexpr auto ORIGINAL_NAME = L"original_name";
static constexpr auto MAX_RESULTS = 10;

Index::Index() :
    m_indexPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
                "/indexes/tv_series"),
    m_analyzer(Lucene::newLucene<Lucene::StandardAnalyzer>(Lucene::LuceneVersion::LUCENE_CURRENT))
{
    QDir dir(m_indexPath);

    if(!dir.exists())
    {
        dir.mkpath(m_indexPath);
    }

    m_indexDirectory = Lucene::FSDirectory::open(
        m_indexPath.toStdWString(), Lucene::NoLockFactory::getNoLockFactory());

    if(dir.isEmpty())
    {
        QThreadPool::globalInstance()->start([this] {
            QWriteLocker locker(&m_lock);
            buildIndex();
        });
    }
    else
    {
        m_reader = Lucene::IndexReader::open(m_indexDirectory);
    }
}

void Index::search(const QString &file)
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

    emit done(scores);
}

QString Index::query(const QString &file) const
{
    QFileInfo info(file);
    QString dirAndName = info.dir().dirName() + " " + info.completeBaseName();
    return dirAndName.replace(QRegularExpression("\\W"), " ");
}

void Index::buildIndex()
{
    QFile file(QFileInfo(__BASE_FILE__).path() + "/tv_series_ids.json");

    if(!file.open(QIODevice::ReadOnly))
    {
        return;
    }

     emit status(tr("Building index of shows..."));

     auto writer = Lucene::newLucene<Lucene::IndexWriter>(
        m_indexDirectory, m_analyzer, true, Lucene::IndexWriter::MaxFieldLengthLIMITED);

    writer->initialize();

    for(auto line = file.readLine(); !line.isNull(); line = file.readLine())
    {
        auto json = QJsonDocument::fromJson(line);
        auto name = json["original_name"];
        auto popularity = json["popularity"];

        auto doc = Lucene::newLucene<Lucene::Document>();
        auto field = Lucene::newLucene<Lucene::Field>(
            Lucene::String(ORIGINAL_NAME),
            Lucene::String(json[QString::fromStdWString(ORIGINAL_NAME)].toString().toStdWString()),
            Lucene::AbstractField::STORE_YES,
            Lucene::AbstractField::INDEX_ANALYZED);

        doc->setBoost(popularity.toDouble());
        doc->add(field);

        writer->addDocument(doc);
    }

    writer->optimize();
    writer->close();

    emit status(tr("Finished"));

    m_reader = Lucene::IndexReader::open(m_indexDirectory);
}

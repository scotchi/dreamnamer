#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>

#include "Renamer.h"

Lucene::AnalyzerPtr Renamer::m_analyzer;
Lucene::IndexWriterPtr Renamer::m_index;

static constexpr auto ORIGINAL_NAME = L"original_name";
static constexpr auto MAX_RESULTS = 10;

Renamer::Renamer(const QString &file) :
    m_file(file)
{
    buildIndex();
    qDebug() << scores();
}

QList<Renamer::Score> Renamer::scores() const
{
    QList<Score> scores;

    auto searcher = Lucene::newLucene<Lucene::IndexSearcher>(m_index->getReader());

    auto parser = Lucene::newLucene<Lucene::QueryParser>(
        Lucene::LuceneVersion::LUCENE_CURRENT, ORIGINAL_NAME, m_analyzer);

    qDebug() << query();

    auto q = parser->parse(query().toStdWString());
    auto results = searcher->search(q, MAX_RESULTS);


    for(auto score : results->scoreDocs)
    {
        auto name = m_index->getReader()->document(score->doc)->get(ORIGINAL_NAME);
        scores.append(QPair(QString::fromStdWString(name), score->score));
    }

    return scores;
}

QString Renamer::query() const
{
    QFileInfo info(m_file);
    QString dirAndName = info.dir().dirName() + " " + info.completeBaseName();
    return dirAndName.replace(QRegularExpression("\\W"), " ");
}

void Renamer::buildIndex()
{
    if(m_index)
    {
        return;
    }

    QFile file(QFileInfo(__BASE_FILE__).path() + "/tv_series_ids.json");

    if(!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    auto dir = Lucene::newLucene<Lucene::RAMDirectory>();
    m_analyzer = Lucene::newLucene<Lucene::StandardAnalyzer>(
        Lucene::LuceneVersion::LUCENE_CURRENT);
    m_index = Lucene::newLucene<Lucene::IndexWriter>(
        dir, m_analyzer, true, Lucene::IndexWriter::MaxFieldLengthLIMITED);
    m_index->initialize();

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

        m_index->addDocument(doc);
    }
}

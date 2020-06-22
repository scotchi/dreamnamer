#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QFileInfo>

#include "Renamer.h"

Renamer::Renamer(const QStringList &files)
{
    buildIndex();
}

void Renamer::buildIndex()
{
    QFile file(QFileInfo(__BASE_FILE__).path() + "/tv_series_ids.json");

    if(!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    auto dir = Lucene::newLucene<Lucene::RAMDirectory>();
    auto analyzer = Lucene::newLucene<Lucene::StandardAnalyzer>(
        Lucene::LuceneVersion::LUCENE_CURRENT);
    m_index = Lucene::newLucene<Lucene::IndexWriter>(
        dir, analyzer, true, Lucene::IndexWriter::MaxFieldLengthLIMITED);
    m_index->initialize();

    for(auto line = file.readLine(); !line.isNull(); line = file.readLine())
    {
        auto json = QJsonDocument::fromJson(line);
        auto name = json["original_name"];
        auto popularity = json["popularity"];

        auto doc = Lucene::newLucene<Lucene::Document>();
        auto field = Lucene::newLucene<Lucene::Field>(
            Lucene::String(L"original_name"),
            Lucene::String(json["original_name"].toString().toStdWString()),
            Lucene::AbstractField::STORE_YES,
            Lucene::AbstractField::INDEX_ANALYZED);

        doc->setBoost(popularity.toDouble());
        doc->add(field);

        m_index->addDocument(doc);
    }
}

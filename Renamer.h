#pragma once

#include <QStringList>
#include "LuceneHeaders.h"

class Renamer
{
public:
    using Score = QPair<QString, double>;

    Renamer(const QString &file);
    QList<Score> scores() const;
private:
    QString query() const;
    void buildIndex();
    Lucene::AnalyzerPtr m_analyzer;
    Lucene::IndexReaderPtr m_reader;

    QString m_file;
};

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
    static void buildIndex();
    static Lucene::AnalyzerPtr m_analyzer;
    static Lucene::IndexWriterPtr m_index;

    QString m_file;
};

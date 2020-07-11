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

    QString m_indexPath;
    QString m_file;

    Lucene::DirectoryPtr m_indexDirectory;
    Lucene::AnalyzerPtr m_analyzer;
    Lucene::IndexReaderPtr m_reader;
};

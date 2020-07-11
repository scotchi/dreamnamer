#pragma once

#include <QStringList>
#include <QReadWriteLock>

#include "LuceneHeaders.h"

class Renamer : public QObject
{
    Q_OBJECT
public:
    using Score = QPair<QString, double>;

    Renamer();
    void search(const QString &file);

signals:
    void status(const QString &message);
    void done(const QList<Score> &scores);

private:
    QString query(const QString &file) const;
    void buildIndex();

    QString m_indexPath;
    QReadWriteLock m_lock;

    Lucene::DirectoryPtr m_indexDirectory;
    Lucene::AnalyzerPtr m_analyzer;
    Lucene::IndexReaderPtr m_reader;
};

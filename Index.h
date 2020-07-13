#pragma once

#include <QStringList>
#include <QReadWriteLock>
#include <QObject>
#include <QNetworkAccessManager>

#include "LuceneHeaders.h"

class Index : public QObject
{
    Q_OBJECT
public:
    enum class Update
    {
        IfNeeded,
        Force
    };

    using Score = QPair<QString, double>;

    Index();
    bool isReady() const;
    QList<Score> search(const QString &file);

signals:
    void ready();
    void status(const QString &message);

private:
    QString cacheDir() const;
    QString dumpFile() const;
    QByteArray decompress(const QByteArray &compressed) const;

    QString query(const QString &file) const;
    bool needsSync() const;
    void sync(Update update = Update::IfNeeded);
    void build();

    QNetworkAccessManager m_networkManager;
    QString m_indexPath;
    QReadWriteLock m_lock;

    Lucene::DirectoryPtr m_indexDirectory;
    Lucene::AnalyzerPtr m_analyzer;
    Lucene::IndexReaderPtr m_reader;
};
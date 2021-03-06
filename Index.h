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

    struct Score
    {
        int id = -1;
        QString name;
        double score = 0;
    };

    Index(const QString &name, const QString &titleKey);
    bool isReady() const;
    QList<Score> search(const QString &file);

signals:
    void ready();

private:
    QString cacheDir() const;
    QString dumpFile() const;

    QString query(const QString &file) const;
    bool needsSync() const;
    void sync(Update update = Update::IfNeeded);
    void build();

    QString m_name;
    QString m_titleKey;
    QString m_strippedKey;

    QNetworkAccessManager m_networkManager;
    QString m_indexPath;
    QReadWriteLock m_lock;

    Lucene::DirectoryPtr m_indexDirectory;
    Lucene::AnalyzerPtr m_analyzer;
    Lucene::IndexReaderPtr m_reader;
};

QDebug &operator<<(QDebug &debug, const Index::Score &s);

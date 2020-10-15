#pragma once

#include <QObject>
#include <QMap>
#include <QNetworkAccessManager>

class MovieDatabaseQuery : public QObject
{
    Q_OBJECT
public:
    struct MetaData
    {
        int year = 0;
    };

    MovieDatabaseQuery(const QList<int> &ids);
    void run();
signals:
    void ready();
private:
    QNetworkAccessManager m_networkManager;
    MetaData parse(const QByteArray &data) const;
    QList<int> m_ids;
    QMap<int, MetaData> m_metadata;
};

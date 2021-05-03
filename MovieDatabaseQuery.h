#pragma once

#include <QObject>
#include <QMap>
#include <QNetworkAccessManager>

#include "Episode.h"

enum class ShowType
{
    Movie,
    Series
};

class MovieDatabaseQuery : public QObject
{
    Q_OBJECT
public:
    struct MetaData
    {
        int year = 0;
    };

    using MetaDataMap = QMap<int, MetaData>;

    MovieDatabaseQuery(ShowType type, const Episode &episode, const QList<int> &ids);
    void run();
signals:
    void ready(const MetaDataMap &metaDataMap);
private:
    QNetworkAccessManager m_networkManager;
    MetaData parse(const QByteArray &data) const;
    ShowType m_type;
    Episode m_episode;
    QList<int> m_ids;
    MetaDataMap m_metaData;
};

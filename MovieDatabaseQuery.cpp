#include "MovieDatabaseQuery.h"

#include <QNetworkReply>
#include <QJsonDocument>

namespace API
{
    constexpr const char key[] = "496bde1cf55b3ea50ac092a7038def7f";
    constexpr const char url[] = "https://api.themoviedb.org/3/%1/%2?api_key=%3";
}

MovieDatabaseQuery::MovieDatabaseQuery(ShowType type, const Episode &episode, const QList<int> &ids) :
    m_type(type),
    m_episode(episode),
    m_ids(ids),
    m_networkManager(this)
{

}

void MovieDatabaseQuery::run()
{
    for(auto id : m_ids)
    {
        auto type = m_type == ShowType::Movie ? "movie" : "tv";
        auto url = QString(API::url).arg(type).arg(id).arg(API::key);

        qDebug() << "API URL:" << url;

        auto reply = m_networkManager.get(QNetworkRequest(url));

        connect(reply, &QNetworkReply::finished, this, [this, reply, id] {
            m_metaData[id] = parse(reply->readAll());

            qDebug() << id << m_metaData[id].year;

            if(m_metaData.size() == m_ids.size())
            {
                emit ready(m_metaData);
            }

            reply->deleteLater();
        });
    }
}

MovieDatabaseQuery::MetaData MovieDatabaseQuery::parse(const QByteArray &data) const
{
    MetaData metaData;

    auto doc = QJsonDocument::fromJson(data);

    if(doc.isNull())
    {
        return metaData;
    }

    auto dateField = m_type == ShowType::Movie ? "release_date" : "first_air_date";
    auto date = QDate::fromString(doc[dateField].toString(), Qt::ISODate);

    metaData.year = date.year();

    return metaData;
}

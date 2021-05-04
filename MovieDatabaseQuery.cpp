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
        getYear(id);
        getEpisode(id);
    }
}

void MovieDatabaseQuery::getYear(int id)
{
    auto type = m_type == ShowType::Movie ? "movie" : "tv";
    auto url = QString(API::url).arg(type).arg(id).arg(API::key);

    qDebug() << "API URL:" << url;

    auto reply = m_networkManager.get(QNetworkRequest(url));

    connect(reply, &QNetworkReply::finished, this, [this, reply, id] {
        m_metaData[id].year = parseYear(reply->readAll());
        qDebug() << m_metaData[id].year;
        incrementFinished();
        reply->deleteLater();
    });
}

void MovieDatabaseQuery::getEpisode(int id)
{
    if(m_type != ShowType::Series || m_episode.season == 0 || m_episode.episode == 0)
    {
        return;
    }

    auto ref = QString("%1/season/%2/episode/%3").arg(id).arg(m_episode.season).arg(m_episode.episode);
    auto url = QString(API::url).arg("tv").arg(ref).arg(API::key);

    qDebug() << "API URL:" << url;

    auto reply = m_networkManager.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, id] {
        m_metaData[id].episode = QJsonDocument::fromJson(reply->readAll())["name"].toString();
        incrementFinished();
        reply->deleteLater();
    });
}

void MovieDatabaseQuery::incrementFinished()
{
    qDebug() << "m_finishedRequests" << m_finishedRequests;

    // We do double the requests for series

    if(++m_finishedRequests >= (m_type == ShowType::Movie ? 1 : 2) * m_ids.size())
    {
        emit ready(m_metaData);
    }
}

int MovieDatabaseQuery::parseYear(const QByteArray &data) const
{
    MetaData metaData;

    auto doc = QJsonDocument::fromJson(data);

    if(doc.isNull())
    {
        return 0;
    }

    auto dateField = m_type == ShowType::Movie ? "release_date" : "first_air_date";
    auto date = QDate::fromString(doc[dateField].toString(), Qt::ISODate);

    return date.year();
}

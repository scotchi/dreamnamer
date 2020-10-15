#include "MovieDatabaseQuery.h"

#include <QNetworkReply>
#include <QJsonDocument>

namespace API
{
    constexpr const char *key = "496bde1cf55b3ea50ac092a7038def7f";
    constexpr const char *url = "https://api.themoviedb.org/3/movie/%1?api_key=%2";
}

MovieDatabaseQuery::MovieDatabaseQuery(const QList<int> &ids) : m_ids(ids), m_networkManager(this)
{

}

void MovieDatabaseQuery::run()
{
    for(auto id : m_ids)
    {
        qDebug() << "API URL:" << QString(API::url).arg(id).arg(API::key);

        auto reply = m_networkManager.get(
            QNetworkRequest(QString(API::url).arg(id).arg(API::key)));

        connect(reply, &QNetworkReply::finished, this, [this, reply, id] {
            m_metadata[id] = parse(reply->readAll());

            qDebug() << id << m_metadata[id].year;

            if(m_metadata.size() == m_ids.size())
            {
                emit ready();
            }

            reply->deleteLater();
        });
    }
}

MovieDatabaseQuery::MetaData MovieDatabaseQuery::parse(const QByteArray &data) const
{
    MetaData metaData;

    qDebug() << "PARSE:" << data;

    auto doc = QJsonDocument::fromJson(data);

    if(doc.isNull())
    {
        return metaData;
    }

    auto date = QDate::fromString(doc["release_date"].toString(), Qt::ISODate);

    metaData.year = date.year();

    return metaData;
}

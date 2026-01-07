#include "FormatSettings.h"

#include <QSettings>
#include <QRegularExpression>

FormatSettings &FormatSettings::instance()
{
    static FormatSettings settings;
    return settings;
}

FormatSettings::FormatSettings()
{
    load();
}

void FormatSettings::load()
{
    QSettings settings;

    settings.beginGroup("Format");

    int movieCount = settings.beginReadArray("movie");

    if(movieCount > 0)
    {
        for(int i = 0; i < movieCount; ++i)
        {
            settings.setArrayIndex(i);
            auto component = static_cast<FormatComponent>(settings.value("component").toInt());
            auto separator = settings.value("separator").toString();
            auto prefix = settings.value("prefix").toString();
            auto padding = settings.value("padding").toInt();
            m_movieFormat.append(FormatItem(component, separator, prefix, padding));
        }
    }
    else
    {
        m_movieFormat = defaultMovieFormat();
    }

    settings.endArray();

    int seriesCount = settings.beginReadArray("series");

    if(seriesCount > 0)
    {
        for(int i = 0; i < seriesCount; ++i)
        {
            settings.setArrayIndex(i);
            auto component = static_cast<FormatComponent>(settings.value("component").toInt());
            auto separator = settings.value("separator").toString();
            auto prefix = settings.value("prefix").toString();
            auto padding = settings.value("padding").toInt();
            m_seriesFormat.append(FormatItem(component, separator, prefix, padding));
        }
    }
    else
    {
        m_seriesFormat = defaultSeriesFormat();
    }

    settings.endArray();

    settings.endGroup();
}

void FormatSettings::save()
{
    QSettings settings;

    settings.beginGroup("Format");

    settings.beginWriteArray("movie", m_movieFormat.size());

    for(int i = 0; i < m_movieFormat.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("component", static_cast<int>(m_movieFormat[i].component));
        settings.setValue("separator", m_movieFormat[i].separator);
        settings.setValue("prefix", m_movieFormat[i].prefix);
        settings.setValue("padding", m_movieFormat[i].padding);
    }

    settings.endArray();

    settings.beginWriteArray("series", m_seriesFormat.size());

    for(int i = 0; i < m_seriesFormat.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("component", static_cast<int>(m_seriesFormat[i].component));
        settings.setValue("separator", m_seriesFormat[i].separator);
        settings.setValue("prefix", m_seriesFormat[i].prefix);
        settings.setValue("padding", m_seriesFormat[i].padding);
    }

    settings.endArray();

    settings.endGroup();
}

FormatTemplate FormatSettings::movieFormat() const
{
    return m_movieFormat;
}

FormatTemplate FormatSettings::seriesFormat() const
{
    return m_seriesFormat;
}

void FormatSettings::setMovieFormat(const FormatTemplate &format)
{
    if(m_movieFormat != format)
    {
        m_movieFormat = format;
        save();
        emit movieFormatChanged();
    }
}

void FormatSettings::setSeriesFormat(const FormatTemplate &format)
{
    if(m_seriesFormat != format)
    {
        m_seriesFormat = format;
        save();
        emit seriesFormatChanged();
    }
}

FormatTemplate FormatSettings::defaultMovieFormat()
{
    return {
        FormatItem(FormatComponent::TitleWithYear)
    };
}

FormatTemplate FormatSettings::defaultSeriesFormat()
{
    return {
        FormatItem(FormatComponent::TitleWithYear),
        FormatItem(FormatComponent::SeasonNumber, " - ", "", 1),
        FormatItem(FormatComponent::EpisodeNumber, "", "x", 2),
        FormatItem(FormatComponent::EpisodeName, " - ")
    };
}

QString FormatSettings::sanitizeFilename(const QString &text)
{
    QString result = text;

    static const QMap<QString, QString> disallowedChars = {
        { ":", " -" },
        { "\\w*:\\w*", " - " },
        { "\\w*\\/\\w*", " - " },
        { "\\w*\\\\\\w*", " - " }
    };

    for(auto [expression, replacement] : disallowedChars.asKeyValueRange())
    {
        result.replace(QRegularExpression(expression), replacement);
    }

    return result;
}

QString FormatSettings::componentValue(const FormatItem &item, const QString &title,
                                       int year, int season, int episode,
                                       const QString &episodeName) const
{
    int padding = item.padding > 0 ? item.padding : 1;

    switch(item.component)
    {
    case FormatComponent::Title:
        return title;
    case FormatComponent::TitleWithYear:
        if(year > 0)
        {
            return QString("%1 (%2)").arg(title).arg(year);
        }
        return title;
    case FormatComponent::Year:
        if(year > 0)
        {
            return QString("%1").arg(year, padding, 10, QChar('0'));
        }
        return QString();
    case FormatComponent::SeasonNumber:
        if(season > 0)
        {
            return QString("%1").arg(season, padding, 10, QChar('0'));
        }
        return QString();
    case FormatComponent::EpisodeNumber:
        if(episode > 0)
        {
            return QString("%1").arg(episode, padding, 10, QChar('0'));
        }
        return QString();
    case FormatComponent::EpisodeName:
        return episodeName;
    }
    return QString();
}

QString FormatSettings::generateMovieName(const QString &title, int year,
                                          const QString &extension) const
{
    QString result;

    for(const auto &item : m_movieFormat)
    {
        QString value = componentValue(item, title, year, 0, 0, QString());

        if(!value.isEmpty())
        {
            if(!result.isEmpty() && !item.separator.isEmpty())
            {
                result += item.separator;
            }
            result += item.prefix + value;
        }
    }

    result = sanitizeFilename(result);

    if(!extension.isEmpty())
    {
        result += "." + extension.toLower();
    }

    return result;
}

QString FormatSettings::generateSeriesName(const QString &title, int year, int season,
                                           int episode, const QString &episodeName,
                                           const QString &extension) const
{
    QString result;

    for(const auto &item : m_seriesFormat)
    {
        QString value = componentValue(item, title, year, season, episode, episodeName);

        if(!value.isEmpty())
        {
            if(!result.isEmpty() && !item.separator.isEmpty())
            {
                result += item.separator;
            }
            result += item.prefix + value;
        }
    }

    result = sanitizeFilename(result);

    if(!extension.isEmpty())
    {
        result += "." + extension.toLower();
    }

    return result;
}

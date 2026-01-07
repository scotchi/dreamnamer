#pragma once

#include <QString>
#include <QList>
#include <QPair>
#include <QVariant>

enum class FormatComponent
{
    Title,
    TitleWithYear,
    Year,
    SeasonNumber,
    EpisodeNumber,
    EpisodeName
};

struct FormatItem
{
    FormatComponent component;
    QString separator;
    QString prefix;
    int padding = 0;

    FormatItem(FormatComponent c = FormatComponent::Title,
               const QString &sep = QString(),
               const QString &pre = QString(),
               int pad = 0)
        : component(c), separator(sep), prefix(pre), padding(pad) {}

    bool operator==(const FormatItem &other) const
    {
        return component == other.component &&
               separator == other.separator &&
               prefix == other.prefix &&
               padding == other.padding;
    }
};

inline bool componentSupportsPrefix(FormatComponent component)
{
    switch(component)
    {
    case FormatComponent::SeasonNumber:
    case FormatComponent::EpisodeNumber:
        return true;
    default:
        return false;
    }
}

inline bool componentSupportsPadding(FormatComponent component)
{
    switch(component)
    {
    case FormatComponent::Year:
    case FormatComponent::SeasonNumber:
    case FormatComponent::EpisodeNumber:
        return true;
    default:
        return false;
    }
}

using FormatTemplate = QList<FormatItem>;

inline QString formatComponentName(FormatComponent component)
{
    switch(component)
    {
    case FormatComponent::Title:         return QObject::tr("Title");
    case FormatComponent::TitleWithYear: return QObject::tr("Title (Year)");
    case FormatComponent::Year:          return QObject::tr("Year");
    case FormatComponent::SeasonNumber:  return QObject::tr("Season #");
    case FormatComponent::EpisodeNumber: return QObject::tr("Episode #");
    case FormatComponent::EpisodeName:   return QObject::tr("Episode Name");
    }
    return QString();
}

inline QString formatComponentDescription(FormatComponent component)
{
    switch(component)
    {
    case FormatComponent::Title:         return QObject::tr("The movie or series title");
    case FormatComponent::TitleWithYear: return QObject::tr("Title with year in parentheses");
    case FormatComponent::Year:          return QObject::tr("Release or first air year");
    case FormatComponent::SeasonNumber:  return QObject::tr("Season number");
    case FormatComponent::EpisodeNumber: return QObject::tr("Episode number");
    case FormatComponent::EpisodeName:   return QObject::tr("Name of the episode");
    }
    return QString();
}

inline QList<FormatComponent> movieComponents()
{
    return {
        FormatComponent::Title,
        FormatComponent::TitleWithYear,
        FormatComponent::Year
    };
}

inline QList<FormatComponent> seriesComponents()
{
    return {
        FormatComponent::Title,
        FormatComponent::TitleWithYear,
        FormatComponent::Year,
        FormatComponent::SeasonNumber,
        FormatComponent::EpisodeNumber,
        FormatComponent::EpisodeName
    };
}

inline QString formatComponentMimeType()
{
    return QStringLiteral("application/x-dreamnamer-format-component");
}

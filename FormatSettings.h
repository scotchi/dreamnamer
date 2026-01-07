#pragma once

#include <QObject>
#include "FormatComponent.h"

class FormatSettings : public QObject
{
    Q_OBJECT
public:
    static FormatSettings &instance();

    FormatTemplate movieFormat() const;
    FormatTemplate seriesFormat() const;

    void setMovieFormat(const FormatTemplate &format);
    void setSeriesFormat(const FormatTemplate &format);

    // Generate filename from format template
    QString generateMovieName(const QString &title, int year, const QString &extension) const;
    QString generateSeriesName(const QString &title, int year, int season, int episode,
                               const QString &episodeName, const QString &extension) const;

    // Default formats (matching original hardcoded behavior)
    static FormatTemplate defaultMovieFormat();
    static FormatTemplate defaultSeriesFormat();

signals:
    void movieFormatChanged();
    void seriesFormatChanged();

private:
    FormatSettings();
    void load();
    void save();

    static QString sanitizeFilename(const QString &text);
    QString componentValue(const FormatItem &item, const QString &title, int year,
                           int season, int episode, const QString &episodeName) const;

    FormatTemplate m_movieFormat;
    FormatTemplate m_seriesFormat;
};

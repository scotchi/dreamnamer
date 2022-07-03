#include <QFileDialog>
#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QMessageBox>

#include "MainWindow.h"
#include "Index.h"

MainWindow::MainWindow() :
    m_overlayLabel(new QLabel(tr("Drop files here..."), this)),
    m_movieIndex("movie", "original_title"),
    m_seriesIndex("tv_series", "original_name")
{
    m_overlayLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    setupUi(this);
    reset();

    connect(renameButton, &QPushButton::clicked, this, &MainWindow::rename);
    connect(skipButton, &QPushButton::clicked, this, &MainWindow::next);
    setAcceptDrops(true);

    connect(actionOpen, &QAction::triggered, [this] {
        m_files += QFileDialog::getOpenFileNames(this);
        next();
    });

    connect(seriesListWidget, &QListWidget::itemSelectionChanged,
            this, &MainWindow::update);

    connect(movieButton, &QRadioButton::toggled, this, [this] (bool checked) {
        if(checked)
        {
            showMatches(ShowType::Movie, m_movieIndex.search(m_file));
        }
    });
    connect(seriesButton, &QRadioButton::toggled, this, [this] (bool checked) {
        if(checked)
        {
            showMatches(ShowType::Series, m_seriesIndex.search(m_file));
        }
    });

    if(!m_movieIndex.isReady() || !m_seriesIndex.isReady())
    {
        statusBar()->showMessage(tr("Building index of movies and series..."));

        auto indexFinished = [this] () {
            if(m_movieIndex.isReady() && m_seriesIndex.isReady())
            {
                statusBar()->showMessage(tr("Indexing finished."), 3000);
                next();
            }
        };

        connect(&m_movieIndex, &Index::ready, this, indexFinished);
        connect(&m_seriesIndex, &Index::ready, this, indexFinished);
    }
}

void MainWindow::addFiles(const QStringList &files)
{
    m_files += files;
    next();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    m_overlayLabel->resize(size());
}

void MainWindow::reset()
{
    Ui::MainWindow::centralWidget->hide();

    m_overlayLabel->show();
    m_overlayLabel->resize(size());

    seriesListWidget->clear();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if(!event->mimeData()->hasUrls())
    {
        return;
    }

    event->acceptProposedAction();

    QStringList files;

    for(auto url : event->mimeData()->urls())
    {
        m_files.enqueue(url.toLocalFile());
    }

    next();
}

void MainWindow::rename()
{
    auto dir = QFileInfo(m_file).dir();
    auto renamed = QFileInfo(dir, renamedLineEdit->text()).filePath();

    if(QFile::rename(m_file, renamed))
    {
        next();
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("Unable to rename file."));
    }
}

void MainWindow::next()
{
    if(!m_movieIndex.isReady() || !m_seriesIndex.isReady())
    {
        return;
    }

    if(m_files.isEmpty())
    {
        reset();
        return;
    }

    m_file = m_files.dequeue();

    QFileInfo info(m_file);

    if(info.isDir())
    {
        if(m_visited.contains(info.canonicalFilePath()))
        {
            return;
        }

        auto filter = QDir::AllEntries | QDir::NoDotAndDotDot;
        auto files = QDir(m_file).entryInfoList(filter);

        for(auto f : files)
        {
            m_files.enqueue(f.canonicalFilePath());
        }

        m_visited += info.canonicalFilePath();
        next();
        return;
    }

    if(!info.isFile() || !isVideoFile(info))
    {
        next();
        return;
    }

    fileNameLineEdit->setText(QFileInfo(m_file).fileName());

    auto movieMatches = m_movieIndex.search(m_file);
    auto seriesMatches = m_seriesIndex.search(m_file);

    if(movieMatches.isEmpty())
    {
        showMatches(ShowType::Series, seriesMatches, seriesButton);
    }
    else if(seriesMatches.isEmpty())
    {
        showMatches(ShowType::Movie, movieMatches, movieButton);
    }
    else if(movieMatches.first().score > seriesMatches.first().score)
    {
        showMatches(ShowType::Movie, movieMatches, movieButton);
    }
    else
    {
        showMatches(ShowType::Series, seriesMatches, seriesButton);
    }
}

void MainWindow::showMatches(ShowType type, const QList<Index::Score> &scores, QRadioButton *button)
{
    if(button)
    {
        button->setChecked(true);
    }

    if(scores.isEmpty())
    {
        reset();
        return;
    }

    m_overlayLabel->hide();
    Ui::MainWindow::centralWidget->show();

    QList<int> ids;

    for(auto score : scores)
    {
        ids.append(score.id);
    }

    qDebug() << ids;

    m_episodes.clear();

    auto query = new MovieDatabaseQuery(type, episode(), ids);
    connect(query, &MovieDatabaseQuery::ready, [this, query, scores] (
                const MovieDatabaseQuery::MetaDataMap &metaDataMap) {
        auto previousItem = seriesListWidget->currentItem();
        auto previous = previousItem ? previousItem->text() : QString();

        seriesListWidget->clear();

        for(const auto &score : scores)
        {
            QString name = score.name;

            if(metaDataMap.contains(score.id) && metaDataMap[score.id].year)
            {
                auto year = metaDataMap[score.id].year;
                name = QString("%1 (%2)").arg(score.name).arg(year);
            }

            seriesListWidget->addItem(name);

            // This is a bit of a hack.  I should use a real model for the list view.

            if(metaDataMap.contains(score.id) && !metaDataMap[score.id].episode.isEmpty())
            {
                m_episodes[name] = metaDataMap[score.id].episode;
            }
        }

        seriesListWidget->setCurrentItem(seriesListWidget->item(0));
        renamedLineEdit->setText(suggestedName());
        update();

        if(seriesButton->isChecked())
        {
            for(auto i = 0; i < seriesListWidget->count(); i++)
            {
                auto item = seriesListWidget->item(i);
                if(item->text() == previous)
                {
                    seriesListWidget->setCurrentItem(item);
                    break;
                }
            }
        }

        query->deleteLater();
    });

    query->run();
}

void MainWindow::update()
{
    renamedLineEdit->setText(suggestedName());
}

bool MainWindow::isVideoFile(const QFileInfo &info) const
{
    static QMimeDatabase mimeTypes;
    return mimeTypes.mimeTypeForFile(info).name().startsWith("video/");
}

Episode MainWindow::episode() const
{
    static const QStringList episodePatterns = {
        "s(\\d{1,3})\\.{0,1}e(\\d{1,4})",
        "(\\d{1,3})x(\\d{1,4})",
        "e(\\d{1,4})"
    };

    for(auto pattern : episodePatterns)
    {
        auto expression = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
        auto match = expression.match(m_file);

        if(match.hasMatch())
        {
            switch(expression.captureCount())
            {
            case 2:
                return Episode(match.captured(1).toInt(), match.captured(2).toInt());
            case 1:
                return Episode(0, match.captured(1).toInt());
            }
        }
    }

    return Episode(0, 0);
}

QString MainWindow::suggestedName() const
{
    auto cleanup = [] (auto title) {
        static const QMap<QString, QString> disallowedChars = {
            { ":", " -" },
            { "\\w*:\\w*", " - " },
            { "\\w*\\/\\w*", " - " },
            { "\\w*\\\\\\w*", " - " }
        };

        for(auto it = disallowedChars.begin(); it != disallowedChars.end(); ++it)
        {
            title.replace(QRegularExpression(it.key()), it.value());
        }

        return title;
    };

    auto title = seriesListWidget->currentItem()->text();
    auto extension = QFileInfo(m_file).suffix();

    if(seriesButton->isChecked())
    {
        auto episode = MainWindow::episode();

        if(episode.season > 0)
        {
            if(m_episodes.contains(title))
            {
                return cleanup(QString("%1 - %2x%3 - %4.%5")
                               .arg(title)
                               .arg(episode.season)
                               .arg(episode.episode, 2, 10, QChar('0'))
                               .arg(m_episodes[title])
                               .arg(extension.toLower()));
            }

            return cleanup(QString("%1 - %2x%3.%5")
                           .arg(title)
                           .arg(episode.season)
                           .arg(episode.episode, 2, 10, QChar('0'))
                           .arg(extension.toLower()));
        }
        else
        {
            if(m_episodes.contains(title))
            {
                return cleanup(QString("%1 - %3 - %4.%5")
                               .arg(title)
                               .arg(episode.episode, 2, 10, QChar('0'))
                               .arg(m_episodes[title])
                               .arg(extension.toLower()));
            }

            return cleanup(QString("%1 - %3.%5")
                           .arg(title)
                           .arg(episode.season)
                           .arg(episode.episode, 2, 10, QChar('0'))
                           .arg(extension.toLower()));

        }
    }

    return cleanup(QString("%1.%2").arg(title).arg(extension.toLower()));
}

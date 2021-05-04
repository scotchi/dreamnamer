#include <QFileDialog>
#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include <QMimeDatabase>

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

    connect(renameButton, &QPushButton::clicked, this, &MainWindow::next);
    connect(skipButton, &QPushButton::clicked, this, &MainWindow::next);
    setAcceptDrops(true);

    connect(actionOpen, &QAction::triggered, [this] {
        m_files += QFileDialog::getOpenFileNames(this);
        next();
    });

    connect(seriesListWidget, &QListWidget::itemSelectionChanged,
            this, &MainWindow::update);

    connect(&m_movieIndex, &Index::ready, this, &MainWindow::next);
    connect(&m_seriesIndex, &Index::ready, this, &MainWindow::next);

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

void MainWindow::next()
{
    if(!m_movieIndex.isReady() || !m_seriesIndex.isReady())
    {
        return;
    }

    statusBar()->showMessage(tr("Indexing finished."), 3000);

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
        renamedLineEdit->setText(renamed());
        update();

        query->deleteLater();
    });

    query->run();
}

void MainWindow::update()
{
    renamedLineEdit->setText(renamed());
}

bool MainWindow::isVideoFile(const QFileInfo &info) const
{
    static QMimeDatabase mimeTypes;
    return mimeTypes.mimeTypeForFile(info).name().startsWith("video/");
}

Episode MainWindow::episode() const
{
    QRegularExpression pattern("s(\\d{1,3})e(\\d{1,4})", QRegularExpression::CaseInsensitiveOption);
    auto match = pattern.match(m_file);
    auto season = match.captured(1).toInt();
    auto episode = match.captured(2).toInt();
    return Episode(season, episode);
}

QString MainWindow::renamed() const
{
    auto title = seriesListWidget->currentItem()->text();
    auto extension = QFileInfo(m_file).suffix();

    if(seriesButton->isChecked())
    {
        auto episode = MainWindow::episode();

        if(m_episodes.contains(title))
        {
            return QString("%1 - %2x%3 - %4.%5")
                .arg(title)
                .arg(episode.season)
                .arg(episode.episode, 2, 10, QChar('0'))
                .arg(m_episodes[title])
                .arg(extension.toLower());
        }

        return QString("%1 - %2x%3.%5")
            .arg(title)
            .arg(episode.season)
            .arg(episode.episode, 2, 10, QChar('0'))
            .arg(extension.toLower());
    }

    return QString("%1.%2").arg(title).arg(extension.toLower());
}

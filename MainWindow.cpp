#include <QFileDialog>
#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>
#include <QMimeDatabase>

#include "MainWindow.h"
#include "Index.h"

MainWindow::MainWindow() :
    m_overlayLabel(new QLabel(tr("Drop files here..."), this))
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

    connect(&m_index, &Index::done, this, &MainWindow::showMatches);
    connect(&m_index, &Index::status, this, [this] (const QString &message) {
        qDebug() << message;
        statusBar()->showMessage(message, 3000);
    });

    connect(seriesListWidget, &QListWidget::itemSelectionChanged,
            this, &MainWindow::update);
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
    if(!m_index.isReady())
    {
        connect(&m_index, &Index::ready, this, &MainWindow::next);
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

    seriesListWidget->clear();
    fileNameLineEdit->setText(QFileInfo(m_file).fileName());
    m_index.search(m_file);
}

void MainWindow::showMatches(const QList<Index::Score> &scores)
{
    qDebug() << scores;

    if(scores.isEmpty())
    {
        reset();
        return;
    }

    m_overlayLabel->hide();
    Ui::MainWindow::centralWidget->show();

    for(auto score : scores)
    {
        seriesListWidget->addItem(score.first);
    }

    seriesListWidget->setCurrentItem(seriesListWidget->item(0));
    renamedLineEdit->setText(renamed());
    update();
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

MainWindow::Episode MainWindow::episode() const
{
    QRegularExpression pattern("s(\\d{1,3})e(\\d{1,4})", QRegularExpression::CaseInsensitiveOption);
    auto match = pattern.match(m_file);
    auto season = match.captured(1).toInt();
    auto episode = match.captured(2).toInt();
    return Episode(season, episode);
}

QString MainWindow::renamed() const
{
    auto series = seriesListWidget->currentItem()->text();
    auto episode = MainWindow::episode();
    auto extension = QFileInfo(m_file).suffix();

    return QString("%1 - %2x%3.%4")
        .arg(series)
        .arg(episode.season)
        .arg(episode.episode, 2, 10, QChar('0'))
        .arg(extension.toLower());
}

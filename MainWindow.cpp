#include <QFileDialog>

#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QLabel>

#include "MainWindow.h"
#include "Renamer.h"

MainWindow::MainWindow() :
    m_overlayLabel(new QLabel(tr("Drop files here..."), this))
{
    m_overlayLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    setupUi(this);
    reset();

    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::reset);
    setAcceptDrops(true);

    connect(actionOpen, &QAction::triggered, [this] {
        query(QFileDialog::getOpenFileNames(this));
    });

    connect(&m_renamer, &Renamer::done, this, &MainWindow::showMatches);
    connect(&m_renamer, &Renamer::status, this, [this] (const QString &message) {
        qDebug() << message;
        statusBar()->showMessage(message, 3000);
    });
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
        files.append(url.toLocalFile());
    }

    query(files);
}

void MainWindow::query(const QStringList &files)
{
    for(auto file : files)
    {
        m_renamer.search(file);
    }
}


void MainWindow::showMatches(const QList<Renamer::Score> &scores)
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
}

#include <QFileDialog>

#include <QDebug>
#include <QDropEvent>
#include <QMimeData>

#include "MainWindow.h"
#include "Renamer.h"
#include "Selector.h"

MainWindow::MainWindow()
{
    setupUi(this);
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
    auto selector = new Selector(this);
    selector->add(scores);
    setCentralWidget(selector);
}

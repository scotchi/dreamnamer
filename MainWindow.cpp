#include <QFileDialog>

#include <QDebug>
#include <QDropEvent>
#include <QMimeData>

#include "MainWindow.h"
#include "Renamer.h"
#include "Selector.h"

MainWindow::MainWindow() :
    m_selector(new Selector(this))
{
    reset();

    connect(m_selector, &Selector::canceled, this, &MainWindow::reset);

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

void MainWindow::reset()
{
    m_selector->hide();
    setupUi(this);
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
    m_selector->add(scores);
    setCentralWidget(m_selector);
    m_selector->show();
}

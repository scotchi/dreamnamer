#include <QFileDialog>

#include <QDebug>
#include <QDropEvent>
#include <QMimeData>

#include "MainWindow.h"
#include "Renamer.h"

MainWindow::MainWindow()
{
    setupUi(this);
    setAcceptDrops(true);

    connect(actionOpen, &QAction::triggered, [this] {
        rename(QFileDialog::getOpenFileNames(this));
    });
}

MainWindow::~MainWindow()
{

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

    rename(files);
}

void MainWindow::rename(const QStringList &files)
{
    for(auto file : files)
    {
        Renamer renamer(file);
    }
}

// #include "MainWindow.moc"

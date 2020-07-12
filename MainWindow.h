#pragma once

#include <QQueue>

#include "ui_MainWindow.h"

#include "Renamer.h"

class QLabel;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow();
    void addFiles(const QStringList &files);
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void reset();
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void next();
    void showMatches(const QList<Renamer::Score> &scores);

    QQueue<QString> m_files;
    QLabel *m_overlayLabel = nullptr;
    Renamer m_renamer;
};

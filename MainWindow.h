#pragma once

#include "ui_MainWindow.h"

#include "Renamer.h"

class QLabel;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow();
protected:
    void resizeEvent(QResizeEvent *event) override;
private:
    void reset();
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void query(const QStringList &files);
    void showMatches(const QList<Renamer::Score> &scores);

    QLabel *m_overlayLabel = nullptr;
    Renamer m_renamer;
};

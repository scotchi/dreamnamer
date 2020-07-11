#pragma once

#include "ui_MainWindow.h"

#include "Renamer.h"

class Selector;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow();
private:
    void reset();
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void query(const QStringList &files);
    void showMatches(const QList<Renamer::Score> &scores);

    Selector *m_selector = nullptr;
    Renamer m_renamer;
};

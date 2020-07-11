#pragma once

#include "ui_MainWindow.h"

#include "Renamer.h"

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT
public:
    MainWindow();
private:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void rename(const QStringList &files);

    Renamer m_renamer;
};

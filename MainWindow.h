#pragma once

#include <QQueue>

#include "ui_MainWindow.h"

#include "Index.h"

class QLabel;
class QFileInfo;

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
    void showMatches(const QList<Index::Score> &scores);
    void update();
    bool isVideoFile(const QFileInfo &info) const;

    struct Episode
    {
        Episode(int s, int e) : season(s), episode(e) {}
        int season = 0;
        int episode = 0;
    };

    Episode episode() const;
    QString renamed() const;

    QString m_file;
    QSet<QString> m_visited;
    QQueue<QString> m_files;
    QLabel *m_overlayLabel = nullptr;
    Index m_index;
};

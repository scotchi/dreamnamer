#pragma once

#include <QQueue>

#include "ui_MainWindow.h"

#include "Index.h"
#include "Episode.h"
#include "MovieDatabaseQuery.h"

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

    void rename();
    void next();
    void showMatches(ShowType type, const QList<Index::Score> &scores,
                     QRadioButton *button = nullptr);
    void update();
    void selectItem(int id);
    bool isVideoFile(const QFileInfo &info) const;
    void performRename();
    void setPendingRename(bool pending);

    Episode episode() const;
    QString suggestedName() const;

    QString m_file;
    QSet<QString> m_visited;
    QQueue<QString> m_files;
    QLabel *m_overlayLabel = nullptr;
    QMap<QString, QString> m_episodes;
    QMap<ShowType, int> m_lastSelectedId;
    ShowType m_type = ShowType::Movie;
    Index m_movieIndex;
    Index m_seriesIndex;
    bool m_loadingMetadata = false;
    bool m_pendingRename = false;
    int m_queryId = 0;
};

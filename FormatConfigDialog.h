#pragma once

#include <QDialog>
#include <QListWidget>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

#include "FormatComponent.h"

class QPropertyAnimation;

class ComponentPalette : public QListWidget
{
    Q_OBJECT
public:
    explicit ComponentPalette(QWidget *parent = nullptr);
    void setComponents(const QList<FormatComponent> &components);

protected:
    void startDrag(Qt::DropActions supportedActions) override;
    QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override;
    QStringList mimeTypes() const override;

private:
    QPixmap createDragPixmap(QListWidgetItem *item) const;
};

class FormatItemWidget : public QFrame
{
    Q_OBJECT
public:
    FormatItemWidget(const FormatItem &item, QWidget *parent = nullptr);

    FormatItem formatItem() const;
    void setFormatItem(const FormatItem &item);

    bool isSelected() const;
    void setSelected(bool selected);

signals:
    void dragStarted(FormatItemWidget *widget);
    void clicked(FormatItemWidget *widget);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void updateDisplay();
    void updateStyle();

    FormatItem m_item;
    QLabel *m_label;
    QPoint m_dragStartPos;
    bool m_selected = false;
};

class DragFloatWidget : public QWidget
{
    Q_OBJECT
public:
    DragFloatWidget(const QPixmap &pixmap, QWidget *parent = nullptr);
    void fadeOut();

private:
    QPixmap m_pixmap;

protected:
    void paintEvent(QPaintEvent *event) override;
};

class ComponentOptionsPanel : public QFrame
{
    Q_OBJECT
public:
    explicit ComponentOptionsPanel(QWidget *parent = nullptr);

    void setFormatItem(const FormatItem &item);
    FormatItem formatItem() const;

    void clear();

signals:
    void optionsChanged();

private:
    void updateVisibility();

    FormatItem m_item;
    QLineEdit *m_separatorEdit;
    QLineEdit *m_prefixEdit;
    QComboBox *m_paddingCombo;
    QLabel *m_separatorLabel;
    QLabel *m_prefixLabel;
    QLabel *m_paddingLabel;
    QLabel *m_componentLabel;
};

class FormatArea : public QFrame
{
    Q_OBJECT
public:
    explicit FormatArea(QWidget *parent = nullptr);

    void setFormat(const FormatTemplate &format);
    FormatTemplate format() const;

    FormatItemWidget *selectedWidget() const;
    void clearSelection();
    void relayout();

signals:
    void formatChanged();
    void selectionChanged(FormatItemWidget *widget);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    FormatItemWidget *addItem(const FormatItem &item);
    void removeItem(FormatItemWidget *widget);
    void layoutItems(bool animate = true);
    int indexAt(const QPoint &pos) const;
    void startDrag(FormatItemWidget *widget);
    void selectWidget(FormatItemWidget *widget);
    void finishInternalDrag(const QPoint &globalPos);

    QList<FormatItemWidget *> m_items;
    FormatItemWidget *m_selectedWidget = nullptr;
    int m_dropIndex = -1;
    int m_dragSourceIndex = -1;
    FormatItemWidget *m_draggedWidget = nullptr;
    DragFloatWidget *m_dragFloat = nullptr;
    bool m_internalDragging = false;
    int m_spacing = 6;
    int m_padding = 8;
};

class FormatConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FormatConfigDialog(QWidget *parent = nullptr);

private slots:
    void updatePreview();
    void accept() override;
    void onMovieSelectionChanged(FormatItemWidget *widget);
    void onSeriesSelectionChanged(FormatItemWidget *widget);
    void onMovieOptionsChanged();
    void onSeriesOptionsChanged();

private:
    void setupMovieTab(QWidget *tab);
    void setupSeriesTab(QWidget *tab);
    QString generatePreview(const FormatTemplate &format, bool isMovie) const;

    ComponentPalette *m_moviePalette = nullptr;
    FormatArea *m_movieFormatArea = nullptr;
    ComponentOptionsPanel *m_movieOptions = nullptr;
    QLabel *m_moviePreview = nullptr;

    ComponentPalette *m_seriesPalette = nullptr;
    FormatArea *m_seriesFormatArea = nullptr;
    ComponentOptionsPanel *m_seriesOptions = nullptr;
    QLabel *m_seriesPreview = nullptr;
};

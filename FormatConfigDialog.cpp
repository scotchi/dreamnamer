#include "FormatConfigDialog.h"
#include "FormatSettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMimeData>
#include <QDrag>
#include <QDropEvent>
#include <QMouseEvent>
#include <QDataStream>
#include <QApplication>
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

ComponentPalette::ComponentPalette(QWidget *parent) :
    QListWidget(parent)
{
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setSpacing(4);
    setResizeMode(QListView::Adjust);
    setMaximumHeight(40);
    setStyleSheet(
        "QListWidget { background: transparent; border: none; }"
        "QListWidget::item { background: palette(mid); border-radius: 4px; padding: 4px 8px; }"
        "QListWidget::item:selected { background: palette(dark); }"
    );
}

void ComponentPalette::setComponents(const QList<FormatComponent> &components)
{
    clear();

    for(auto component : components)
    {
        auto item = new QListWidgetItem(formatComponentName(component));
        item->setData(Qt::UserRole, static_cast<int>(component));
        item->setToolTip(formatComponentDescription(component));
        addItem(item);
    }
}

void ComponentPalette::startDrag(Qt::DropActions supportedActions)
{
    auto draggedItem = currentItem();

    if(!draggedItem)
    {
        return;
    }

    auto drag = new QDrag(this);
    drag->setMimeData(mimeData({draggedItem}));
    drag->setPixmap(createDragPixmap(draggedItem));
    drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
    drag->exec(supportedActions, Qt::CopyAction);
}

QPixmap ComponentPalette::createDragPixmap(QListWidgetItem *item) const
{
    QFont font = this->font();
    QFontMetrics fm(font);
    QString text = item->text();

    int padding = 8;
    int height = fm.height() + padding * 2;
    int width = fm.horizontalAdvance(text) + padding * 2;

    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setFont(font);

    painter.setBrush(palette().mid());
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(pixmap.rect(), 4, 4);

    painter.setPen(palette().text().color());
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);

    return pixmap;
}

QStringList ComponentPalette::mimeTypes() const
{
    return { formatComponentMimeType() };
}

QMimeData *ComponentPalette::mimeData(const QList<QListWidgetItem *> &items) const
{
    if(items.isEmpty())
    {
        return nullptr;
    }

    auto mimeData = new QMimeData;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    auto component = static_cast<FormatComponent>(items.first()->data(Qt::UserRole).toInt());
    stream << static_cast<int>(component) << QString() << QString() << 0;

    mimeData->setData(formatComponentMimeType(), data);
    return mimeData;
}

DragFloatWidget::DragFloatWidget(const QPixmap &pixmap, QWidget *parent) :
    QWidget(parent),
    m_pixmap(pixmap)
{
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    int shadowSize = 8;
    setFixedSize(pixmap.size() + QSize(shadowSize * 2, shadowSize * 2));
    m_pixmap = pixmap;
}

void DragFloatWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int shadowSize = 8;
    QRect pixmapRect(shadowSize, shadowSize, m_pixmap.width(), m_pixmap.height());

    for(int i = shadowSize; i > 0; --i)
    {
        int alpha = 8 * (shadowSize - i + 1) / shadowSize;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, alpha));
        painter.drawRoundedRect(pixmapRect.adjusted(-i, -i, i, i), 4 + i, 4 + i);
    }

    painter.drawPixmap(pixmapRect.topLeft(), m_pixmap);
}

void DragFloatWidget::fadeOut()
{
    auto effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);

    auto anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(150);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::OutQuad);

    connect(anim, &QPropertyAnimation::finished, this, &QWidget::deleteLater);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

FormatItemWidget::FormatItemWidget(const FormatItem &item, QWidget *parent) :
    QFrame(parent),
    m_item(item)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);

    m_label = new QLabel;
    layout->addWidget(m_label);

    updateStyle();
    updateDisplay();
    adjustSize();
}

FormatItem FormatItemWidget::formatItem() const
{
    return m_item;
}

void FormatItemWidget::setFormatItem(const FormatItem &item)
{
    m_item = item;
    updateDisplay();
    adjustSize();
}

bool FormatItemWidget::isSelected() const
{
    return m_selected;
}

void FormatItemWidget::setSelected(bool selected)
{
    if(m_selected != selected)
    {
        m_selected = selected;
        updateStyle();
    }
}

void FormatItemWidget::updateStyle()
{
    if(m_selected)
    {
        setStyleSheet(
            "FormatItemWidget { background: palette(highlight); border-radius: 4px; "
            "border: 2px solid palette(text); }"
            "QLabel { color: palette(highlighted-text); }"
        );
    }
    else
    {
        setStyleSheet(
            "FormatItemWidget { background: palette(highlight); border-radius: 4px; "
            "border: 1px solid rgba(0, 0, 0, 0.2); }"
            "QLabel { color: palette(highlighted-text); }"
        );
    }
}

void FormatItemWidget::updateDisplay()
{
    QString text = formatComponentName(m_item.component);
    m_label->setText(text);
}

void FormatItemWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_dragStartPos = event->pos();
        emit clicked(this);
    }

    QFrame::mousePressEvent(event);
}

void FormatItemWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(!(event->buttons() & Qt::LeftButton))
    {
        return;
    }

    if((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance())
    {
        return;
    }

    emit dragStarted(this);
}

ComponentOptionsPanel::ComponentOptionsPanel(QWidget *parent) :
    QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);

    m_componentLabel = new QLabel;
    m_componentLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(m_componentLabel);

    layout->addSpacing(24);

    m_separatorLabel = new QLabel(tr("Separator:"));
    layout->addWidget(m_separatorLabel);
    m_separatorEdit = new QLineEdit;
    m_separatorEdit->setFixedWidth(70);
    m_separatorEdit->setTextMargins(4, 4, 4, 4);
    layout->addWidget(m_separatorEdit);

    layout->addSpacing(16);

    m_prefixLabel = new QLabel(tr("Prefix:"));
    layout->addWidget(m_prefixLabel);
    m_prefixEdit = new QLineEdit;
    m_prefixEdit->setFixedWidth(60);
    m_prefixEdit->setTextMargins(4, 4, 4, 4);
    layout->addWidget(m_prefixEdit);

    layout->addSpacing(16);

    m_paddingLabel = new QLabel(tr("Digits:"));
    layout->addWidget(m_paddingLabel);
    m_paddingCombo = new QComboBox;
    m_paddingCombo->addItems({"1", "2", "3", "4"});
    m_paddingCombo->setCurrentIndex(0);
    layout->addWidget(m_paddingCombo);

    // Retain space for hideable widgets
    auto retainSize = [](QWidget *w) {
        auto sp = w->sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        w->setSizePolicy(sp);
    };
    retainSize(m_prefixLabel);
    retainSize(m_prefixEdit);
    retainSize(m_paddingLabel);
    retainSize(m_paddingCombo);

    layout->addStretch();

    connect(m_separatorEdit, &QLineEdit::textChanged, this, &ComponentOptionsPanel::optionsChanged);
    connect(m_prefixEdit, &QLineEdit::textChanged, this, &ComponentOptionsPanel::optionsChanged);
    connect(m_paddingCombo, &QComboBox::currentIndexChanged, this, &ComponentOptionsPanel::optionsChanged);

    clear();
}

void ComponentOptionsPanel::setFormatItem(const FormatItem &item)
{
    m_item = item;

    m_separatorEdit->blockSignals(true);
    m_prefixEdit->blockSignals(true);
    m_paddingCombo->blockSignals(true);

    m_componentLabel->setText(formatComponentName(item.component));
    m_separatorEdit->setText(item.separator);
    m_prefixEdit->setText(item.prefix);
    int paddingIndex = (item.padding > 0 ? item.padding : 1) - 1;
    m_paddingCombo->setCurrentIndex(paddingIndex);

    m_separatorEdit->blockSignals(false);
    m_prefixEdit->blockSignals(false);
    m_paddingCombo->blockSignals(false);

    updateVisibility();
    setEnabled(true);
}

FormatItem ComponentOptionsPanel::formatItem() const
{
    FormatItem item = m_item;
    item.separator = m_separatorEdit->text();
    item.prefix = m_prefixEdit->text();
    item.padding = m_paddingCombo->currentIndex() + 1;
    return item;
}

void ComponentOptionsPanel::clear()
{
    m_item = FormatItem();
    m_componentLabel->setText(tr("Select a component"));
    m_separatorEdit->clear();
    m_prefixEdit->clear();
    m_paddingCombo->setCurrentIndex(0);
    setEnabled(false);
}

void ComponentOptionsPanel::updateVisibility()
{
    bool showPrefix = componentSupportsPrefix(m_item.component);
    bool showPadding = componentSupportsPadding(m_item.component);

    m_prefixLabel->setVisible(showPrefix);
    m_prefixEdit->setVisible(showPrefix);
    m_paddingLabel->setVisible(showPadding);
    m_paddingCombo->setVisible(showPadding);
}

FormatArea::FormatArea(QWidget *parent) :
    QFrame(parent)
{
    setAcceptDrops(true);
    setFixedHeight(46);
    setStyleSheet(
        "FormatArea { background: palette(base); border: 1px solid palette(mid); border-radius: 4px; }"
    );
}

void FormatArea::setFormat(const FormatTemplate &format)
{
    qDeleteAll(m_items);
    m_items.clear();
    m_selectedWidget = nullptr;

    for(const auto &item : format)
    {
        addItem(item);
    }

    layoutItems(false);
}

FormatTemplate FormatArea::format() const
{
    FormatTemplate result;

    for(auto widget : m_items)
    {
        if(widget != m_draggedWidget)
        {
            result.append(widget->formatItem());
        }
    }

    return result;
}

FormatItemWidget *FormatArea::selectedWidget() const
{
    return m_selectedWidget;
}

void FormatArea::clearSelection()
{
    if(m_selectedWidget)
    {
        m_selectedWidget->setSelected(false);
        m_selectedWidget = nullptr;
        emit selectionChanged(nullptr);
    }
}

void FormatArea::relayout()
{
    layoutItems(false);
}

FormatItemWidget *FormatArea::addItem(const FormatItem &item)
{
    auto widget = new FormatItemWidget(item, this);

    connect(widget, &FormatItemWidget::dragStarted, this, &FormatArea::startDrag);
    connect(widget, &FormatItemWidget::clicked, this, &FormatArea::selectWidget);

    m_items.append(widget);
    widget->show();

    return widget;
}

void FormatArea::removeItem(FormatItemWidget *widget)
{
    if(widget == m_selectedWidget)
    {
        m_selectedWidget = nullptr;
        emit selectionChanged(nullptr);
    }
    m_items.removeOne(widget);
    widget->deleteLater();
}

void FormatArea::selectWidget(FormatItemWidget *widget)
{
    if(m_selectedWidget == widget)
    {
        return;
    }

    if(m_selectedWidget)
    {
        m_selectedWidget->setSelected(false);
    }

    m_selectedWidget = widget;

    if(m_selectedWidget)
    {
        m_selectedWidget->setSelected(true);
    }

    emit selectionChanged(m_selectedWidget);
}

void FormatArea::mousePressEvent(QMouseEvent *event)
{
    if(childAt(event->pos()) == nullptr)
    {
        clearSelection();
    }

    QFrame::mousePressEvent(event);
}

void FormatArea::layoutItems(bool animate)
{
    int x = m_padding;
    int y = m_padding;
    int itemHeight = height() - m_padding * 2;

    for(int i = 0; i < m_items.size(); ++i)
    {
        auto widget = m_items[i];

        if(widget == m_draggedWidget)
        {
            continue;
        }

        if(m_dropIndex >= 0 && i == m_dropIndex)
        {
            x += 20;
        }

        QRect targetRect(x, y, widget->sizeHint().width(), itemHeight);

        if(animate && widget->pos() != targetRect.topLeft())
        {
            auto anim = new QPropertyAnimation(widget, "geometry");
            anim->setDuration(150);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->setStartValue(widget->geometry());
            anim->setEndValue(targetRect);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
        else
        {
            widget->setGeometry(targetRect);
        }

        x += widget->sizeHint().width() + m_spacing;
    }
}

int FormatArea::indexAt(const QPoint &pos) const
{
    int x = m_padding;

    for(int i = 0; i < m_items.size(); ++i)
    {
        auto widget = m_items[i];

        if(widget == m_draggedWidget)
        {
            continue;
        }

        int widgetCenter = x + widget->sizeHint().width() / 2;

        if(pos.x() < widgetCenter)
        {
            return i;
        }

        x += widget->sizeHint().width() + m_spacing;
    }

    return m_items.size();
}

void FormatArea::startDrag(FormatItemWidget *widget)
{
    m_draggedWidget = widget;
    m_dragSourceIndex = m_items.indexOf(widget);
    m_internalDragging = true;

    QPixmap pixmap(widget->size());
    pixmap.fill(Qt::transparent);
    widget->render(&pixmap);

    widget->hide();

    m_dragFloat = new DragFloatWidget(pixmap);
    QPoint globalPos = QCursor::pos();
    int shadowSize = 8;
    m_dragFloat->move(globalPos.x() - pixmap.width() / 2 - shadowSize,
                      globalPos.y() - pixmap.height() / 2 - shadowSize);
    m_dragFloat->show();

    setMouseTracking(true);
    grabMouse();
}

void FormatArea::mouseMoveEvent(QMouseEvent *event)
{
    if(m_internalDragging && m_dragFloat)
    {
        QPoint globalPos = event->globalPosition().toPoint();
        int shadowSize = 8;
        int pixmapWidth = m_dragFloat->width() - shadowSize * 2;
        int pixmapHeight = m_dragFloat->height() - shadowSize * 2;
        m_dragFloat->move(globalPos.x() - pixmapWidth / 2 - shadowSize,
                          globalPos.y() - pixmapHeight / 2 - shadowSize);

        QPoint localPos = event->pos();

        if(rect().contains(localPos))
        {
            int newIndex = indexAt(localPos);

            if(newIndex != m_dropIndex)
            {
                m_dropIndex = newIndex;
                layoutItems();
            }
        }
        else if(m_dropIndex != -1)
        {
            m_dropIndex = -1;
            layoutItems();
        }
    }

    QFrame::mouseMoveEvent(event);
}

void FormatArea::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_internalDragging)
    {
        releaseMouse();
        setMouseTracking(false);
        finishInternalDrag(event->globalPosition().toPoint());
    }

    QFrame::mouseReleaseEvent(event);
}

void FormatArea::finishInternalDrag(const QPoint &globalPos)
{
    m_internalDragging = false;

    QPoint localPos = mapFromGlobal(globalPos);
    bool insideArea = rect().contains(localPos);

    if(insideArea && m_draggedWidget)
    {
        int insertIndex = m_dropIndex >= 0 ? m_dropIndex : indexAt(localPos);
        int sourceIndex = m_items.indexOf(m_draggedWidget);

        if(sourceIndex >= 0)
        {
            m_items.removeAt(sourceIndex);

            if(sourceIndex < insertIndex)
            {
                insertIndex--;
            }
        }

        m_items.insert(insertIndex, m_draggedWidget);

        int itemHeight = height() - m_padding * 2;
        m_draggedWidget->setGeometry(
            localPos.x() - m_draggedWidget->width() / 2,
            m_padding,
            m_draggedWidget->sizeHint().width(),
            itemHeight
        );
        m_draggedWidget->show();
        m_draggedWidget = nullptr;

        if(m_dragFloat)
        {
            delete m_dragFloat;
            m_dragFloat = nullptr;
        }
    }
    else if(m_draggedWidget)
    {
        if(m_draggedWidget == m_selectedWidget)
        {
            m_selectedWidget = nullptr;
            emit selectionChanged(nullptr);
        }

        m_items.removeOne(m_draggedWidget);
        m_draggedWidget->deleteLater();
        m_draggedWidget = nullptr;

        if(m_dragFloat)
        {
            m_dragFloat->fadeOut();
            m_dragFloat = nullptr;
        }
    }

    m_dragSourceIndex = -1;
    m_dropIndex = -1;
    layoutItems();
    emit formatChanged();
}

void FormatArea::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat(formatComponentMimeType()))
    {
        event->acceptProposedAction();
    }
}

void FormatArea::dragMoveEvent(QDragMoveEvent *event)
{
    if(event->mimeData()->hasFormat(formatComponentMimeType()))
    {
        int newIndex = indexAt(event->position().toPoint());

        if(newIndex != m_dropIndex)
        {
            m_dropIndex = newIndex;
            layoutItems();
        }

        event->acceptProposedAction();
    }
}

void FormatArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    m_dropIndex = -1;
    layoutItems();
}

void FormatArea::dropEvent(QDropEvent *event)
{
    if(!event->mimeData()->hasFormat(formatComponentMimeType()))
    {
        return;
    }

    QByteArray data = event->mimeData()->data(formatComponentMimeType());
    QDataStream stream(&data, QIODevice::ReadOnly);

    int componentInt;
    QString separator, prefix;
    int padding;
    stream >> componentInt >> separator >> prefix >> padding;

    auto component = static_cast<FormatComponent>(componentInt);
    int insertIndex = m_dropIndex >= 0 ? m_dropIndex : indexAt(event->position().toPoint());

    QPoint dropPos = event->position().toPoint();

    if(m_draggedWidget)
    {
        int sourceIndex = m_items.indexOf(m_draggedWidget);

        if(sourceIndex >= 0)
        {
            m_items.removeAt(sourceIndex);

            if(sourceIndex < insertIndex)
            {
                insertIndex--;
            }
        }

        m_items.insert(insertIndex, m_draggedWidget);

        int itemHeight = height() - m_padding * 2;
        m_draggedWidget->setGeometry(
            dropPos.x() - m_draggedWidget->width() / 2,
            m_padding,
            m_draggedWidget->sizeHint().width(),
            itemHeight
        );
        m_draggedWidget->show();
        m_draggedWidget = nullptr;
    }
    else
    {
        auto widget = addItem(FormatItem(component, separator, prefix, padding));
        m_items.removeOne(widget);
        m_items.insert(insertIndex, widget);

        int itemHeight = height() - m_padding * 2;
        widget->setGeometry(
            dropPos.x() - widget->width() / 2,
            m_padding,
            widget->sizeHint().width(),
            itemHeight
        );

        selectWidget(widget);
    }

    m_dropIndex = -1;
    m_dragSourceIndex = -1;
    layoutItems();
    event->acceptProposedAction();
    emit formatChanged();
}

void FormatArea::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    layoutItems(false);
}

FormatConfigDialog::FormatConfigDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("File Naming Format"));
    setMinimumSize(720, 320);
    resize(750, 340);

    auto layout = new QVBoxLayout(this);

    auto tabWidget = new QTabWidget;
    layout->addWidget(tabWidget);

    auto movieTab = new QWidget;
    setupMovieTab(movieTab);
    tabWidget->addTab(movieTab, tr("Movie"));

    auto seriesTab = new QWidget;
    setupSeriesTab(seriesTab);
    tabWidget->addTab(seriesTab, tr("Series"));

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FormatConfigDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    m_movieFormatArea->setFormat(FormatSettings::instance().movieFormat());
    m_seriesFormatArea->setFormat(FormatSettings::instance().seriesFormat());

    updatePreview();
}

void FormatConfigDialog::setupMovieTab(QWidget *tab)
{
    auto layout = new QVBoxLayout(tab);
    layout->setSpacing(6);

    layout->addWidget(new QLabel(tr("Available components:")));

    m_moviePalette = new ComponentPalette;
    m_moviePalette->setComponents(movieComponents());
    layout->addWidget(m_moviePalette);

    layout->addWidget(new QLabel(tr("Your format:")));

    m_movieFormatArea = new FormatArea;
    layout->addWidget(m_movieFormatArea);
    connect(m_movieFormatArea, &FormatArea::formatChanged, this, &FormatConfigDialog::updatePreview);
    connect(m_movieFormatArea, &FormatArea::selectionChanged, this, &FormatConfigDialog::onMovieSelectionChanged);

    m_movieOptions = new ComponentOptionsPanel;
    layout->addWidget(m_movieOptions);
    connect(m_movieOptions, &ComponentOptionsPanel::optionsChanged, this, &FormatConfigDialog::onMovieOptionsChanged);

    layout->addStretch();

    auto previewLayout = new QHBoxLayout;
    previewLayout->addWidget(new QLabel(tr("Preview:")));
    m_moviePreview = new QLabel;
    m_moviePreview->setStyleSheet("QLabel { font-style: italic; color: palette(link); }");
    previewLayout->addWidget(m_moviePreview, 1);
    layout->addLayout(previewLayout);
}

void FormatConfigDialog::setupSeriesTab(QWidget *tab)
{
    auto layout = new QVBoxLayout(tab);
    layout->setSpacing(6);

    layout->addWidget(new QLabel(tr("Available components:")));

    m_seriesPalette = new ComponentPalette;
    m_seriesPalette->setComponents(seriesComponents());
    layout->addWidget(m_seriesPalette);

    layout->addWidget(new QLabel(tr("Your format:")));

    m_seriesFormatArea = new FormatArea;
    layout->addWidget(m_seriesFormatArea);
    connect(m_seriesFormatArea, &FormatArea::formatChanged, this, &FormatConfigDialog::updatePreview);
    connect(m_seriesFormatArea, &FormatArea::selectionChanged, this, &FormatConfigDialog::onSeriesSelectionChanged);

    m_seriesOptions = new ComponentOptionsPanel;
    layout->addWidget(m_seriesOptions);
    connect(m_seriesOptions, &ComponentOptionsPanel::optionsChanged, this, &FormatConfigDialog::onSeriesOptionsChanged);

    layout->addStretch();

    auto previewLayout = new QHBoxLayout;
    previewLayout->addWidget(new QLabel(tr("Preview:")));
    m_seriesPreview = new QLabel;
    m_seriesPreview->setStyleSheet("QLabel { font-style: italic; color: palette(link); }");
    previewLayout->addWidget(m_seriesPreview, 1);
    layout->addLayout(previewLayout);
}

void FormatConfigDialog::onMovieSelectionChanged(FormatItemWidget *widget)
{
    if(widget)
    {
        m_movieOptions->setFormatItem(widget->formatItem());
    }
    else
    {
        m_movieOptions->clear();
    }
}

void FormatConfigDialog::onSeriesSelectionChanged(FormatItemWidget *widget)
{
    if(widget)
    {
        m_seriesOptions->setFormatItem(widget->formatItem());
    }
    else
    {
        m_seriesOptions->clear();
    }
}

void FormatConfigDialog::onMovieOptionsChanged()
{
    auto widget = m_movieFormatArea->selectedWidget();

    if(widget)
    {
        widget->setFormatItem(m_movieOptions->formatItem());
        m_movieFormatArea->relayout();
        updatePreview();
    }
}

void FormatConfigDialog::onSeriesOptionsChanged()
{
    auto widget = m_seriesFormatArea->selectedWidget();

    if(widget)
    {
        widget->setFormatItem(m_seriesOptions->formatItem());
        m_seriesFormatArea->relayout();
        updatePreview();
    }
}

void FormatConfigDialog::updatePreview()
{
    if(m_moviePreview && m_movieFormatArea)
    {
        m_moviePreview->setText(generatePreview(m_movieFormatArea->format(), true));
    }

    if(m_seriesPreview && m_seriesFormatArea)
    {
        m_seriesPreview->setText(generatePreview(m_seriesFormatArea->format(), false));
    }
}

QString FormatConfigDialog::generatePreview(const FormatTemplate &format, bool isMovie) const
{
    const QString sampleTitle = "Example Show";
    const int sampleYear = 2024;
    const int sampleSeason = 1;
    const int sampleEpisode = 1;
    const QString sampleEpisodeName = "Pilot";
    const QString sampleExtension = "mkv";

    QString result;

    for(const auto &item : format)
    {
        QString value;
        int padding = item.padding > 0 ? item.padding : 1;

        switch(item.component)
        {
        case FormatComponent::Title:
            value = sampleTitle;
            break;
        case FormatComponent::TitleWithYear:
            value = QString("%1 (%2)").arg(sampleTitle).arg(sampleYear);
            break;
        case FormatComponent::Year:
            value = QString("%1").arg(sampleYear, padding, 10, QChar('0'));
            break;
        case FormatComponent::SeasonNumber:
            if(!isMovie)
            {
                value = QString("%1").arg(sampleSeason, padding, 10, QChar('0'));
            }
            break;
        case FormatComponent::EpisodeNumber:
            if(!isMovie)
            {
                value = QString("%1").arg(sampleEpisode, padding, 10, QChar('0'));
            }
            break;
        case FormatComponent::EpisodeName:
            if(!isMovie)
            {
                value = sampleEpisodeName;
            }
            break;
        }

        if(!value.isEmpty())
        {
            if(!result.isEmpty() && !item.separator.isEmpty())
            {
                result += item.separator;
            }
            result += item.prefix + value;
        }
    }

    if(!result.isEmpty())
    {
        result += "." + sampleExtension;
    }

    return result.isEmpty() ? tr("(empty format)") : result;
}

void FormatConfigDialog::accept()
{
    FormatSettings::instance().setMovieFormat(m_movieFormatArea->format());
    FormatSettings::instance().setSeriesFormat(m_seriesFormatArea->format());
    QDialog::accept();
}

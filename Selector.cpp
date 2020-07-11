#include "Selector.h"

Selector::Selector(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
}

void Selector::add(const QList<Renamer::Score> &scores)
{
    if(scores.isEmpty())
    {
        seriesListWidget->clear();
        return;
    }

    for(auto score : scores)
    {
        seriesListWidget->addItem(score.first);
    }

    seriesListWidget->setCurrentItem(seriesListWidget->item(0));
}

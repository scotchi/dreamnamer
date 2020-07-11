#pragma once

#include "Renamer.h"

#include "ui_Selector.h"

class Selector : public QWidget, private Ui::Selector
{
    Q_OBJECT
public:
    Selector(QWidget *parent);
    void add(const QList<Renamer::Score> &scores);
signals:
    void canceled();
};

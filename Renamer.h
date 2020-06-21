#pragma once

#include <QStringList>

class Renamer
{
public:
    Renamer(const QStringList &files);
private:
    void buildIndex();
};

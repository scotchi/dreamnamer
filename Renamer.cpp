#include <QDebug>

#include "Document.h"

#include "Renamer.h"

Renamer::Renamer(const QStringList &files)
{
    buildIndex();
    qDebug() << files;
}

void Renamer::buildIndex()
{

}

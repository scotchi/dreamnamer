#pragma once

#include <QStringList>
#include "LuceneHeaders.h"

class Renamer
{
public:
    Renamer(const QStringList &files);
private:
    void buildIndex();

    Lucene::IndexWriterPtr m_index;
};

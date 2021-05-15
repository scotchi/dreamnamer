#include "FileNameBuilder.h"

#include <QVariant>

FileNameBuilder::Component::Component(Type type,
                                      const QString &pattern,
                                      const QString &before) :
    Component(before, type, [&pattern](auto &value) { return pattern.arg(value.toString()); })
{

}

FileNameBuilder::Component::Component(Type type,
                                      const QString &before,
                                      std::function<QString(const QVariant &value)> renderer) :
    m_before(before),
    m_type(type),
    m_renderer(renderer)
{

}

QString FileNameBuilder::Component::render(Position position, const QVariant &value) const
{
    return (position == First) ? r() : m_before + r(value);
}

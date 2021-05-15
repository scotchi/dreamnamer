#include <QString>

class FileNameBuilder
{
public:
    class Component
    {
        enum Type
        {
            Optional,
            Required
        };

        enum Position
        {
            First,
            Middle,
            Last
        };

        Component(Type type = Required,
                  const QString &pattern = "%1",
                  const QString &before = " - ");


        Component(Type type,
                  const QString &before,
                  std::function<QString(const QVariant &value)> renderer);

        QString render(Position position, const QVariant &value) const;

    private:
        QString m_before;
        Type m_type = Required;
        std::function<QString(const QString &value)> m_renderer;
    };
};

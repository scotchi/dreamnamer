#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationDomain("dreamnamer.org");
    app.setApplicationName("Dreamnamer");

    MainWindow win;

    auto args = app.arguments();
    args.removeFirst();

    if(!args.isEmpty())
    {
        win.addFiles(args);
    }

    win.show();

    return app.exec();
}

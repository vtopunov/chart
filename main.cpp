#include "widget.h"
#include <QApplication>
#include <QDir>

#pragma warning(push)
#pragma warning(disable : 26485) 

int main(int argc, char* argv[])
{
    QApplication::addLibraryPath(QDir::currentPath());

    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}

#pragma warning(pop)

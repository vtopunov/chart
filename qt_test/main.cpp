#include "qt_test.h"
#include <QtWidgets/QApplication>

#include "../core/color.h"

int main(int argc, char *argv[])
{
    auto color = colors::black;

    QApplication a(argc, argv);
    qt_test w;
    w.show();
    return a.exec();
}

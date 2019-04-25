#include "qt_test.h"

#include <qpainter.h>

qt_test::qt_test(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

void qt_test::paintEvent(QPaintEvent *) noexcept
{
    QPainter context{ this };

    context.setPen(QColor(255, 0, 0));

    context.drawEllipse(this->rect().center(), 100, 100);
}

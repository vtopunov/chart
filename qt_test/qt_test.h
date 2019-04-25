#pragma once

#include <QtWidgets/QWidget>
#include "ui_qt_test.h"

class qt_test : public QWidget
{
    Q_OBJECT

public:
    qt_test(QWidget *parent = Q_NULLPTR);

protected:
    void paintEvent(QPaintEvent*) noexcept final;

private:
    Ui::qt_testClass ui;
};

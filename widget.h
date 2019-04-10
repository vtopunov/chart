#pragma once

#include <memory>

#include <QWidget>

namespace Ui
{
    class widgetClass;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = nullptr) noexcept;
    ~Widget();

private:
    std::unique_ptr<Ui::widgetClass> ui;
};


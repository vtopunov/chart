#include "widget.h"
#include "ui_widget.h"
#include "num_sequence.h"

#include <qmath.h>

Widget::Widget(QWidget* parent) noexcept
    : QWidget(parent)
    , ui(std::make_unique<Ui::widgetClass>())
{
    ui->setupUi(this);

    constexpr double dx = 5 * M_PI;
    constexpr num_sequence<double> x_points{ -dx, dx, 6000 };

    constexpr auto sinc = [](double x)
    {
        return (abs(x) > std::numeric_limits<double>::epsilon()) ? sin(x) / x : 1.0;
    };

    std::vector<QPointF> points;
    points.reserve(x_points.size());
    for (const auto x : x_points)
    {
        points.emplace_back(x, sinc(x));
    }

    ui->chart->figures.add(std::move(points));
}

Widget::~Widget() = default;

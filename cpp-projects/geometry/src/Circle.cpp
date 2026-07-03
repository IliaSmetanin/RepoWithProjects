#include "Circle.hpp"

#include <numbers>

double Circle::Perimeter() const {
    return 2 * std::numbers::pi * a_;
}

double Circle::Area() const {
    return std::numbers::pi * a_ * a_;
}

Shape::Figure Circle::GetFigure() const {
    return kCircle;
}

double Circle::Radius() const {
    return a_;
}
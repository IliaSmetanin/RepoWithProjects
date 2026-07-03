#pragma once

#include "Ellipse.hpp"

class Circle : public Ellipse {
public:
    Circle(const Point& center, double radius) : Ellipse(center, center, 2 * radius) {
    }

    Circle(const std::pair<const Point&, double>& pair)
        : Ellipse(pair.first, pair.first, pair.second) {
    }

    ~Circle() override {
    }

    double Perimeter() const override;

    double Area() const override;

    Figure GetFigure() const override;

    double Radius() const;
};
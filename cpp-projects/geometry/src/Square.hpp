#pragma once

#include "Circle.hpp"
#include "Rectangle.hpp"

class Square : public Rectangle {
public:
    Square(const Point& left_lower, const Point& right_higher)
        : Rectangle(left_lower, right_higher) {
    }

    ~Square() override {
    }

    double Perimeter() const override;

    double Area() const override;

    Figure GetFigure() const override;

    Circle CircumscribedCircle() const;

    Circle InscribedCircle() const;
};
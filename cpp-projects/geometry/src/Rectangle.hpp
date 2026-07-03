#pragma once

#include "Point_Vector.hpp"
#include "Polygon.hpp"

class Rectangle : public Polygon {
public:
    Rectangle(const Point& left_lower, const Point& right_higher, double ratio = 1);

    ~Rectangle() override {
    }

    double Perimeter() const override;

    double Area() const override;

    Figure GetFigure() const override;

    Point Center() const;

    std::pair<Line, Line> Diagonals() const;

protected:
    double ratio_{};
};
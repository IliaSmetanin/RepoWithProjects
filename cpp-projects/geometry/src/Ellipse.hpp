#pragma once

#include "Point_Vector.hpp"
#include "Shape.hpp"

#include <cmath>

class Ellipse : public Shape {
public:
    Ellipse() = default;

    Ellipse(const Point& f1, const Point& f2, double dist) : f1_(f1), f2_(f2), a_(dist / 2.0) {
    }

    ~Ellipse() override {
    }

    double Perimeter() const override;

    double Area() const override;

    bool operator==(const Shape& right) const override;

    bool operator==(const Ellipse& right) const;

    bool IsSimilarTo(const Shape& right) const override;

    bool IsCongruentTo(const Shape& right) const override;

    bool ContainsPoint(const Point& pt) const override;

    void Rotate(const Point& center, double angle) override;

    void Reflect(const Point& center) override;

    void Reflect(const Line& axis) override;

    void Scale(const Point& center, double coefficient) override;

    Figure GetFigure() const override;

    std::pair<Point, Point> Focuses() const;

    Point Center() const;

    double Eccentricity() const;

    std::pair<Line, Line> Directrices() const;

protected:
    bool ImpossibeCompare(const Shape& right) const;

    Point f1_{};
    Point f2_{};
    double a_{};
};
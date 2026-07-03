#pragma once

#include "Circle.hpp"
#include "Line.hpp"
#include "Polygon.hpp"

class Triangle : public Polygon {
public:
    Triangle(const std::vector<Point>& vertices) : Polygon(vertices) {
    }

    Triangle(const std::initializer_list<Point>& vertices) : Polygon(vertices) {
    }

    Triangle(const Point& a, const Point& b, const Point& c) : Polygon({a, b, c}) {
    }

    ~Triangle() override {
    }

    double Perimeter() const override;

    double Area() const override;

    Figure GetFigure() const override;

    Circle CircumscribedCircle() const;

    Circle InscribedCircle() const;

    Point Centroid() const;

    Point Orthocenter() const;

    Line EulerLine() const;

    Circle NinePointsCircle() const;

    void GetLengths(double& ab, double& bc, double& ac) const;
};
#pragma once

#include "Line.hpp"

class Shape {
public:
    enum Figure { kPol, kEll, kCircle, kRect, kSq, kTrian };

    virtual ~Shape() = default;

    virtual double Perimeter() const = 0;

    virtual double Area() const = 0;

    virtual bool operator==(const Shape& right) const = 0;

    virtual bool IsSimilarTo(const Shape& right) const = 0;

    virtual bool IsCongruentTo(const Shape& right) const = 0;

    virtual bool ContainsPoint(const Point& pt) const = 0;

    virtual void Rotate(const Point& center, double angle) = 0;

    virtual void Reflect(const Point& center) = 0;

    virtual void Reflect(const Line& axis) = 0;

    virtual void Scale(const Point& center, double coefficient) = 0;

    virtual Figure GetFigure() const = 0;
};
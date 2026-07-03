#pragma once

#include "Point_Vector.hpp"

class Line {
public:
    Line(double a, double b, double c);

    Line(const Point& left, const Point& right);

    Line(double k, double b);

    Line(const Point& pt, double k);

    Line(double a, double b, const Point& pt);

    void GetData(double& a, double& b, double& c) const;

    void Normalize();

    Line PerpendicularAtPoint(const Point& right) const;

private:
    double a_{0};  //  ax + by + c = 0
    double b_{0};
    double c_{0};
};

double Distance(const Point& pt, const Line& ln);

double Distance(const Line& ln, const Point& pt);

Point IntersectionPoint(const Line& left, const Line& right);

Point ReflectPointOverLine(const Point& pt, const Line& axis);

bool operator==(const Line& left, const Line& right);

bool operator!=(const Line& left, const Line& right);
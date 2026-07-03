#include "Line.hpp"

#include <cmath>

Line::Line(double a, double b, double c) : a_(a), b_(b), c_(c) {
    Normalize();
}

Line::Line(const Point& left, const Point& right) {
    if (std::abs(left.x - right.x) < kEpsilon) {
        a_ = 1.0;
        b_ = 0.0;
        c_ = -left.x;
        return;
    }
    if (std::abs(left.y - right.y) < kEpsilon) {
        a_ = 0.0;
        b_ = 1.0;
        c_ = -left.y;
        return;
    }
    a_ = right.y - left.y;
    b_ = left.x - right.x;
    c_ = right.x * left.y - left.x * right.y;
    Normalize();
}

Line::Line(double k, double b) : a_(k), b_(-1.0), c_(b) {
    Normalize();
}

Line::Line(const Point& pt, double k) : a_(k), b_(-1.0), c_(pt.y - k * pt.x) {
    Normalize();
}

Line::Line(double a, double b, const Point& pt) : a_(a), b_(b) {
    c_ = (-a * pt.x) - (b * pt.y);
    Normalize();
}

void Line::GetData(double& a, double& b, double& c) const {
    a = a_;
    b = b_;
    c = c_;
}

void Line::Normalize() {
    double normalizer = sqrt(a_ * a_ + b_ * b_);
    a_ /= normalizer;
    b_ /= normalizer;
    c_ /= normalizer;
    if (a_ < 0 || ((std::abs(a_) < kEpsilon && b_ < 0))) {
        a_ = -a_;
        b_ = -b_;
        c_ = -c_;
    }
    if (std::abs(a_) < kEpsilon) {
        a_ = 0.0;
    }
    if (std::abs(b_) < kEpsilon) {
        b_ = 0.0;
    }
    if (std::abs(c_) < kEpsilon) {
        c_ = 0.0;
    }
}

double Distance(const Point& pt, const Line& ln) {
    double a, b, c;
    ln.GetData(a, b, c);

    double x0{pt.x};
    double y0{pt.y};

    double numerator = std::abs(a * x0 + b * y0 + c);
    double denominator = sqrt(a * a + b * b);

    return numerator / denominator;
}

double Distance(const Line& ln, const Point& pt) {
    return Distance(pt, ln);
}

Line Line::PerpendicularAtPoint(const Point& right) const {
    return Line(b_, -a_, right);
}

Point IntersectionPoint(const Line& left, const Line& right) {
    double a1, b1, c1;
    double a2, b2, c2;
    left.GetData(a1, b1, c1);
    right.GetData(a2, b2, c2);

    return {(b1 * c2 - b2 * c1) / (a1 * b2 - a2 * b1), (a2 * c1 - a1 * c2) / (a1 * b2 - a2 * b1)};
}

Point ReflectPointOverLine(const Point& pt, const Line& axis) {
    Line perpendicular = axis.PerpendicularAtPoint(pt);
    Point intersection = IntersectionPoint(axis, perpendicular);
    return Point(intersection * 2.0 - pt);
}

bool operator==(const Line& left, const Line& right) {
    double left_a, left_b, left_c, right_a, right_b, right_c;
    left.GetData(left_a, left_b, left_c);
    right.GetData(right_a, right_b, right_c);
    return (std::abs(left_a - right_a) >= kEpsilon)   ? false
           : (std::abs(left_b - right_b) >= kEpsilon) ? false
           : (std::abs(left_c - right_c) >= kEpsilon) ? false
                                                      : true;
}

bool operator!=(const Line& left, const Line& right) {
    return !(left == right);
}

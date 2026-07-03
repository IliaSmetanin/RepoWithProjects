#include "Ellipse.hpp"

#include <numbers>

double Ellipse::Perimeter() const {
    return 4 * a_ * std::comp_ellint_2(Eccentricity());
}

double Ellipse::Area() const {
    double e = Eccentricity();
    double b = a_ * sqrt(1 - e * e);
    return std::numbers::pi * a_ * b;
}

bool Ellipse::operator==(const Shape& right) const {
    if (ImpossibeCompare(right)) {
        return false;
    }
    const Ellipse& right_ell = dynamic_cast<const Ellipse&>(right);
    if (((f1_ == right_ell.f1_ && f2_ == right_ell.f2_) ||
         (f1_ == right_ell.f2_ && f2_ == right_ell.f1_)) &&
        std::abs(a_ - right_ell.a_) < kEpsilon) {
        return true;
    }
    return false;
}

bool Ellipse::operator==(const Ellipse& right) const {
    return operator==(static_cast<const Shape&>(right));
}

bool Ellipse::IsSimilarTo(const Shape& right) const {
    if (ImpossibeCompare(right)) {
        return false;
    }
    const Ellipse& right_ell = dynamic_cast<const Ellipse&>(right);
    if (std::abs(Eccentricity() - right_ell.Eccentricity()) < kEpsilon) {
        return true;
    }
    return false;
}

bool Ellipse::IsCongruentTo(const Shape& right) const {
    if (ImpossibeCompare(right)) {
        return false;
    }
    const Ellipse& right_ell = dynamic_cast<const Ellipse&>(right);
    if (std::abs(Eccentricity() - right_ell.Eccentricity()) < kEpsilon &&
        std::abs(a_ - right_ell.a_) < kEpsilon) {
        return true;
    }
    return false;
}

bool Ellipse::ContainsPoint(const Point& pt) const {
    double d1 = Distance(pt, f1_);
    double d2 = Distance(pt, f2_);
    return d1 + d2 <= 2.0 * a_ + kEpsilon;
}

void Ellipse::Rotate(const Point& center, double angle) {
    double rad_angle = angle * std::numbers::pi / 180.0;
    f1_ = Point((f1_ - center).Rotate(rad_angle) + center);
    f2_ = Point((f2_ - center).Rotate(rad_angle) + center);
}

void Ellipse::Reflect(const Point& center) {
    f1_ = Point(center * 2.0 - f1_);
    f2_ = Point(center * 2.0 - f2_);
}

void Ellipse::Reflect(const Line& axis) {
    f1_ = ReflectPointOverLine(f1_, axis);
    f2_ = ReflectPointOverLine(f2_, axis);
}

void Ellipse::Scale(const Point& center, double coefficient) {
    f1_ = Point(center + (f1_ - center) * coefficient);
    f2_ = Point(center + (f2_ - center) * coefficient);
    a_ *= std::abs(coefficient);
}

Shape::Figure Ellipse::GetFigure() const {
    return kEll;
}

std::pair<Point, Point> Ellipse::Focuses() const {
    return {f1_, f2_};
}

Point Ellipse::Center() const {
    return {(f1_.x + f2_.x) / 2.0, (f1_.y + f2_.y) / 2.0};
}

double Ellipse::Eccentricity() const {
    double c = Distance(f1_, f2_) / 2.0;
    return c / a_;
}

std::pair<Line, Line> Ellipse::Directrices() const {
    Line f1_f2(f1_, f2_);
    Point center = Center();
    double e = Eccentricity();
    double p = a_ / e;
    double a, b, c;
    f1_f2.GetData(a, b, c);
    double d0 = (-b) * center.x + a * center.y;
    double d1 = -d0 + p;
    double d2 = -d0 - p;
    return {Line(-b, a, d1), Line(-b, a, d2)};
}

bool Ellipse::ImpossibeCompare(const Shape& right) const {
    Figure right_type = right.GetFigure();
    if (right_type != kEll && right_type != kCircle) {
        return true;
    }
    return false;
}
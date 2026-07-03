#include "Rectangle.hpp"

Rectangle::Rectangle(const Point& left_lower, const Point& right_higher, double ratio)
    : ratio_(ratio) {
    vertices_.push_back(left_lower);
    double a, b;
    double bh;
    double ah;
    double hypot = Distance(left_lower, right_higher);
    if (ratio_ < 1) {
        b = hypot / sqrt(ratio_ * ratio_ + 1.0);
        a = b * ratio_;
        bh = a / sqrt(ratio_ * ratio_ + 1.0);
        ah = bh * ratio_;
    } else {
        a = hypot / sqrt(ratio_ * ratio_ + 1.0);
        ah = a / sqrt(ratio_ * ratio_ + 1.0);
        bh = ah * ratio_;
    }
    Vector ac(left_lower, right_higher);
    Vector direct = ac.GetDirect();
    Vector left_normal = ac.GetNormalLeft();  //  для точки B
    //  идем до точки B через катет(direct * ah), лежащий на диагонали, и высоту(normal * hb)
    Vector ob = left_lower + direct * ah + left_normal * bh;
    vertices_.push_back(Point(ob));
    vertices_.push_back(right_higher);
    Vector right_normal = ac.GetNormalRight();  //  для точки D
    Vector od = right_higher - direct * ah + right_normal * bh;
    vertices_.push_back(Point(od));
}

double Rectangle::Perimeter() const {
    double d = Distance(vertices_[0], vertices_[2]);
    double a = d / sqrt(ratio_ * ratio_ + 1);
    return 2.0 * (a + sqrt(d * d - a * a));
}

double Rectangle::Area() const {
    double d = Distance(vertices_[0], vertices_[2]);
    double a = d / sqrt(ratio_ * ratio_ + 1);
    return a * a * ratio_;
}

Shape::Figure Rectangle::GetFigure() const {
    return kRect;
}

Point Rectangle::Center() const {
    return {(vertices_[0].x + vertices_[2].x) / 2, (vertices_[0].y + vertices_[2].y) / 2};
}

std::pair<Line, Line> Rectangle::Diagonals() const {
    return {Line(vertices_[0], vertices_[2]), Line(vertices_[1], vertices_[3])};
}
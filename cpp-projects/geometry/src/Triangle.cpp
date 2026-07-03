#include "Triangle.hpp"

double Triangle::Perimeter() const {
    double ab, bc, ac;
    GetLengths(ab, bc, ac);
    return ab + bc + ac;
}

double Triangle::Area() const {
    double ab, bc, ac;
    GetLengths(ab, bc, ac);
    double p = (ab + bc + ac) / 2.0;
    return sqrt(p * (p - ab) * (p - bc) * (p - ac));
}

Shape::Figure Triangle::GetFigure() const {
    return kTrian;
}

Circle Triangle::CircumscribedCircle() const {
    Point ab_mid((Vector(vertices_[0]) + Vector(vertices_[1])) / 2.0);
    Point bc_mid((Vector(vertices_[1]) + Vector(vertices_[2])) / 2.0);
    Line ab(vertices_[0], vertices_[1]);
    Line bc(vertices_[1], vertices_[2]);
    Line ab_mid_perpend = ab.PerpendicularAtPoint(ab_mid);
    Line bc_mid_perpend = bc.PerpendicularAtPoint(bc_mid);
    Point circle_center = IntersectionPoint(ab_mid_perpend, bc_mid_perpend);
    double radius = Distance(circle_center, vertices_[0]);
    return Circle(circle_center, radius);
}

Circle Triangle::InscribedCircle() const {  //  свойство биссектрисы треугольника
    double ab, bc, ac;
    GetLengths(ab, bc, ac);
    double ratio1 = ab / ac;
    double ratio2 = ab / bc;
    Point pt1((Vector(vertices_[1]) + ratio1 * Vector(vertices_[2])) / (1.0 + ratio1));
    Point pt2((Vector(vertices_[0]) + ratio2 * Vector(vertices_[2])) / (1.0 + ratio2));
    Line bisector1(vertices_[0], pt1);
    Line bisector2(vertices_[1], pt2);
    Point circle_center = IntersectionPoint(bisector1, bisector2);
    double radius = Distance(circle_center, Line(vertices_[0], vertices_[1]));
    return Circle(circle_center, radius);
}

Point Triangle::Centroid() const {
    return Point((Vector(vertices_[0]) + Vector(vertices_[1]) + Vector(vertices_[2])) / 3.0);
}

Point Triangle::Orthocenter() const {
    Line bc(vertices_[1], vertices_[2]);
    Line ac(vertices_[0], vertices_[2]);
    Line ah1 = bc.PerpendicularAtPoint((vertices_[0]));
    Line bh2 = ac.PerpendicularAtPoint((vertices_[1]));
    return IntersectionPoint(ah1, bh2);
}

Line Triangle::EulerLine() const {
    Point centroid = Centroid();
    Point orthocenter = Orthocenter();
    if (centroid == orthocenter) {
        return Line(centroid, vertices_[0]);
    }
    return Line(centroid, orthocenter);
}

Circle Triangle::NinePointsCircle() const {
    Point ab_mid((Vector(vertices_[0]) + Vector(vertices_[1])) / 2.0);
    Point bc_mid((Vector(vertices_[1]) + Vector(vertices_[2])) / 2.0);
    Point ac_mid((Vector(vertices_[0]) + Vector(vertices_[2])) / 2.0);
    Vector v1(ab_mid, bc_mid);
    Vector v2(ab_mid, ac_mid);
    double cross = v1.x_ * v2.y_ - v1.y_ * v2.x_;
    if (std::abs(cross) < kEpsilon) {
        Point circumcenter = CircumscribedCircle().Center();
        Point orthocenter = Orthocenter();
        Point center((circumcenter.x + orthocenter.x) / 2.0,
                     (circumcenter.y + orthocenter.y) / 2.0);
        double radius = Distance(center, ab_mid);
        return Circle(center, radius);
    }
    Triangle tr({ab_mid, bc_mid, ac_mid});
    return tr.CircumscribedCircle();
}

void Triangle::GetLengths(double& ab, double& bc, double& ac) const {
    ab = Distance(vertices_[0], vertices_[1]);
    bc = Distance(vertices_[1], vertices_[2]);
    ac = Distance(vertices_[0], vertices_[2]);
}
#include "Square.hpp"

double Square::Perimeter() const {
    double d = Distance(vertices_[0], vertices_[2]);
    return sqrt(8) * d;
}

double Square::Area() const {
    double d = Distance(vertices_[0], vertices_[2]);
    return d * d / 2.0;
}

Shape::Figure Square::GetFigure() const {
    return kSq;
}

Circle Square::CircumscribedCircle() const {
    return Circle(Center(), Distance(vertices_[0], vertices_[2]) / 2.0);
}

Circle Square::InscribedCircle() const {
    return Circle(Center(), Distance(vertices_[0], vertices_[2]) / sqrt(8.0));
}
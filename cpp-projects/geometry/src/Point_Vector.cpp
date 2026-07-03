#include "Point_Vector.hpp"

#include <cmath>

double Distance(const Point& left, const Point& right) {
    return sqrt((left.x - right.x) * (left.x - right.x) + (left.y - right.y) * (left.y - right.y));
}

bool operator==(const Point& left, const Point& right) {
    return (std::abs(left.x - right.x) >= kEpsilon)   ? false
           : (std::abs(left.y - right.y) >= kEpsilon) ? false
                                                      : true;
}

bool operator!=(const Point& left, const Point& right) {
    return !(left == right);
}

Vector& Vector::operator+=(const Vector& right) {
    x_ += right.x_;
    y_ += right.y_;
    return *this;
}

Vector& Vector::operator-=(const Vector& right) {
    x_ -= right.x_;
    y_ -= right.y_;
    return *this;
}

Vector& Vector::operator*=(double right) {
    x_ *= right;
    y_ *= right;
    return *this;
}

Vector& Vector::operator/=(double right) {
    x_ /= right;
    y_ /= right;
    return *this;
}

double Vector::GetAngle(const Vector& left, const Vector& right) {
    return (left.x_ * right.x_ + left.y_ * right.y_) /
           sqrt((left.x_ * left.x_ + left.y_ * left.y_) *
                (right.x_ * right.x_ + right.y_ * right.y_));
}

Vector& Vector::Rotate(double rad_angle) {
    using std::cos, std::sin;
    double new_x = x_ * cos(rad_angle) - y_ * sin(rad_angle);
    double new_y = x_ * sin(rad_angle) + y_ * cos(rad_angle);
    x_ = new_x;
    y_ = new_y;
    return *this;
}

double Vector::Length() const {
    return sqrt(x_ * x_ + y_ * y_);
}

Vector Vector::GetDirect() const {
    double len = Length();
    return Vector(x_ / len, y_ / len);
}

Vector Vector::GetNormalLeft() const {
    double len = Length();
    return Vector(-y_ / len, x_ / len);
}

Vector Vector::GetNormalRight() const {
    double len = Length();
    return Vector(y_ / len, -x_ / len);
}

Vector::operator Point() const {
    return {x_, y_};
}

Vector operator+(const Vector& left, const Vector& right) {
    Vector temp = left;
    temp += right;
    return temp;
}

Vector operator-(const Vector& left, const Vector& right) {
    Vector temp = left;
    temp -= right;
    return temp;
}

Vector operator*(const Vector& left, double right) {
    Vector temp = left;
    temp *= right;
    return temp;
}

Vector operator*(double left, const Vector& right) {
    return right * left;
}

Vector operator/(const Vector& left, double right) {
    Vector temp = left;
    temp /= right;
    return temp;
}

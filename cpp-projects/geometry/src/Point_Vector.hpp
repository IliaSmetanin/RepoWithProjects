#pragma once

const double kEpsilon = 1e-7;

struct Point {
public:
    Point(double x = 0, double y = 0) : x(x), y(y) {
    }

    double x{0};
    double y{0};
};

double Distance(const Point& left, const Point& right);

bool operator==(const Point& left, const Point& right);

bool operator!=(const Point& left, const Point& right);

class Vector {
public:
    Vector(double x = 0, double y = 0) : x_(x), y_(y) {
    }

    Vector(const Point& pt) : x_(pt.x), y_(pt.y) {
    }

    Vector(const Point& begin, const Point& end) : x_(end.x - begin.x), y_(end.y - begin.y) {
    }

    Vector& operator+=(const Vector& right);

    Vector& operator-=(const Vector& right);

    Vector& operator*=(double right);

    Vector& operator/=(double right);

    static double GetAngle(const Vector& left, const Vector& right);

    Vector& Rotate(double rad_angle);

    double Length() const;

    Vector GetDirect() const;

    Vector GetNormalLeft() const;

    Vector GetNormalRight() const;

    explicit operator Point() const;

    double x_{0};
    double y_{0};
};

Vector operator+(const Vector& left, const Vector& right);

Vector operator-(const Vector& left, const Vector& right);

Vector operator*(const Vector& left, double right);

Vector operator*(double left, const Vector& right);

Vector operator/(const Vector& left, double right);
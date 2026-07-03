#pragma once

#include "Line.hpp"
#include "Point_Vector.hpp"
#include "Shape.hpp"

#include <cmath>
#include <vector>

class Polygon : public Shape {
public:
    Polygon() = default;

    Polygon(const std::vector<Point>& vertices) : vertices_(vertices) {
    }

    Polygon(const std::initializer_list<Point>& vertices) : vertices_(vertices) {
    }

    ~Polygon() override {
    }

    double Perimeter() const override;

    double Area() const override;

    bool operator==(const Shape& right) const override;

    bool operator==(const Polygon& right) const;

    bool IsSimilarTo(const Shape& right) const override;

    bool IsCongruentTo(const Shape& right) const override;

    bool ContainsPoint(const Point& pt) const override;

    void Rotate(const Point& center, double angle) override;

    void Reflect(const Point& center) override;

    void Reflect(const Line& axis) override;

    void Scale(const Point& center, double coefficient) override;

    Figure GetFigure() const override;

    size_t VerticesCount() const;

    const std::vector<Point>& GetVertices() const;

    bool IsConvex() const;

protected:
    static bool SidesSimilaritry(const std::vector<double>& left, const std::vector<double>& right,
                                 double& ratio, size_t entry_pos);

    template <typename T>
    static bool VectorsEquality(const std::vector<T>& left, const std::vector<T>& right,
                                bool (*equal)(T a, T b), size_t& entry_pos);

    bool IsPointOnEdge(const Point& p1, const Point& p2, const Point& pt) const;

    std::vector<double> GetSides() const;

    std::vector<double> GetAngles() const;

    bool CheckSimilarity(const Shape& right, double& ratio) const;

    bool ImpossibleCompare(const Shape& right) const;

    bool DefineTurn(const Point& a, const Point& b, const Point& c) const;

    std::vector<Point> vertices_{};
};
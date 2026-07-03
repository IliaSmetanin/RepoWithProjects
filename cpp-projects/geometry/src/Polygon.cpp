#include "Polygon.hpp"

#include <numbers>

double Polygon::Perimeter() const {
    size_t vert_count = VerticesCount();
    double perimeter = 0;
    for (size_t i = 0; i < vert_count - 1; ++i) {
        perimeter += Distance(vertices_[i], vertices_[i + 1]);
    }
    perimeter += Distance(vertices_[0], vertices_[vert_count - 1]);
    return perimeter;
}

double Polygon::Area() const {
    size_t vert_count = VerticesCount();
    if (vert_count < 3) {
        return 0;
    }
    double sum = 0;
    for (size_t i = 0; i < vert_count - 1; ++i) {
        sum += vertices_[i].x * vertices_[i + 1].y - vertices_[i + 1].x * vertices_[i].y;
    }
    sum +=
        vertices_[vert_count - 1].x * vertices_[0].y - vertices_[0].x * vertices_[vert_count - 1].y;
    return std::abs(sum) / 2.0;
}

bool Polygon::operator==(const Shape& right) const {
    if (ImpossibleCompare(right)) {
        return false;
    }
    const Polygon& right_pol = dynamic_cast<const Polygon&>(right);
    size_t entry_pos;
    return VectorsEquality<Point>(
        vertices_, right_pol.vertices_, [](Point a, Point b) { return a == b; }, entry_pos);
}

bool Polygon::operator==(const Polygon& right) const {
    return operator==(static_cast<const Shape&>(right));
}

bool Polygon::IsSimilarTo(const Shape& right) const {
    double ratio = 0;
    return CheckSimilarity(right, ratio);
}

bool Polygon::IsCongruentTo(const Shape& right) const {
    double ratio = 0;
    bool flag_similarity = CheckSimilarity(right, ratio);
    bool flag_ratio = std::abs(ratio - 1.0) < kEpsilon;
    return flag_similarity && flag_ratio;
}

bool Polygon::ContainsPoint(const Point& pt) const {
    bool inside = false;
    size_t size = VerticesCount();
    for (size_t i = 0; i < size; ++i) {
        size_t j = (i + 1) % size;
        const Point& p1 = vertices_[i];
        const Point& p2 = vertices_[j];
        if (IsPointOnEdge(p1, p2, pt)) {
            return true;
        }
        bool intersect = ((pt.y < p1.y) != (pt.y < p2.y)) &&
                         (pt.x < (p2.x - p1.x) * (pt.y - p1.y) / (p2.y - p1.y) + p1.x);
        if (intersect) {
            inside = !inside;
        }
    }
    return inside;
}

void Polygon::Rotate(const Point& center, double angle) {
    using std::sin, std::cos;
    double rad_angle = angle * std::numbers::pi / 180.0;
    size_t size = VerticesCount();
    for (size_t i = 0; i < size; ++i) {
        // Vector vector_relative(vertices_[i] - center);
        // vertices_[i] = Point((vector_relative.Rotate(rad_angle) + center));
        double dx = vertices_[i].x - center.x;
        double dy = vertices_[i].y - center.y;
        vertices_[i] = {center.x + dx * cos(rad_angle) - dy * sin(rad_angle),
                        center.y + dx * sin(rad_angle) + dy * cos(rad_angle)};
    }
}

void Polygon::Reflect(const Point& center) {
    size_t size = VerticesCount();
    for (size_t i = 0; i < size; ++i) {
        vertices_[i] = Point(center * 2.0 - vertices_[i]);
    }
}

void Polygon::Reflect(const Line& axis) {
    size_t size = VerticesCount();
    for (size_t i = 0; i < size; ++i) {
        vertices_[i] = ReflectPointOverLine(vertices_[i], axis);
    }
}

void Polygon::Scale(const Point& center, double coefficient) {
    size_t size = VerticesCount();
    for (size_t i = 0; i < size; ++i) {
        vertices_[i] = Point(center + (vertices_[i] - center) * coefficient);
    }
}

Shape::Figure Polygon::GetFigure() const {
    return kPol;
}

size_t Polygon::VerticesCount() const {
    return vertices_.size();
}

const std::vector<Point>& Polygon::GetVertices() const {
    return vertices_;
}

bool Polygon::IsConvex() const {
    bool turn_direction;  //  направление поворота(0 - налево, 1 - направо)
    size_t size = VerticesCount();
    Point last_pt = vertices_[size - 1];
    Point first_pt = vertices_[0];
    turn_direction = DefineTurn(vertices_[size - 2], last_pt, first_pt);
    if (DefineTurn(last_pt, first_pt, vertices_[1]) != turn_direction) {
        return false;
    }
    for (size_t i = 1; i < size - 1; ++i) {
        if (DefineTurn(vertices_[i - 1], vertices_[i], vertices_[i + 1]) != turn_direction) {
            return false;
        }
    }
    return true;
}

bool Polygon::SidesSimilaritry(const std::vector<double>& left, const std::vector<double>& right,
                               double& ratio, size_t entry_pos) {
    if (left.size() != right.size()) {
        return false;
    }
    size_t size = left.size();
    double ratio_forward = left[0] / right[entry_pos];
    double ratio_reverse = left[0] / right[(entry_pos - 1 + size) % size];
    bool match_forward = true;
    bool match_reverse = true;
    for (size_t i = 0; i < size; ++i) {
        size_t j_forward = (entry_pos + i) % size;
        size_t j_reverse = (entry_pos - 1 + size - i) % size;
        if (std::abs(left[i] - right[j_forward] * ratio_forward) >= kEpsilon) {
            match_forward = false;
        }
        if (std::abs(left[i] - right[j_reverse] * ratio_reverse) >= kEpsilon) {
            match_reverse = false;
        }
        if (!match_forward && !match_reverse) {
            return false;
        }
    }
    ratio = (match_forward) ? ratio_forward : ratio_reverse;
    return true;
}

template <typename T>
bool Polygon::VectorsEquality(const std::vector<T>& left, const std::vector<T>& right,
                              bool (*equal)(T a, T b), size_t& entry_pos) {
    size_t left_size = left.size();
    size_t right_size = right.size();
    if (left_size != right_size) {
        return false;
    }
    size_t size = left_size;
    std::vector<size_t> entry_positions{};
    for (size_t i = 0; i < size; ++i) {
        if (equal(left[0], right[i])) {
            entry_positions.push_back(i);
        }
    }
    if (entry_positions.empty()) {
        return false;
    }
    size_t pos_size = entry_positions.size();
    for (size_t k = 0; k < pos_size; ++k) {
        bool match_forward = true;
        bool match_reverse = true;
        size_t entry_position = entry_positions[k];
        for (size_t i = 0; i < size; ++i) {
            size_t j_forward = (entry_position + i) % size;
            size_t j_reverse = (entry_position + size - i) % size;
            if (!equal(left[i], right[j_forward])) {
                match_forward = false;
            }
            if (!equal(left[i], right[j_reverse])) {
                match_reverse = false;
            }
            if (!match_forward && !match_reverse) {
                break;
            }
        }
        if (match_forward || match_reverse) {
            entry_pos = entry_position;
            return true;
        }
    }
    return false;
}

bool Polygon::IsPointOnEdge(const Point& p1, const Point& p2, const Point& pt) const {
    if (pt == p1 || pt == p2) {
        return true;
    }
    double check_coll = (pt.y - p1.y) * (p2.x - p1.x) - (pt.x - p1.x) * (p2.y - p1.y);
    if (std::abs(check_coll) >= kEpsilon) {
        return false;
    }  //  проверка что точка лежит на линии ребра
    double between = (pt.x - p1.x) * (p2.x - p1.x) + (pt.y - p1.y) * (p2.y - p1.y);
    if (between < 0) {
        return false;
    }  //  проверка что точка лежит до p1
    double sq_len = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
    if (between > sq_len) {
        return false;
    }  //  проверка что точка лежит после p2
    return true;
}

std::vector<double> Polygon::GetSides() const {
    std::vector<double> sides{};
    size_t size = VerticesCount();
    sides.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        size_t next = (i + 1) % size;
        double d = Distance(vertices_[i], vertices_[next]);
        sides.push_back(d);
    }
    return sides;
}

std::vector<double> Polygon::GetAngles() const {
    std::vector<double> angles{};
    size_t size = VerticesCount();
    angles.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        size_t next = (i + 1) % size;
        size_t previous = (i + size - 1) % size;
        Vector v1(vertices_[i], vertices_[next]);
        Vector v2(vertices_[i], vertices_[previous]);
        angles.push_back(Vector::GetAngle(v1, v2));
    }
    return angles;
}

bool Polygon::CheckSimilarity(const Shape& right, double& ratio) const {
    if (ImpossibleCompare(right)) {
        return false;
    }
    const Polygon& right_pol = dynamic_cast<const Polygon&>(right);
    if (IsConvex() != right_pol.IsConvex()) {
        return false;
    }
    std::vector<double> left_angles = GetAngles();
    std::vector<double> right_angles = right_pol.GetAngles();
    size_t entry_pos;
    if (!VectorsEquality<double>(
            left_angles, right_angles,
            [](double a, double b) { return std::abs(a - b) < kEpsilon; }, entry_pos)) {
        return false;
    }
    std::vector<double> left_sides = GetSides();
    std::vector<double> right_sides = right_pol.GetSides();
    return SidesSimilaritry(left_sides, right_sides, ratio, entry_pos);
}

bool Polygon::ImpossibleCompare(const Shape& right) const {
    Figure right_type = right.GetFigure();
    if (right_type == kEll || right_type == kCircle) {
        return true;
    }
    size_t left_size = VerticesCount();
    const Polygon& right_pol = dynamic_cast<const Polygon&>(right);
    size_t right_size = right_pol.VerticesCount();
    if (left_size != right_size) {
        return true;
    }
    return false;
}

bool Polygon::DefineTurn(const Point& a, const Point& b, const Point& c) const {
    Vector ab(a, b);
    Vector bc(b, c);
    return (ab.x_ * bc.y_ - ab.y_ * bc.x_) < 0;
}

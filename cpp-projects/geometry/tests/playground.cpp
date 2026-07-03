
#include "geometry.hpp"

#include <gtest/gtest.h>

TEST(YourTest, Test) {
    Point p{0, 0};

    ASSERT_EQ(p.x, 0);
    ASSERT_EQ(p.y, 0);
}

TEST(EllipseCircle, Comparisons) {
    Point center(0, 0);
    Circle circle(center, 5);
    Ellipse ellipse(Point(-3, 0), Point(3, 0), 10);

    ASSERT_FALSE(circle == ellipse);
    ASSERT_TRUE(circle != ellipse);

    Circle circle2(center, 5);
    ASSERT_TRUE(circle == circle2);
    ASSERT_FALSE(circle != circle2);

    ASSERT_TRUE(circle.ContainsPoint(Point(0, 0)));
    ASSERT_TRUE(circle.ContainsPoint(Point(4, 0)));
    ASSERT_TRUE(circle.ContainsPoint(Point(5, 0)));
    ASSERT_FALSE(circle.ContainsPoint(Point(6, 0)));
}

TEST(ShapeOperators, AmbiguityAllignment) {
    Point p1(0, 0), p2(3, 0), p3(0, 4);
    Triangle t1(p1, p2, p3);
    Triangle t2(p1, p2, p3);

    Shape& s1 = t1;
    Shape& s2 = t2;

    ASSERT_TRUE(s1 == s2);
    ASSERT_FALSE(s1 != s2);
}

TEST(ShapeOperators, MixedTypesComparison) {
    Point a(0, 0), b(4, 0), c(0, 3);
    Triangle triangle(a, b, c);

    Point ll(0, 0), ur(4, 3);
    Rectangle rect(ll, ur, 4.0/3.0);

    Shape& s1 = triangle;
    Shape& s2 = rect;

    ASSERT_FALSE(s1 == s2);
    ASSERT_TRUE(s1 != s2);
}

TEST(ShapeOperators, EllipseCircleComparison) {
    Point center(0, 0);
    Circle circle(center, 5);
    Ellipse ellipse(Point(-3, 0), Point(3, 0), 10);

    Shape& s1 = circle;
    Shape& s2 = ellipse;

    ASSERT_FALSE(s1 == s2);
    ASSERT_TRUE(s1 != s2);

    Circle circle2(center, 5);
    Shape& s3 = circle2;

    ASSERT_TRUE(s1 == s3);
    ASSERT_FALSE(s1 != s3);
}

TEST(Triangle, BasicProperties) {
    Triangle t1({Point(0, 0), Point(3, 0), Point(0, 4)});

    EXPECT_NEAR(t1.Perimeter(), 12.0, kEpsilon);
    EXPECT_NEAR(t1.Area(), 6.0, kEpsilon);
    EXPECT_TRUE(t1.ContainsPoint(Point(1, 1)));
    EXPECT_FALSE(t1.ContainsPoint(Point(-1, -1)));
}

TEST(Triangle, CircumscribedCircle) {
    Triangle t1({Point(0, 0), Point(3, 0), Point(0, 4)});
    Circle c1 = t1.CircumscribedCircle();

    EXPECT_NEAR(c1.Radius(), 2.5, kEpsilon);
    EXPECT_TRUE(t1.ContainsPoint(c1.Center()) || 
                std::abs(Distance(c1.Center(), Line(Point(0,0), Point(3,0)))) < kEpsilon);

    for (const auto& vertex : t1.GetVertices()) {
        EXPECT_NEAR(Distance(c1.Center(), vertex), c1.Radius(), kEpsilon);
    }
}

TEST(Triangle, InscribedCircle) {
    Triangle t1({Point(0, 0), Point(3, 0), Point(0, 4)});
    Circle c1 = t1.InscribedCircle();

    double ab, bc, ac;
    t1.GetLengths(ab, bc, ac);
    double expected_radius = 1.0;
    EXPECT_NEAR(c1.Radius(), expected_radius, kEpsilon);

    for (size_t i = 0; i < 3; ++i) {
        size_t j = (i + 1) % 3;
        Line side(t1.GetVertices()[i], t1.GetVertices()[j]);
        EXPECT_NEAR(Distance(c1.Center(), side), c1.Radius(), kEpsilon);
    }
}

TEST(Triangle, Centroid) {
    Triangle t1({Point(0, 0), Point(6, 0), Point(0, 9)});
    Point c = t1.Centroid();

    EXPECT_NEAR(c.x, 2.0, kEpsilon);
    EXPECT_NEAR(c.y, 3.0, kEpsilon);

    Triangle t2({Point(1, 1), Point(4, 2), Point(2, 5)});
    Point c2 = t2.Centroid();
    EXPECT_NEAR(c2.x, (1 + 4 + 2) / 3.0, kEpsilon);
    EXPECT_NEAR(c2.y, (1 + 2 + 5) / 3.0, kEpsilon);
}

TEST(Triangle, Orthocenter) {
    Triangle t1({Point(0, 0), Point(3, 0), Point(0, 4)});
    Point o = t1.Orthocenter();

    EXPECT_NEAR(o.x, 0.0, kEpsilon);
    EXPECT_NEAR(o.y, 0.0, kEpsilon);

    Triangle t2({Point(0, 0), Point(4, 0), Point(2, 3)});
    Point o2 = t2.Orthocenter();
    EXPECT_NEAR(o2.x, 2.0, kEpsilon);
    EXPECT_NEAR(o2.y, 1.333333333333333333, kEpsilon);
}

TEST(Triangle, EulerLine) {
    Triangle t1({Point(0, 0), Point(4, 0), Point(2, 3)});
    Line euler = t1.EulerLine();

    Point centroid = t1.Centroid();
    Point orthocenter = t1.Orthocenter();

    double a, b, c;
    euler.GetData(a, b, c);
    EXPECT_NEAR(a * centroid.x + b * centroid.y + c, 0.0, kEpsilon);
    EXPECT_NEAR(a * orthocenter.x + b * orthocenter.y + c, 0.0, kEpsilon);
}

TEST(Triangle, NinePointsCircle) {
    Triangle t1({Point(0, 0), Point(4, 0), Point(2, 3)});
    Circle nine = t1.NinePointsCircle();

    // Point centroid = t1.Centroid();
    // Point orthocenter = t1.Orthocenter();
    Circle circum = t1.CircumscribedCircle();

    Point expected_center = {2.0, 1.0833333333333};

    EXPECT_NEAR(Distance(nine.Center(), expected_center), 0.0, kEpsilon);
    EXPECT_NEAR(nine.Radius(), circum.Radius() / 2.0, kEpsilon);

    Point ab_mid((t1.GetVertices()[0].x + t1.GetVertices()[1].x) / 2.0,
                (t1.GetVertices()[0].y + t1.GetVertices()[1].y) / 2.0);
    EXPECT_NEAR(Distance(nine.Center(), ab_mid), nine.Radius(), kEpsilon);
}

TEST(Triangle, InscribedCircumscribed) {
    {
    Triangle tr({10,3}, {10,9}, {3,7});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }

    {
    Triangle tr({0, 0}, {-60, 0}, {0, 80});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }

    {
    Triangle tr({0, 12}, {-5, 0}, {0,0});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }

    {
    Triangle tr({1005,3}, {-1005,9}, {0,-768});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }
    
    {
    Triangle tr({0, 0}, {9, 0}, {4.5, 7.794});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }

    {
    Triangle tr({0, 0}, {7, 0}, {3.5, 6.062});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }

    {
    Triangle tr({0, 0}, {2, 0}, {1, -3});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }

    {
    Triangle tr({0, 0}, {1, 0}, {0.5, 1e-6});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }

    {
    Triangle tr({0, 0}, {1, 0}, {0.5, sqrt(3)/2});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }

    {
    Triangle tr({0, 0}, {1, 0}, {0.5, -10});

    Point c_in = tr.InscribedCircle().Center();
    Point c_out = tr.CircumscribedCircle().Center();
    double r_in = tr.InscribedCircle().Radius();
    double r_out = tr.CircumscribedCircle().Radius();

    EXPECT_NEAR(Distance(c_in, c_out), sqrt(std::abs(r_out * (r_out - 2.0 * r_in))), kEpsilon);
    }
}

TEST(Triangle, EquilateralProperties) {
    double side = 5.0;
    Triangle t1({Point(0, 0), Point(side, 0), 
                Point(side/2.0, side * sqrt(3.0)/2.0)});

    EXPECT_TRUE(t1.IsConvex());

    Circle circum = t1.CircumscribedCircle();
    Circle insc = t1.InscribedCircle();

    EXPECT_NEAR(circum.Radius(), side / sqrt(3.0), kEpsilon);
    EXPECT_NEAR(insc.Radius(), side * sqrt(3.0) / 6.0, kEpsilon);

    Point centroid = t1.Centroid();
    Point orthocenter = t1.Orthocenter();
    EXPECT_NEAR(Distance(centroid, orthocenter), 0.0, kEpsilon);
}

TEST(Triangle, RightTriangleProperties) {
    Triangle t1({Point(0, 0), Point(3, 0), Point(0, 4)});

    Point orthocenter = t1.Orthocenter();
    EXPECT_NEAR(orthocenter.x, 0.0, kEpsilon);
    EXPECT_NEAR(orthocenter.y, 0.0, kEpsilon);

    Circle circum = t1.CircumscribedCircle();
    EXPECT_NEAR(circum.Center().x, 1.5, kEpsilon);
    EXPECT_NEAR(circum.Center().y, 2.0, kEpsilon);
    EXPECT_NEAR(circum.Radius(), 2.5, kEpsilon);
}
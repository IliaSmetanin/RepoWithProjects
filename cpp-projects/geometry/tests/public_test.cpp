#include "geometry.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

static double double_error = 1e-5;

bool EqualsDouble(double lhs, double rhs, double eps) {
    return std::abs(lhs - rhs) < eps;
}

bool Contains(const auto& container, const auto& item) {
    return std::find(std::begin(container), std::end(container), item) != std::end(container);
}

// ===== Polygon =====
struct PolygonData {
    std::initializer_list<Point> points;
    bool is_convex = false;
    double perimeter = 0.0;
    double area = 0.0;

    bool do_raster = false;
    double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;
    size_t samples_x = 0, samples_y = 0;
    std::string image = {};

    std::vector<Point> GetPointsVector() const {
        return {points};
    }

    std::vector<Point> GetReversedPoints() const {
        auto result = GetPointsVector();
        std::reverse(result.begin(), result.end());
        return result;
    }
};

const std::vector<Point> kManualTestPoints = {
    Point{0.8, 0.8},
    Point{2.6, 2.8},
    Point{0.6, 2.2},
};

const PolygonData kPolygonCollection[] = {
    {
        .points =
            {
                {0.8, 0.8},
                {3.4, 1.8},
                {2.6, 2.8},
                {1.4, 1.8},
                {0.6, 2.2},
            },
        .is_convex = false,
        .perimeter = 7.936993191477734,
        .area = 2.52,

        .do_raster = true,
        .x0 = 0,
        .y0 = 0,
        .x1 = 4,
        .y1 = 3,
        .samples_x = 41,
        .samples_y = 31,
        .image = R"((


                          #
                         ##
                        ####
                       ######
                      ########
                    ###########
      #            ############
       ##         ##############
       ####      ################
       ######   ##################
       ############################
       #########################
       ######################
       ####################
        ################
        ##############
        ###########
        ########
        ######
        ###
        #








))",
    },

    {
        .points =
            {
                {0.1, 0.1},
                {3.4, 1.8},
                {2.6, 2.8},
                {1.4, 1.8},
            },
        .is_convex = true,
        .perimeter = 8.694910477225286,
        .area = 2.7,

        .do_raster = true,
        .x0 = 0,
        .y0 = 0,
        .x1 = 4,
        .y1 = 3,
        .samples_x = 41,
        .samples_y = 31,
        .image = R"((


                          #
                         ##
                        ####
                       ######
                      ########
                    ###########
                   ############
                  ##############
                 ################
                ##################
              #####################
              ###################
             ##################
            #################
           ################
           ##############
          #############
         ############
        ###########
        #########
       ########
      #######
     ######
     ####
    ###
   ##
  #
 #

))",
    },

    {
        .points =
            {
                {0.4, 1.4},
                {0.6, 1.4},
                {0.6, 1.8},
                {1, 1.8},
                {1, 1.4},
                {1.4, 1.4},
                {1.6, 2},
                {1.8, 1.6},
                {2.4, 1.4},
                {1.8, 1},
                {2.2, 0.8},
                {1.8, 0.4},
                {0.4, 0.4},
            },
        .is_convex = false,
        .perimeter = 7.646133935109306,
        .area = 2,

        .do_raster = true,
        .x0 = 0,
        .y0 = 0,
        .x1 = 2.6,
        .y1 = 2.2,
        .samples_x = 27,
        .samples_y = 23,
        .image = R"((


                #
                #
      #####     ##
      #####    ###
      #####    ####
      #####    #######
    #####################
    ###################
    ##################
    ################
    ###############
    #################
    ###################
    ##################
    #################
    ################
    ###############




))",
    },

    {
        .points =
            {
                {2, 1.8},
                {3.4, 1.8},
                {2.6, 2.8},
                {1, 1.4},
            },
        .is_convex = false,
        .perimeter = 5.8836869714604,
        .area = 1.08,
    },

    {
        .points =
            {
                {0, 0},
                {1, 0},
                {1, 1},
                {0, 1},
                {-0.5, 0.5},
            },
        .is_convex = true,
        .perimeter = 4.414213562373095,
        .area = 1.25,
    },

    {
        .points =
            {
                {0, 0},
                {1, 0},
                {1, 1},
                {0, 1},
                {0.5, 0.5},
            },
        .is_convex = false,
        .perimeter = 4.414213562373095,
        .area = 0.75,
    },

    {
        .points =
            {
                {0, 0},
                {1, 0},
                {0.5, 0.8660254037844386},
            },
        .is_convex = true,
        .perimeter = 3,
        .area = 0.4330127018922193,
    },

    {
        .points =
            {
                {1, 1},
                {1, 4},
                {5, 4},
            },
        .is_convex = true,
        .perimeter = 12,
        .area = 6,
    },

    {
        .points =
            {
                {2.2, 1.2},
                {3.4, 1.8},
                {1.2, 2},
            },
        .is_convex = true,
        .perimeter = 4.831337837423896,
        .area = 0.78,
    },
};

void TestRaster(PolygonData const& testcase, Polygon const& poly) {
    for (auto p : testcase.points) {
        ASSERT_TRUE(poly.ContainsPoint(p));
    }

    if (!testcase.do_raster) {
        return;
    }

    std::string image = "(\n";
    for (size_t y_sample = testcase.samples_y; y_sample--;) {
        std::string line;
        for (size_t x_sample = 0; x_sample < testcase.samples_x; ++x_sample) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
            double x =
                (testcase.x0 * (testcase.samples_x - x_sample - 1) + testcase.x1 * x_sample) /
                (testcase.samples_x - 1);
            double y =
                (testcase.y0 * (testcase.samples_y - y_sample - 1) + testcase.y1 * y_sample) /
                (testcase.samples_y - 1);
#pragma GCC diagnostic pop
            line += poly.ContainsPoint({x, y}) ? '#' : ' ';
        }
        while (!line.empty() && line.back() == ' ') {
            line.pop_back();
        }
        image += line + '\n';
    }
    image += ")";

    ASSERT_EQ(testcase.image, image);
    if (testcase.image != image) {
        std::cout << "Expected image: " << testcase.image << "\nProduced image: " << image
                  << std::endl;
    }
}

struct LinearTransform {
    double a0 = 1, b0 = 0, c0 = 0;
    double a1 = 0, b1 = 1, c1 = 0;

    bool is_identity = true;
    bool is_movement = true;

    Point ApplyTo(Point const& p) const {
        return {p.x * a0 + p.y * b0 + c0, p.x * a1 + p.y * b1 + c1};
    }

    LinearTransform Rotate(double angle) const {
        double sin = std::sin(angle), cos = std::cos(angle);

        return {
            .a0 = a0 * cos - a1 * sin,
            .b0 = b0 * cos - b1 * sin,
            .c0 = c0 * cos - c1 * sin,

            .a1 = a0 * sin + a1 * cos,
            .b1 = b0 * sin + b1 * cos,
            .c1 = c0 * sin + c1 * cos,

            .is_identity = false,
            .is_movement = is_movement,
        };
    }

    LinearTransform Move(Point p) const {
        return {
            .a0 = a0,
            .b0 = b0,
            .c0 = c0 + p.x,
            .a1 = a1,
            .b1 = b1,
            .c1 = c1 + p.y,

            .is_identity = false,
            .is_movement = is_movement,
        };
    }

    LinearTransform Scale(double k) const {
        return {
            .a0 = k * a0,
            .b0 = k * b0,
            .c0 = k * c0,
            .a1 = k * a1,
            .b1 = k * b1,
            .c1 = k * c1,

            .is_identity = false,
            .is_movement = false,
        };
    }
};

constexpr LinearTransform kIdentity{};
constexpr LinearTransform kMirror{-1, 0, 0, 0, 1, 0, false};

const std::vector<LinearTransform> kTransformsCollection = {
    kIdentity,
    kIdentity.Rotate(M_PI / 2),
    kIdentity.Move({1, 2}),
    kIdentity.Move({0, 1}),
    kIdentity.Scale(2),

    kIdentity.Move({-1, -1}).Scale(2).Move({1, 1}),
    kIdentity.Move({-1, 2}).Rotate(M_PI / 3).Move({1, -2}),

    kMirror,
    kMirror.Rotate(M_PI / 2),
    kMirror.Move({1, 2}),
    kMirror.Move({0, 1}),
    kMirror.Scale(2),

    kMirror.Move({-1, -1}).Scale(2).Move({1, 1}),
    kMirror.Move({-1, 2}).Rotate(M_PI / 3).Move({1, -2}),

};

TEST(Point, ConstructionAndComparison) {
    ASSERT_TRUE((Point{1, 2} == Point{1, 2}));
    ASSERT_TRUE(!(Point{1, 2} == Point{1.5, 2}));
    ASSERT_TRUE(!(Point{1, 2} == Point{1, 2.5}));
    ASSERT_TRUE(!(Point{1, 2} == Point{1.5, 2.5}));
    ASSERT_TRUE((Point{0, 0} == Point{0, 0}));

    ASSERT_TRUE(!(Point{1, 2} != Point{1, 2}));
    ASSERT_TRUE((Point{1, 2} != Point{1.5, 2}));
    ASSERT_TRUE((Point{1, 2} != Point{1, 2.5}));
    ASSERT_TRUE((Point{1, 2} != Point{1.5, 2.5}));
    ASSERT_TRUE(!(Point{0, 0} != Point{0, 0}));
}

TEST(Line, ConstructionAndComparison) {
    std::vector<Line> lines = {
        /*0*/ Line{2, 5},
        /*1*/ Line{Point{0, 5}, 2},
        /*2*/ Line{Point{-4, -3}, 2},
        /*3*/ Line{Point{-4, -3}, Point{1.5, 8}},
        /*4*/ Line{Point{1.5, 8}, Point{-4, -3}},

        /*5*/ Line{2, 7},
        /*6*/ Line{Point{-0.5, 6}, 2},
        /*7*/ Line{Point{0, 7}, Point{-3.25, 0.5}},
        /*8*/ Line{Point{-3.25, 0.5}, 2},

        /*9*/ Line{0, 0},
        /*10*/ Line{Point{-0.3, 0}, Point{0.4, 0}},
        /*11*/ Line{Point{-224, 0}, 0},

        /*12*/ Line{0, 1},
        /*13*/ Line{Point{-0.3, 1}, Point{0.4, 1}},
        /*14*/ Line{Point{-224, 1}, 0},

        /*15*/ Line{Point{0, 0}, Point{0, 1}},
        /*16*/ Line{Point{0, 224}, Point{0, 37700}},

        /*17*/ Line{Point{1, 0}, Point{1, 1}},
        /*18*/ Line{Point{1, 420}, Point{1, 3665}},
    };

    std::vector<int> equiv_class = {
        0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5,
    };

    for (size_t i = 0; i < lines.size(); ++i) {
        for (size_t j = 0; j < lines.size(); ++j) {
            ASSERT_EQ(lines[i] == lines[j], equiv_class[i] == equiv_class[j]);
            ASSERT_EQ(lines[i] != lines[j], equiv_class[i] != equiv_class[j]);
        }
    }
}

TEST(Polygon, Construction) {
    for (auto const& testcase : kPolygonCollection) {
        Polygon poly{testcase.points};
        ASSERT_EQ(poly.VerticesCount(), testcase.points.size());
        ASSERT_EQ(poly.GetVertices(), testcase.GetPointsVector());
    }

    auto poly = Polygon{
        Point{0.8, 0.8},
        Point{2.6, 2.8},
        Point{0.6, 2.2},
    };
    ASSERT_EQ(poly.VerticesCount(), size_t(3));
    ASSERT_EQ(poly.GetVertices(), kManualTestPoints);
}

TEST(Polygon, GetVerticesOfRValue) {
    for (auto const& testcase : kPolygonCollection) {
        ASSERT_EQ(Polygon{testcase.points}.GetVertices(), testcase.GetPointsVector());
    }

    ASSERT_EQ((Polygon{
                   Point{0.8, 0.8},
                   Point{2.6, 2.8},
                   Point{0.6, 2.2},
               })
                  .GetVertices(),
              kManualTestPoints);
}

TEST(Polygon, PerimeterAndArea) {
    for (auto const& testcase : kPolygonCollection) {
        ASSERT_TRUE(std::abs(Polygon{testcase.points}.Perimeter() - testcase.perimeter) < 1e-5);
        ASSERT_TRUE(std::abs(Polygon{testcase.points}.Area() - testcase.area) < 1e-5);
    }
}

TEST(Polygon, IsConvex) {
    for (auto const& testcase : kPolygonCollection) {
        ASSERT_EQ(Polygon{testcase.points}.IsConvex(), testcase.is_convex);
        ASSERT_EQ(Polygon(testcase.GetReversedPoints()).IsConvex(), testcase.is_convex);
    }
}

TEST(Polygon, ContainsPoint) {
    for (auto const& testcase : kPolygonCollection) {
        Polygon poly{testcase.points};

        TestRaster(testcase, poly);
        TestRaster(testcase, Polygon(testcase.GetReversedPoints()));
    }
}

TEST(Polygon, TrivialComparisons) {
    for (auto const& poly1 : kPolygonCollection) {
        for (auto const& poly2 : kPolygonCollection) {
            bool are_equal = &poly1 - kPolygonCollection == &poly2 - kPolygonCollection;

            ASSERT_EQ(Polygon(poly1.points) == Polygon(poly2.points), are_equal);
            ASSERT_EQ(Polygon(poly1.points).IsCongruentTo(Polygon(poly2.points)), are_equal);
            ASSERT_EQ(Polygon(poly1.points).IsSimilarTo(Polygon(poly2.points)), are_equal);
        }
    }
}

TEST(Polygon, Comparisons) {
    for (auto const& poly : kPolygonCollection) {
        for (auto const& poly1 : kPolygonCollection) {
            bool are_equal = &poly - kPolygonCollection == &poly1 - kPolygonCollection;

            for (auto const& transform : kTransformsCollection) {
                std::vector<Point> poly2 = poly1.GetPointsVector();
                for (auto& p : poly2) {
                    p = transform.ApplyTo(p);
                }

                for (size_t is_reversed = 0; is_reversed < 2; ++is_reversed) {
                    for (size_t shift = 0; shift < poly2.size(); ++shift) {
                        Polygon poly3(poly.points);
                        ASSERT_EQ(poly3 == Polygon(poly2), transform.is_identity && are_equal);
                        ASSERT_EQ(poly3.IsCongruentTo(Polygon(poly2)),
                                  transform.is_movement && are_equal);
                        ASSERT_EQ(poly3.IsSimilarTo(Polygon(poly2)), are_equal);
                        std::rotate(poly2.begin(), poly2.begin() + 1, poly2.end());
                    }
                    std::reverse(poly2.begin(), poly2.end());
                }
            }
        }
    }
}

TEST(Polygon, Transforms) {
    for (auto const& poly : kPolygonCollection) {
        auto check = [&](Polygon const& got, LinearTransform expected_transform, std::string what) {
            std::vector<Point> short_cut = poly.GetPointsVector();
            for (auto& p : short_cut) {
                p = expected_transform.ApplyTo(p);
            }

            ASSERT_EQ(got, Polygon{short_cut}) << what << '\n';
        };

        Polygon p(poly.points), q = p;
        q.Rotate({0, 0}, 45);
        check(q, kIdentity.Rotate(M_PI / 4), "Rotate 45deg");

        q = p;
        q.Rotate({1, 2}, -45);
        check(q, kIdentity.Move({-1, -2}).Rotate(-M_PI / 4).Move({1, 2}),
              "Rotate 45deg around (1, 2)");

        q = p;
        q.Reflect(Point{0, 0});
        check(q, {-1, 0, 0, 0, -1, 0}, "Reflect over (0, 0)");

        q = p;
        q.Reflect(Point{1, 2});
        check(q, {-1, 0, 2, 0, -1, 4}, "Reflect over (1, 2)");

        q = p;
        q.Scale({0, 0}, 2);
        check(q, {2, 0, 0, 0, 2, 0}, "Scale by 2");

        q = p;
        q.Scale({1, 2}, 2);
        check(q, {2, 0, -1, 0, 2, -2}, "Scale by 2 using (1, 2) as the origin");

        q = p;
        q.Reflect(Line{0, 0});
        check(q, {1, 0, 0, 0, -1, 0}, "Reflect through y=0");

        q = p;
        q.Reflect(Line{Point{0, 0}, Point{0, 2}});
        check(q, {-1, 0, 0, 0, 1, 0}, "Reflect through x=0");

        q = p;
        q.Reflect(Line{Point{1, 0}, Point{0, 1}});
        check(q, {0, -1, 1, -1, 0, 1}, "Reflect through x+y=1 (#1)");

        q = p;
        q.Reflect(Line{Point{2, -1}, Point{0, 1}});
        check(q, {0, -1, 1, -1, 0, 1}, "Reflect through x+y=1 (#2)");
    }
}

TEST(Ellipse, ConstructionAndGetters) {
    Ellipse a{{-1.5, 0}, {1.5, 0}, 5};
    Ellipse b{{1, 1}, {4, 5}, 6};

    {
        auto [first, second] = a.Focuses();
        Point correct[] = {{-1.5, 0}, {1.5, 0}};
        ASSERT_TRUE(Contains(correct, first));
        ASSERT_TRUE(Contains(correct, second));
    }

    {
        auto [first, second] = a.Directrices();
        Line correct[] = {{{-4.166666666666667, 0}, {-4.166666666666667, 1}},
                          {{4.166666666666667, 0}, {4.166666666666667, 1}}};
        ASSERT_TRUE(Contains(correct, first));
        ASSERT_TRUE(Contains(correct, second));
    }

    ASSERT_EQ(a.Center(), (Point{0, 0}));
    ASSERT_TRUE(EqualsDouble(a.Eccentricity(), 0.6, double_error));
    ASSERT_TRUE(EqualsDouble(a.Area(), 15.707963267948966, double_error));
    {
        const double real_perimeter = 14.180833944487244;
        const double perimeter_error = real_perimeter * 0.05 + double_error;
        ASSERT_TRUE(EqualsDouble(a.Perimeter(), real_perimeter, perimeter_error));
    }

    {
        auto [first, second] = b.Focuses();
        Point correct[] = {{1, 1}, {4, 5}};
        ASSERT_TRUE(Contains(correct, first));
        ASSERT_TRUE(Contains(correct, second));
    }

    {
        auto [first, second] = b.Directrices();
        Line correct[] = {
            {-0.75, 0.375},
            {-0.75, 9.375},
        };
        ASSERT_TRUE(Contains(correct, first));
        ASSERT_TRUE(Contains(correct, second));
    }

    ASSERT_EQ(b.Center(), (Point{2.5, 3}));
    ASSERT_TRUE(EqualsDouble(b.Eccentricity(), 5. / 6, double_error));
    ASSERT_TRUE(EqualsDouble(b.Area(), 15.629226114141467, double_error));
    {
        const double real_perimeter = 14.180833944487244;
        const double perimeter_error = real_perimeter * 0.05 + double_error;
        ASSERT_TRUE(EqualsDouble(b.Perimeter(), 14.939631086609392, perimeter_error));
    }
}

TEST(Rectangle, Construction) {
    ASSERT_EQ((Rectangle{{1, 1}, {3, 3}, 1}), (Polygon{{1, 1}, {3, 1}, {3, 3}, {1, 3}}));
    ASSERT_EQ((Rectangle{{1, 2}, {5, 5}, 0.75}), (Polygon{{1, 2}, {5, 2}, {5, 5}, {1, 5}}));
    ASSERT_EQ((Rectangle{{1, 2}, {5, 5}, 4 / 3.}), (Polygon{{1, 2}, {5, 2}, {5, 5}, {1, 5}}));
}

TEST(Triangle, ConstructionAndGetters) {
    Triangle a{{3, 0}, {0, 3}, {6, 1}};

    ASSERT_EQ(a.CircumscribedCircle(), (Circle{{3.5, 3.5}, 3.5355339059327373}));
    ASSERT_EQ(a.InscribedCircle(), (Circle{{3.23606797749979, 1}, 0.8740320488976421}));
    ASSERT_EQ(a.Centroid(), (Point{3, 1.3333333333333333}));
    ASSERT_EQ(a.Orthocenter(), (Point{2, -3}));
    ASSERT_EQ(a.EulerLine(), (Line{{2, -3}, {3, 1.3333333333333333}}));
    ASSERT_EQ(a.NinePointsCircle(), (Circle{{2.75, 0.25}, 1.7677669529663687}));
}

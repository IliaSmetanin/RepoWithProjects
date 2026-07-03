
#include "rational.hpp"

#include <gtest/gtest.h>

void Check(bool statement, std::string_view error_msg) {
    ASSERT_TRUE(statement) << error_msg;
}

namespace detail {
const size_t kLen = 70;
}

void CheckWithConversions(const Rational& value, const Rational& expected_r,
                          const std::string& expected_d, std::string_view expected_s,
                          const std::string& error_msg, const size_t len = detail::kLen) {
    Check(value == expected_r, error_msg + " (with operator==)");
    Check(value.AsDecimal(len) == expected_d, error_msg + " (with AsDecimal)" +
                                                  "\nexp: " + expected_d +
                                                  "\ngot: " + value.AsDecimal(len) + "\n");
    Check(value.ToString() == expected_s, error_msg + " (with ToString)");
}

TEST(Rational, ArithmeticsTest) {
    {
        Rational r = 5;
        CheckWithConversions(r, Rational(5), "5.0", "5", "Constructor from int", 1);
        r += 3;
        CheckWithConversions(r, Rational(8), "8.0", "8", "Addition", 1);
        r *= 7;  // r==56
        CheckWithConversions(r, Rational(56), "56.0", "56", "Multiplication", 1);
        BigInteger b = 15;
        r /= 2;
        CheckWithConversions(-r, Rational(-28), "-28.0", "-28", "Division", 1);
        (r /= 4) -= b;  // r = 7-15=-8
        CheckWithConversions(-r, Rational(8), "8.0", "8", "Negation", 1);
    }
    {
        Rational s = 4 * 3 * 7 * 13 * 19 * 41 * 43 * 11;  // 2^2×3×7×13×19×41×43×11
        Rational t = -17 * 13 * 23 * 79;
        s *= s * s, t *= t * t;
        Rational q = s / t;
        Check(q.ToString() == "-29650611427828166204352/29472131485369",
              "Multiplication or division");
        Check(q / 1000000000 < 1, "Arithmetics or double conversion");
        Check(0 / q == 0, "Multiplication by zero");
        q *= t / s;
        CheckWithConversions(q, Rational(1), "1.0", "1", "Multiply by inverse element", 1);
    }
    {
        Rational s = Rational(85) / 37;
        Rational t = Rational(29) / BigInteger(-163);
        s += t;
        t = 1;
        for (int i = 0; i < 15; ++i) {
            t *= s;
        }
        Check((1 / t).ToString() ==
                  "507972178875842800075597772950831264898404875587494819951"
                  "/39717526884730183825748150063721595142668632339785349901549568",
              "Multiplication or division");
    }
}

TEST(Rational, AsDecimalTest) {
    {
        Rational s = 4 * 3 * 7 * 13 * 19 * 41 * 43 * 11;
        Rational t = s - 25;  // t=402365939
        ((s = 1000000007) *= 1000000009) *= 2147483647;

        Check((s / t).AsDecimal(10) == "5337140829307966068.3989202202",
              "Arithmetics with reassignment");
        t = -t;
        Check((t / s).AsDecimal(25) == "-0.0000000000000000001873662",
              "Arithmetics or precision error");
    }
}

TEST(Rational, IOBaseTest) {
    {
        std::istringstream iss("-145.6545 -291309/2000");
        Rational x;
        iss >> x;
        Check(x.ToString() == "-291309/2000", "Wrong operator>>");
        iss >> x;
        Check(x.AsDecimal(4) == "-145.6545", "Wrong operator>>");
        x = "-145.6545"_rat;
        Check(x.ToString() == "-291309/2000",
              "Wrong operator"
              "_rat");
        x = -145.6545_rat;
        Check(x.ToString() == "-291309/2000",
              "Wrong operator"
              "_rat");
        x = "-291309/2000"_rat;
        Check(x.AsDecimal(4) == "-145.6545", "Wrong operator>>");
    }
}

TEST(Rational, CompareBaseTest) {
    {
        Rational x = 12.45_rat, y = 12.46_rat;
        double a = 12.45, b = 12.46;
        Check((x <=> y) == (a <=> b), "Wrong operator<=>");
        b = -b;
        y = -y;
        Check((x <=> y) == (a <=> b), "Wrong operator<=>");
    }
    {
        Rational x = "1/2"_rat, y = "0.5"_rat;
        double a = 0.5, b = 0.5;
        Check((x == y) == (a == b), "Wrong operator==");
        b *= b;
        y *= y;
        Check((x != y) == (a != b), "Wrong operator!=");
    }
}

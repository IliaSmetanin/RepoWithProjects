
#include "rational.hpp"

#include <sstream>
#include <string>

#include <gtest/gtest.h>

TEST(BeginRational, SimpleOperations) {
    {
        Rational var1(100000000000, 7000000);
        Rational var2("1000009237450245712057862341249"_bi, "9245712957125752592578725"_bi);
        Rational var3("24384101234184790131"_bi, "4127489174379812798429134190234781290402814"_bi);
    }
}

TEST(BeginRational, InOut) {
    {
        std::istringstream iss(
            std::string("1756851983451793456178913577134578743568.2349877919501935097190501945"));
        Rational var;
        iss >> var;
    }
    {
        std::istringstream iss(
            std::string("1756851983451793456178913577134578743568.2349877919501935097190501945"));
        Rational var;
        iss >> var;
    }
}

TEST(RationalBasic, DefaultConstructor) {
    {
        Rational r;
        EXPECT_EQ(r.GetNumerator(), 0);
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_FALSE(r.GetSign());
    }
    // {
    //     Rational r1(-9223372036854775808LL);
    //     Rational r2("-9223372036854775808");
    //     EXPECT_EQ(r1, r2);
    // }
}

TEST(RationalBasic, IntegerConstructor) {
    Rational r1(42);
    EXPECT_EQ(r1.GetNumerator(), 42);
    EXPECT_EQ(r1.GetDenominator(), 1);
    EXPECT_FALSE(r1.GetSign());

    Rational r2(-42);
    EXPECT_EQ(r2.GetNumerator(), 42);
    EXPECT_EQ(r2.GetDenominator(), 1);
    EXPECT_TRUE(r2.GetSign());

    Rational r3(0);
    EXPECT_EQ(r3.GetNumerator(), 0);
    EXPECT_EQ(r3.GetDenominator(), 1);
    EXPECT_FALSE(r3.GetSign());
}

TEST(RationalBasic, BigIntegerConstructor) {
    BigInteger num("12345678901234567890");
    BigInteger den("98765432109876543210");
    Rational r(num, den);

    EXPECT_LT(r.GetNumerator(), num);
    EXPECT_LT(r.GetDenominator(), den);
    EXPECT_EQ(r.GetNumerator() * den, r.GetDenominator() * num);
}

TEST(RationalArithmetic, SimpleOperations) {
    Rational a(1, 2);
    Rational b(1, 3);

    Rational c = a + b;
    EXPECT_EQ(c.GetNumerator(), 5);
    EXPECT_EQ(c.GetDenominator(), 6);

    Rational d = a - b;
    EXPECT_EQ(d.GetNumerator(), 1);
    EXPECT_EQ(d.GetDenominator(), 6);

    Rational e = a * b;
    EXPECT_EQ(e.GetNumerator(), 1);
    EXPECT_EQ(e.GetDenominator(), 6);

    Rational f = a / b;
    EXPECT_EQ(f.GetNumerator(), 3);
    EXPECT_EQ(f.GetDenominator(), 2);
}

TEST(RationalArithmetic, CompoundOperations) {
    Rational a(2, 3);
    Rational b(3, 4);

    a += b;
    EXPECT_EQ(a.GetNumerator(), 17);
    EXPECT_EQ(a.GetDenominator(), 12);

    a -= Rational(1, 2);
    EXPECT_EQ(a.GetNumerator(), 11);
    EXPECT_EQ(a.GetDenominator(), 12);

    a *= Rational(2, 11);
    EXPECT_EQ(a.GetNumerator(), 1);
    EXPECT_EQ(a.GetDenominator(), 6);

    a /= Rational(1, 3);
    EXPECT_EQ(a.GetNumerator(), 1);
    EXPECT_EQ(a.GetDenominator(), 2);
}

TEST(RationalComparison, SimpleComparisons) {
    Rational a(1, 2);
    Rational b(1, 3);
    Rational c(1, 2);
    Rational d(-1, 2);
    Rational e(0);

    EXPECT_GT(a, b);
    EXPECT_LT(b, a);
    EXPECT_EQ(a, c);
    EXPECT_NE(a, b);

    EXPECT_LT(d, e);
    EXPECT_LT(d, a);
    EXPECT_GT(a, d);

    EXPECT_EQ(e, Rational(0));
}

TEST(RationalComparison, LargeNumbers) {
    BigInteger big_num1("123456789012345678901234567890");
    BigInteger big_den1("987654321098765432109876543210");
    BigInteger big_num2("123456789012345678901234567891");
    BigInteger big_den2("987654321098765432109876543210");

    Rational r1(big_num1, big_den1);
    Rational r2(big_num2, big_den2);

    EXPECT_LT(r1, r2);
    EXPECT_GT(r2, r1);
    EXPECT_NE(r1, r2);
}

TEST(RationalIO, SimpleInputOutput) {
    std::istringstream iss1("42");
    Rational r1;
    iss1 >> r1;
    EXPECT_EQ(r1.GetNumerator(), 42);
    EXPECT_EQ(r1.GetDenominator(), 1);

    std::istringstream iss2("-3/4");
    Rational r2;
    iss2 >> r2;
    EXPECT_EQ(r2.GetNumerator(), 3);
    EXPECT_EQ(r2.GetDenominator(), 4);
    EXPECT_TRUE(r2.GetSign());

    std::istringstream iss3("0.75");
    Rational r3;
    iss3 >> r3;
    EXPECT_EQ(r3.GetNumerator(), 3);
    EXPECT_EQ(r3.GetDenominator(), 4);
    EXPECT_FALSE(r3.GetSign());
}

TEST(RationalIO, LargeNumberInput) {
    std::string big_int_str = "1234567890123456789012345678901234567890";
    std::istringstream iss1(big_int_str);
    Rational r1;
    iss1 >> r1;
    EXPECT_EQ(r1.GetNumerator(), BigInteger(big_int_str));
    EXPECT_EQ(r1.GetDenominator(), 1);
    EXPECT_FALSE(r1.GetSign());

    std::string big_frac_str = big_int_str + "/" + big_int_str + "1";
    std::istringstream iss2(big_frac_str);
    Rational r2;
    iss2 >> r2;
    EXPECT_LT(r2, Rational(1));
    EXPECT_GT(r2, Rational(0));
}

TEST(RationalIO, DecimalLargeNumbers) {
    std::string big_decimal =
        "1756851983451793456178913577134578743568.2349877919501935097190501945";
    std::istringstream iss(big_decimal);
    Rational r;
    iss >> r;
    EXPECT_FALSE(r.GetSign());
    BigInteger int_part("1756851983451793456178913577134578743568");
    EXPECT_GT(r, Rational(int_part));

    EXPECT_LT(r, Rational(int_part + 1));
}

TEST(RationalIO, OutputToString) {
    Rational r1(3, 4);
    EXPECT_EQ(r1.ToString(), "3/4");

    Rational r2(-3, 4);
    EXPECT_EQ(r2.ToString(), "-3/4");

    Rational r3(6, 4);
    EXPECT_EQ(r3.ToString(), "3/2");

    Rational r4(5);
    EXPECT_EQ(r4.ToString(), "5");

    Rational r5(0);
    EXPECT_EQ(r5.ToString(), "0");
}

TEST(RationalEdgeCases, ZeroOperations) {
    Rational zero;
    Rational a(3, 4);

    EXPECT_EQ(zero + a, a);
    EXPECT_EQ(a + zero, a);
    EXPECT_EQ(zero - a, -a);
    EXPECT_EQ(a - zero, a);
    EXPECT_EQ(zero * a, zero);
    EXPECT_EQ(a * zero, zero);
    EXPECT_EQ(zero / a, zero);
}

TEST(RationalEdgeCases, NegativeNumbers) {
    Rational a(-1, 2);
    Rational b(1, 3);
    Rational c(-1, 3);

    EXPECT_EQ(a + b, Rational(-1, 6));
    EXPECT_EQ(a + c, Rational(-5, 6));
    EXPECT_EQ(a - b, Rational(-5, 6));
    EXPECT_EQ(a - c, Rational(-1, 6));
    EXPECT_EQ(a * b, Rational(-1, 6));
    EXPECT_EQ(a * c, Rational(1, 6));
    EXPECT_EQ(a / b, Rational(-3, 2));
    EXPECT_EQ(a / c, Rational(3, 2));
}

TEST(RationalEdgeCases, VeryLargeDecimal) {
    std::string long_decimal = "0." + std::string(50, '1') + std::string(50, '0');
    std::istringstream iss(long_decimal);
    Rational r;
    iss >> r;

    EXPECT_FALSE(r.GetSign());
    EXPECT_GT(r, Rational(0));
    EXPECT_LT(r, Rational(1));

    std::ostringstream oss;
    oss << r;
    EXPECT_FALSE(oss.str().empty());
}

TEST(RationalSpecial, PiApproximation) {
    Rational pi_approx(355, 113);

    EXPECT_GT(pi_approx, Rational(3, 1));
    EXPECT_LT(pi_approx, Rational(4, 1));

    Rational better_approx(103993, 33102);
    EXPECT_LT(better_approx, pi_approx);
}

TEST(RationalLargeNumbers, Arithmetic) {
    {
        Rational a("12389057102597807255909021579209553297847"_bi,
                   "1429384819270097812404204991237899"_bi);
        Rational b("557507569616901326515905971064429898403115"_bi,
                   "64322316867154401558189224605705455"_bi);
        ASSERT_EQ(a, b);
    }

    {
        Rational a("123456789012345678901234567890"_bi, "987654321098765432109876543210"_bi);
        Rational b("555555555555555555555555555555"_bi, "999999999999999999999999999999"_bi);

        BigInteger expected_num = BigInteger("123456789012345678901234567890"_bi) *
                                  BigInteger("555555555555555555555555555555"_bi);
        BigInteger expected_den = BigInteger("987654321098765432109876543210"_bi) *
                                  BigInteger("999999999999999999999999999999"_bi);

        Rational result = a * b;
        EXPECT_EQ(result.GetNumerator() * expected_den, result.GetDenominator() * expected_num);
    }
}

TEST(AsDecimal, StringStreams) {
    {
        std::string a_str = "-1232384921.42462142747896580210";
        std::istringstream iss(a_str);
        std::ostringstream oss;
        Rational a;
        iss >> a;
        oss << a.AsDecimal(20);
        ASSERT_EQ(a_str, oss.str());
    }

    {
        std::string a_str = "-10000000000.000000000000000000000000000000000000000000000000001";
        std::istringstream iss(a_str);
        std::ostringstream oss;
        Rational a;
        iss >> a;
        oss << a.AsDecimal(51);
        ASSERT_EQ(a_str, oss.str());
    }

    {
        std::string a_str = "-0.000000000001";
        std::string compare = "0.0000";
        std::istringstream iss(a_str);
        std::ostringstream oss;
        Rational a;
        iss >> a;
        oss << a.AsDecimal(4);
        ASSERT_EQ(compare, oss.str());
    }

    {
        std::string a_str = "1238714.921451795";
        std::string compare = "1238714.9214518";
        std::istringstream iss(a_str);
        std::ostringstream oss;
        Rational a;
        iss >> a;
        oss << a.AsDecimal(7);
        ASSERT_EQ(compare, oss.str());
    }

    {
        std::string a_str = "-99999.9999999999999999999999999999";
        std::string compare = "-100000.0";
        std::istringstream iss(a_str);
        std::ostringstream oss;
        Rational a;
        iss >> a;
        oss << a.AsDecimal(1);
        ASSERT_EQ(compare, oss.str());
    }
}

TEST(RationalFractionalStr, Basic) {
    Rational r1(1, 2);
    EXPECT_EQ(r1.FractionalStr(1), "5");
    EXPECT_EQ(r1.FractionalStr(3), "500");
    EXPECT_EQ(r1.FractionalStr(5), "50000");
}

TEST(RationalFractionalStr, Third) {
    Rational r1(1, 3);
    EXPECT_EQ(r1.FractionalStr(1), "3");
    EXPECT_EQ(r1.FractionalStr(3), "333");
    EXPECT_EQ(r1.FractionalStr(6), "333333");
}

TEST(RationalFractionalStr, Simple) {
    {
        Rational r1(0, 5);
        EXPECT_EQ(r1.FractionalStr(5), "00000");
    }

    {
        Rational r1(22, 7);
        EXPECT_EQ(r1.FractionalStr(1), "1");
        EXPECT_EQ(r1.FractionalStr(2), "14");
        EXPECT_EQ(r1.FractionalStr(6), "142857");
    }

    {
        Rational r1(1, 8);
        EXPECT_EQ(r1.FractionalStr(1), "1");
        EXPECT_EQ(r1.FractionalStr(3), "125");
        EXPECT_EQ(r1.FractionalStr(5), "12500");
    }
}

TEST(RationalDouble, Compare) {
    {
        std::string a_str = "-124278214.9012834898";
        double compare = -124278214.901283490;
        std::istringstream iss(a_str);
        std::ostringstream oss;
        Rational a;
        iss >> a;
        ASSERT_EQ(compare, static_cast<double>(a));
    }

    {
        std::string a_str = "0.5";
        double compare = 0.5;
        std::istringstream iss(a_str);
        Rational a;
        iss >> a;
        ASSERT_DOUBLE_EQ(compare, static_cast<double>(a));
    }

    {
        std::string a_str = "-0.75";
        double compare = -0.75;
        std::istringstream iss(a_str);
        Rational a;
        iss >> a;
        ASSERT_DOUBLE_EQ(compare, static_cast<double>(a));
    }

    {
        std::string a_str = "-14881328.234791249";
        double compare = -14881328.234791249;
        std::istringstream iss(a_str);
        Rational a;
        iss >> a;
        ASSERT_DOUBLE_EQ(compare, static_cast<double>(a));
    }

    {
        std::string a_str = "123456789.987654321";
        double compare = 123456789.987654321;
        std::istringstream iss(a_str);
        Rational a;
        iss >> a;
        ASSERT_DOUBLE_EQ(compare, static_cast<double>(a));
    }

    {
        std::string a_str = "-0.493824713";
        double compare = -0.493824713;
        std::istringstream iss(a_str);
        Rational a;
        iss >> a;
        ASSERT_DOUBLE_EQ(compare, static_cast<double>(a));
    }
}

TEST(RationalLiterals, IntegerLiteral) {
    {
        Rational r = 42_rat;
        EXPECT_EQ(r.GetNumerator(), 42);
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_FALSE(r.GetSign());
    }

    {
        Rational r = 0_rat;
        EXPECT_EQ(r.GetNumerator(), 0);
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_FALSE(r.GetSign());
    }

    {
        Rational r = 999999999_rat;
        EXPECT_EQ(r.GetNumerator(), BigInteger(999999999));
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_FALSE(r.GetSign());
    }
}

TEST(RationalLiterals, StringLiteralInteger) {
    {
        Rational r = "42"_rat;
        EXPECT_EQ(r.GetNumerator(), 42);
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_FALSE(r.GetSign());
    }

    {
        Rational r = "-42"_rat;
        EXPECT_EQ(r.GetNumerator(), 42);
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_TRUE(r.GetSign());
    }

    {
        Rational r = "0"_rat;
        EXPECT_EQ(r.GetNumerator(), 0);
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_FALSE(r.GetSign());
    }

    {
        Rational r = "-0"_rat;
        EXPECT_EQ(r.GetNumerator(), 0);
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_FALSE(r.GetSign());
    }
}

TEST(RationalLiterals, StringLiteralFraction) {
    {
        Rational r = "1/2"_rat;
        EXPECT_EQ(r.GetNumerator(), 1);
        EXPECT_EQ(r.GetDenominator(), 2);
        EXPECT_FALSE(r.GetSign());
    }

    {
        Rational r = "-1/2"_rat;
        EXPECT_EQ(r.GetNumerator(), 1);
        EXPECT_EQ(r.GetDenominator(), 2);
        EXPECT_TRUE(r.GetSign());
    }

    {
        Rational r = "3/4"_rat;
        EXPECT_EQ(r.GetNumerator(), 3);
        EXPECT_EQ(r.GetDenominator(), 4);
        EXPECT_FALSE(r.GetSign());
    }

    {
        Rational r = "-15/6"_rat;
        EXPECT_EQ(r.GetNumerator(), 5);
        EXPECT_EQ(r.GetDenominator(), 2);
        EXPECT_TRUE(r.GetSign());
    }

    {
        Rational r = "0/1"_rat;
        EXPECT_EQ(r.GetNumerator(), 0);
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_FALSE(r.GetSign());
    }

    {
        Rational r = "0/42"_rat;
        EXPECT_EQ(r.GetNumerator(), 0);
        EXPECT_EQ(r.GetDenominator(), 1);
        EXPECT_FALSE(r.GetSign());
    }
}
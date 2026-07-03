
#include "../biginteger.hpp"

#include <gtest/gtest.h>

TEST(YourTest, Test) {
    BigInteger b(-9223372036854775807);
}

TEST(BigInteger, CheckSummingTest) {
    {
        BigInteger a("999999");
        BigInteger b("1");
        BigInteger c = a + b;
        ASSERT_EQ(c, BigInteger("1000000")) << "CheckSumming failed on simple addition";
    }
}

TEST(BigInteger, AdditionBasicTest) {
    {
        BigInteger a("123");
        BigInteger b("456");
        a += b;
        ASSERT_EQ(a, BigInteger("579")) << "Simple addition failed";
    }

    {
        BigInteger a("999");
        BigInteger b("1");
        a += b;
        ASSERT_EQ(a, BigInteger("1000")) << "Addition with carry failed";
    }

    {
        BigInteger a("12345678901234567890");
        BigInteger b("98765432109876543210");
        a += b;
        ASSERT_EQ(a, BigInteger("111111111011111111100")) << "Large addition failed";
    }
}

TEST(BigInteger, SubtractionBasicTest) {
    {
        BigInteger a("456");
        BigInteger b("123");
        a -= b;
        ASSERT_EQ(a, BigInteger("333")) << "Simple subtraction failed";
    }

    {
        BigInteger a("1000");
        BigInteger b("1");
        a -= b;
        ASSERT_EQ(a, BigInteger("999")) << "Subtraction with borrow failed";
    }
}

TEST(BigInteger, SignOperationsTest) {
    {
        BigInteger a("100");
        BigInteger b("-50");
        a += b;
        ASSERT_EQ(a, BigInteger("50")) << "Positive + Negative failed";
    }

    {
        BigInteger a("-100");
        BigInteger b("50");
        a += b;
        ASSERT_EQ(a, BigInteger("-50")) << "Negative + Positive failed";
    }

    {
        BigInteger a("-100");
        BigInteger b("-50");
        a += b;
        ASSERT_EQ(a, BigInteger("-150")) << "Negative + Negative failed";
    }

    {
        BigInteger a("100");
        BigInteger b("-50");
        a -= b;
        ASSERT_EQ(a, BigInteger("150")) << "Subtraction of negative failed";
    }
}

TEST(BigInteger, OperatorPlusMinusTest) {
    {
        BigInteger a("10");
        BigInteger b("20");
        BigInteger c("30");
        BigInteger result = a + b - c;
        ASSERT_EQ(result, BigInteger("0")) << "Combined +- operations failed";
    }

    {
        BigInteger a("100");
        BigInteger b("200");
        BigInteger c("300");
        BigInteger d("400");

        a += b;
        a -= c;
        a += d;

        ASSERT_EQ(a, BigInteger("400")) << "Chain of operations failed";
    }
}

TEST(BigInteger, OverflowEdgeCases) {
    {
        BigInteger a(std::to_string(999999));
        BigInteger b("1");
        a += b;
        ASSERT_EQ(a, BigInteger(std::to_string(1000000))) << "kBit boundary addition failed";
    }

    {
        BigInteger a("0");
        for (int i = 0; i < 1000; ++i) {
            a += BigInteger("1");
        }
        ASSERT_EQ(a, BigInteger("1000")) << "Repeated addition failed";
    }
}

TEST(BigInteger, ComparisonWithZero) {
    {
        BigInteger a("123");
        BigInteger zero("0");

        a += zero;
        ASSERT_EQ(a, BigInteger("123")) << "Adding zero failed";

        a -= zero;
        ASSERT_EQ(a, BigInteger("123")) << "Subtracting zero failed";

        zero -= a;
        ASSERT_EQ(zero, BigInteger("-123")) << "0 - positive failed";
        ASSERT_TRUE(zero.GetSign()) << "Should be negative";
    }
}

TEST(BigInteger, ExtremeLargeNumbers) {
    {
        BigInteger a("99999999999999999999999999999999999999999999999999");
        BigInteger b("11111111111111111111111111111111111111111111111111");
        BigInteger expected("111111111111111111111111111111111111111111111111110");
        ASSERT_EQ(a + b, expected) << "Extreme large addition failed";
    }

    {
        BigInteger a("100000000000000000000000000000000000000000000000000");
        BigInteger b("99999999999999999999999999999999999999999999999999");
        BigInteger expected("1");
        ASSERT_EQ(a - b, expected) << "Extreme large subtraction failed";
    }

    {
        BigInteger a("123456789012345678901234567890");
        BigInteger b("987654321098765432109876543210");
        BigInteger expected("121932631137021795226185032733622923332237463801111263526900");
        ASSERT_EQ(a * b, expected) << "Extreme large multiplication failed";
    }

    {
        BigInteger a("12345678901234567890123456789012345678901234567890");
        BigInteger b("12345678901234567890");
        BigInteger expected("1000000000000000000010000000000");
        ASSERT_EQ(a / b, expected) << "Extreme large division failed";
    }

    {
        BigInteger a("12345678901234567890123456789012345678901234567890");
        BigInteger b("12345678901234567891");
        BigInteger expected("12345597063469128320");
        ASSERT_EQ(a % b, expected) << "Extreme large modulo failed";
    }

    {
        std::string huge_num;
        for (int i = 0; i < 100; ++i) {
            huge_num += "999999";
        }

        BigInteger a(huge_num);
        BigInteger b("1");
        BigInteger c = a + b;

        ASSERT_TRUE(c > a) << "Addition with max numbers failed";
    }

    {
        BigInteger a("10000000000000000000000000000000000000000000000000");
        BigInteger b("9999999999999999999999999999999999999999999999999");
        BigInteger sum = a + b;
        BigInteger diff = a - b;

        ASSERT_EQ(sum, BigInteger("19999999999999999999999999999999999999999999999999"))
            << "Addition with different digit counts failed";
        ASSERT_EQ(diff, BigInteger("1")) << "Subtraction with different digit counts failed";
    }

    {
        BigInteger a("1000000000000000000000000000000000000000000000000000000000001");
        BigInteger b("999999999999999999999999999999999999999999999999999999999999");
        BigInteger q = a / b;
        BigInteger r = a % b;

        ASSERT_EQ(q, BigInteger("1")) << "Addition with different digit counts failed";
        ASSERT_EQ(r, BigInteger("2")) << "Subtraction with different digit counts failed";
    }

    {
        BigInteger a("123456789");
        std::string many_zeros(50, '0');
        BigInteger b("1" + many_zeros);

        BigInteger product = a * b;
        std::string expected = "123456789" + many_zeros;

        ASSERT_EQ(product, BigInteger(expected)) << "Multiplication by power of 10 failed";
    }

    {
        std::string many_zeros(50, '0');
        BigInteger a("123456789" + many_zeros);
        BigInteger b("1" + many_zeros);

        BigInteger quotient = a / b;
        ASSERT_EQ(quotient, BigInteger("123456789")) << "Division by power of 10 failed";
    }

    {
        BigInteger a("99999999999999999999999999999999999999999999999999");
        BigInteger b("11111111111111111111111111111111111111111111111111");
        BigInteger c("88888888888888888888888888888888888888888888888888");

        BigInteger result = (a + b) - c;
        BigInteger expected("22222222222222222222222222222222222222222222222222");

        ASSERT_EQ(result, expected) << "Combined operations with large numbers failed";
    }

    {
        BigInteger a("100000000000000000000000000000000");
        BigInteger b("100000000000000000000000000000000");
        BigInteger expected("10000000000000000000000000000000000000000000000000000000000000000");

        ASSERT_EQ(a * b, expected) << "Multiplication overflow test failed";
    }

    {
        BigInteger a("-99999999999999999999999999999999999999999999999999");
        BigInteger b("-11111111111111111111111111111111111111111111111111");
        BigInteger expected("-111111111111111111111111111111111111111111111111110");

        ASSERT_EQ(a + b, expected) << "Large negative addition failed";
    }

    {
        BigInteger a("12345678901234567890123456789012345678901234567890");
        BigInteger neg_a = -a;

        ASSERT_TRUE(neg_a.GetSign()) << "Large number should become negative";
        ASSERT_EQ(a + neg_a, BigInteger("0")) << "Large number + (-itself) should be 0";
    }

    {
        BigInteger dividend("123456789012345678901234567890123456789012345678901234567890");
        BigInteger divisor("123456789012345678901234567890");
        BigInteger quotient = dividend / divisor;
        BigInteger remainder = dividend % divisor;

        ASSERT_EQ(quotient * divisor + remainder, dividend) << "Large division invariant failed";
    }

    {
        std::string thousand_digits;
        for (int i = 0; i < 1000; ++i) {
            thousand_digits += std::to_string(i % 10);
        }

        BigInteger a(thousand_digits);
        BigInteger b("1");
        BigInteger c = a + b;

        ASSERT_TRUE(c > a) << "1000-digit number addition failed";
    }
}

TEST(BigInteger, BoundaryCases) {
    {
        BigInteger a("999999");
        BigInteger b("1");
        BigInteger sum = a + b;

        ASSERT_EQ(sum, BigInteger("1000000")) << "kBit boundary addition failed";
    }

    {
        BigInteger a("12345678901234567890123456789012345678901234567890");
        BigInteger zero("0");

        ASSERT_EQ(a * zero, BigInteger("0")) << "Large number * 0 failed";
        ASSERT_EQ(zero * a, BigInteger("0")) << "0 * large number failed";
    }

    {
        BigInteger a("12345678901234567890123456789012345678901234567890");
        BigInteger one("1");

        ASSERT_EQ(a / one, a) << "Large number / 1 failed";
        ASSERT_EQ(a % one, BigInteger("0")) << "Large number % 1 failed";
    }

    {
        BigInteger a("99999999999999999999999999999999999999999999999999");
        BigInteger b("100000000000000000000000000000000000000000000000000");

        ASSERT_TRUE(a < b) << "Comparison of large numbers failed (a < b)";
        ASSERT_TRUE(b > a) << "Comparison of large numbers failed (b > a)";
        ASSERT_TRUE(a != b) << "Comparison of large numbers failed (a != b)";
    }

    {
        BigInteger a("99999999999999999999999999999999999999999999999999");
        BigInteger b = a;

        ++a;
        --a;

        ASSERT_EQ(a, b) << "Increment/decrement of large number failed";
    }
}

TEST(BigIntegerTest, RemainderPositivePositive) {
    BigInteger a("123456789012345678901234567890");
    BigInteger b("987654321");
    BigInteger r = a % b;
    BigInteger expected("574845669");
    EXPECT_EQ(r, expected);
    EXPECT_EQ(r.GetSign(), false);
}

TEST(BigIntegerTest, RemainderPositiveNegative) {
    BigInteger a("123456789012345678901234567890");
    BigInteger b("-987654321");
    BigInteger r = a % b;
    BigInteger expected("574845669");
    EXPECT_EQ(r, expected);
    EXPECT_EQ(r.GetSign(), false);
}

TEST(BigIntegerTest, RemainderNegativePositive) {
    BigInteger a("-123456789012345678901234567890");
    BigInteger b("987654321");
    BigInteger r = a % b;
    BigInteger expected("-574845669");
    EXPECT_EQ(r, expected);
    EXPECT_EQ(r.GetSign(), true);
}

TEST(BigIntegerTest, RemainderNegativeNegative) {
    BigInteger a("-123456789012345678901234567890");
    BigInteger b("-987654321");
    BigInteger r = a % b;
    BigInteger expected("-574845669");
    EXPECT_EQ(r, expected);
    EXPECT_EQ(r.GetSign(), true);
}

TEST(BigIntegerTest, RemainderWithZeroRemainder) {
    BigInteger a("123456789012345678901234567890");
    BigInteger b("1234567890");
    BigInteger r = a % b;
    BigInteger expected(0);
    EXPECT_EQ(r, expected);
    EXPECT_EQ(r.GetSign(), false);
}

TEST(BigIntegerTest, RemainderPropertyCheck) {
    {
        BigInteger a("1844674407370955161518446744073709551615");
        BigInteger b("1234567890123456789");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_TRUE(r >= BigInteger(0));
        EXPECT_TRUE(r < b.Abs());
    }

    {
        BigInteger a("10000000000000000000000000011");
        BigInteger b("1000000");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
    }

    {
        BigInteger a("999000999000999000999000999000999000999");
        BigInteger b("999000");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(q, BigInteger("1000001000001000001000001000001000"));
    }

    {
        BigInteger a("999000999000999000999000999000999000999");
        BigInteger b("-1");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("0"));
    }

    {
        BigInteger a("999000999000999000999000999000999000999");
        BigInteger b("1");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("0"));
    }

    {
        BigInteger a("999000999000999000999000999000999000990");
        BigInteger b("2");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("0"));
    }

    {
        BigInteger a("999000999000999000999000999000999000999");
        BigInteger b("2");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("1"));
    }

    {
        BigInteger a("-999000999000999000999000999000999000999");
        BigInteger b("9");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("0"));
        EXPECT_EQ(r.GetSign(), false);
    }

    {
        BigInteger a("-999000999000999000999000999000999000");
        BigInteger b("999000999");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("0"));
        EXPECT_EQ(r.GetSign(), false);
    }

    {
        BigInteger a("-999000999000999000999000999000999000");
        BigInteger b("-3");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("0"));
        EXPECT_EQ(r.GetSign(), false);
    }

    {
        BigInteger a("999000999000999000999000999000999000");
        BigInteger b("-3");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("0"));
        EXPECT_EQ(r.GetSign(), false);
    }

    {
        BigInteger a("-999000999000999000999000999000999000");
        BigInteger b("9");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("0"));
        EXPECT_EQ(r.GetSign(), false);
    }

    {
        BigInteger a("999000999000999000999000999000999000");
        BigInteger b("-4");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r.GetSign(), false);
    }

    {
        BigInteger a("-999999999999999999999999999999999999");
        BigInteger b("-333333333333");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r, BigInteger("0"));
        EXPECT_EQ(r.GetSign(), false);
    }

    {
        BigInteger a("-999999999999999999999999999999999999");
        BigInteger b("333333333332");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r.GetSign(), 1);
    }

    {
        BigInteger a("999999999999999999999999999999999999");
        BigInteger b("-333333333334");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r.GetSign(), 0);
    }

    {
        BigInteger a("999999999999999999999999999999999999");
        BigInteger b("333333333330");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r.GetSign(), 0);
    }

    {
        BigInteger a("999999999999999999999999999999999998");
        BigInteger b("999999999999999999999999999999999999");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(r.GetSign(), 0);
        EXPECT_EQ(r, "999999999999999999999999999999999998"_bi);
    }

    {
        BigInteger a("789789789789789789789789789789789789789789");
        BigInteger b("789789789789");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(q, "1000000000001000000000001000000"_bi);
    }

    {
        BigInteger a("999999999999999999999999999999999999999999999999999999999999999");
        BigInteger b("9999999");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
    }

    {
        BigInteger a("900000000000000175033");
        BigInteger b("90000000");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
        EXPECT_EQ(q, "10000000000000"_bi);
    }

    {
        BigInteger a("999999999999999999999999999999999999999999999999999999999999999");
        BigInteger b("99999999");
        BigInteger q = a / b;
        BigInteger r = a % b;
        BigInteger check = q * b + r;
        EXPECT_EQ(a, check);
    }
}

TEST(BigIntegerTest, RemainderLargeNumbers) {
    BigInteger a("9999999999999999999999999999999999999999");
    BigInteger b("5555555555555555555555555555555");
    BigInteger r = a % b;
    BigInteger expected("999999999");
    EXPECT_EQ(r, expected);
}

TEST(BigIntegerTest, RemainderCheck) {
    BigInteger a("1844674407370955161518446744073709551615");
    BigInteger b("1234567890123456789");
    BigInteger q = a / b;
    BigInteger r = a % b;
    BigInteger check = q * b + r;
    EXPECT_EQ(a, check);
    EXPECT_TRUE(r >= BigInteger(0));
    EXPECT_TRUE(r < b.Abs());
}

TEST(BigIntegerTest, RemainderSmallDivisor) {
    BigInteger a("9999999999999999999999999999999999999999");
    BigInteger b(7);
    BigInteger r = a % b;
    BigInteger expected(3);
    EXPECT_EQ(r, expected);
}

TEST(BigIntegerTest, RemainderEqualNumbers) {
    BigInteger a("12345678901234567890");
    BigInteger r = a % a;
    BigInteger expected(0);
    EXPECT_EQ(r, expected);
}

TEST(BigIntegerTest, RemainderDivisorLarger) {
    BigInteger a("123");
    BigInteger b("123456");
    BigInteger r = a % b;
    EXPECT_EQ(r, a);
    EXPECT_EQ(r.GetSign(), false);
}

TEST(BigIntegerTest, RemainderNegativeDivisorLarger) {
    BigInteger a("-123");
    BigInteger b("123456");
    BigInteger r = a % b;
    EXPECT_EQ(r, a);
    EXPECT_EQ(r.GetSign(), true);
}

TEST(BigIntegerTest, RemainderWithOne) {
    BigInteger a("123456789012345678901234567890");
    BigInteger b(1);
    BigInteger r = a % b;
    BigInteger expected(0);
    EXPECT_EQ(r, expected);
}

TEST(BigIntegerTest, RemainderWithMinusOne) {
    BigInteger a("-123456789012345678901234567890");
    BigInteger b(-1);
    BigInteger r = a % b;
    BigInteger expected(0);
    EXPECT_EQ(r, expected);
}

TEST(BigIntegerTest, RemainderExtremeCase) {
    BigInteger a("9999999999999999999999999999999999999999");
    BigInteger b("9999999999999999999999999999999999999998");
    BigInteger r = a % b;
    BigInteger expected(1);
    EXPECT_EQ(r, expected);
}

TEST(BigIntegerTest, RemainderPowerOfTwoBase) {
    BigInteger a("10000000000000000000000000000000000000000");
    BigInteger b("9999999999");
    BigInteger r = a % b;
    BigInteger q = a / b;
    BigInteger check = q * b + r;
    EXPECT_EQ(a, check);
}

TEST(BigIntegerTest, RemainderMultipleZeros) {
    BigInteger a("10000000000000000000000000000000000000000");
    BigInteger b("10000000000");
    BigInteger r = a % b;
    BigInteger expected(0);
    EXPECT_EQ(r, expected);
}

TEST(BigIntegerTest, RemainderDivisionIdentity) {
    BigInteger a = BigInteger("123456789") * BigInteger("987654321") + BigInteger("12345");
    BigInteger b("987654321");
    BigInteger r = a % b;
    BigInteger expected("12345");
    EXPECT_EQ(r, expected);
}

TEST(BigIntegerTest, DivisiontTheSame) {
    {
        BigInteger a = BigInteger("999999999999999999999999");
        BigInteger b("999");
        BigInteger expected("1001001001001001001001");
        EXPECT_EQ(a / b, expected);
    }

    {
        BigInteger a = BigInteger("7350000000005000040");
        BigInteger b("350000");
        BigInteger expected("21000000000014");
        BigInteger expected2("100040");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }

    {
        BigInteger a = BigInteger("7350000000005000040");
        BigInteger b("350001");
        BigInteger expected("20999940000185");
        BigInteger expected2("249855");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }

    {
        BigInteger a = BigInteger("7350000000005000040");
        BigInteger b("350001");
        BigInteger expected("20999940000185");
        BigInteger expected2("249855");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }
}

TEST(BigIntegerTest, DivisionSpecial1) {
    {
        BigInteger a = BigInteger("4987746935907841350919470385079854");
        BigInteger b("1");
        BigInteger expected("4987746935907841350919470385079854");
        BigInteger expected2("0");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }

    {
        BigInteger a = BigInteger("4987746935907841350919470385079854");
        BigInteger b("-1");
        BigInteger expected("-4987746935907841350919470385079854");
        BigInteger expected2("0");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }

    {
        BigInteger a = BigInteger("-4987746935907841350919470385079854");
        BigInteger b("-1");
        BigInteger expected("4987746935907841350919470385079854");
        BigInteger expected2("0");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }

    {
        BigInteger a = BigInteger("-4987746935907841350919470385079854");
        BigInteger b("1");
        BigInteger expected("-4987746935907841350919470385079854");
        BigInteger expected2("0");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }
}

TEST(BigIntegerTest, DivisionSpecial2) {
    {
        BigInteger a = BigInteger("4987746935907841350919470385079854");
        BigInteger b("4987746935907841350919470385079854");
        BigInteger expected("1");
        BigInteger expected2("0");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }

    {
        BigInteger a = BigInteger("4987746935907841350919470385079854");
        BigInteger b("-4987746935907841350919470385079854");
        BigInteger expected("-1");
        BigInteger expected2("0");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }

    {
        BigInteger a = BigInteger("-4987746935907841350919470385079854");
        BigInteger b("4987746935907841350919470385079854");
        BigInteger expected("-1");
        BigInteger expected2("0");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }

    {
        BigInteger a = BigInteger("-4987746935907841350919470385079854");
        BigInteger b("-4987746935907841350919470385079854");
        BigInteger expected("1");
        BigInteger expected2("0");
        EXPECT_EQ(a / b, expected);
        EXPECT_EQ(a % b, expected2);
    }
}

TEST(Modulo, ZeroModuloAnythingIsZero) {
    BigInteger a(0);
    BigInteger b(123);
    EXPECT_EQ(a % b, BigInteger(0));
}

TEST(Modulo, SmallNumbersFastPath) {
    BigInteger a(123);
    BigInteger b(7);
    EXPECT_EQ(a % b, BigInteger(4));
}

TEST(Modulo, AfterNormalizationComparison) {
    BigInteger a("1000");
    BigInteger b("999");
    EXPECT_EQ(a % b, BigInteger(1));
}

TEST(BigIntegerTest, MultipleIncrement) {
    BigInteger a(0);
    for (int i = 0; i < 1000; ++i) {
        ++a;
    }
    EXPECT_EQ(a, BigInteger(1000));
}

TEST(BigIntegerTest, MultipleIncrementWithOverflow) {
    BigInteger a("999999");
    ++a;
    EXPECT_EQ(a, BigInteger("1000000"));
    BigInteger b("999999999999");
    for (int i = 0; i < 10; ++i) {
        ++b;
    }
    EXPECT_EQ(b, BigInteger("1000000000009"));
}

TEST(BigIntegerTest, MultipleIncrementLargeNumbers) {
    BigInteger a("12345678901234567890");
    for (int i = 0; i < 10000; ++i) {
        ++a;
    }
    EXPECT_EQ(a, BigInteger("12345678901234577890"));
}

TEST(BigIntegerTest, PostIncrementMultiple) {
    BigInteger a(0);
    for (int i = 0; i < 500; ++i) {
        BigInteger b = a++;
        EXPECT_EQ(b, BigInteger(i));
    }
    EXPECT_EQ(a, BigInteger(500));
}

TEST(BigIntegerTest, IncrementAtBoundary) {
    BigInteger a("999999999999999999999999999999");
    ++a;
    EXPECT_EQ(a, BigInteger("1000000000000000000000000000000"));
}

TEST(BigIntegerTest, IncrementNegativeToPositive) {
    BigInteger a(-5);
    for (int i = 0; i < 10; ++i) {
        ++a;
    }
    EXPECT_EQ(a, BigInteger(5));
}
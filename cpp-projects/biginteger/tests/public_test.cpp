#include "biginteger.hpp"

#include <compare>
#include <cstdlib>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

std::vector<int64_t> random_numbers = {123,
                                       0,
                                       1,
                                       -1,
                                       2,
                                       -2,
                                       3,
                                       -3,
                                       -20,
                                       -1488,
                                       -998'244'353,
                                       100'000'007,
                                       100'000'009,
                                       -12041,
                                       -1000,
                                       123'456'789,
                                       12345,
                                       12'345'678,
                                       1000 - 7,
                                       1,
                                       11,
                                       111,
                                       1'111,
                                       11'111,
                                       111'111,
                                       -1,
                                       -11,
                                       -111,
                                       -1'111,
                                       -11'111,
                                       -111'111,
                                       69,
                                       177013,
                                       77777777777777777ll,
                                       7575798558265554886ll,
                                       5603045260393967524ll,
                                       2919501250909251307ll,
                                       5914273862407361407ll,
                                       -6708010268629577279ll,
                                       -4611447669001924400ll,
                                       -7716060817467639150ll,
                                       -8363054709112384842ll,
                                       -100001ll,
                                       -10001ll,
                                       42};
std::vector<std::string> random_strings = {
    "123123",
    "1231231",
    "1231247128491748",
    "1347038298201831241421",
    "100000000000000000000000",
    "0",
    "-0",
    "-1000000",
    "101894444317458440603421824036688159663989325253819",
    "-23534576554950000000000000009999990000999900000",
};

std::string RemoveLeadingZeroes(const std::string& num) {
    bool is_negative = (num[0] == '-');
    size_t idx = is_negative;
    for (; num[idx] == '0' && idx < num.size() - 1; ++idx) {
    }
    return (is_negative ? "-" : "") + num.substr(idx);
}

std::strong_ordering CompareStringsAsNums(const std::string& lhs, const std::string& rhs) {
    auto one = RemoveLeadingZeroes(lhs);
    auto two = RemoveLeadingZeroes(rhs);
    if (one == "-0" || two == "-0") {
        return one == "-0" ? CompareStringsAsNums("0", two) : CompareStringsAsNums(one, "0");
    }
    size_t idx = 0;
    bool is_negative = (one[idx] == '-');
    if (one[idx] != two[idx]) {
        if (is_negative) {
            return std::strong_ordering::less;
        }
        if (two[idx] == '-') {
            return std::strong_ordering::greater;
        }
    }

    if (one.size() != two.size()) {
        return (one.size() > two.size()) ^ is_negative ? std::strong_ordering::greater
                                                       : std::strong_ordering::less;
    }

    idx += is_negative;
    while (idx < one.size()) {
        if (one[idx] != two[idx]) {
            return (one[idx] > two[idx]) ^ is_negative ? std::strong_ordering::greater
                                                       : std::strong_ordering::less;
        }
        ++idx;
    }
    return std::strong_ordering::equivalent;
}

TEST(BigInteger, InOutTest) {
    {
        for (auto number : random_numbers) {
            std::istringstream iss(std::to_string(number));
            std::ostringstream oss;
            BigInteger temp;
            iss >> temp;
            oss << temp;
            ASSERT_EQ(iss.str(), oss.str()) << "Input or output failed";
            ASSERT_EQ(temp.ToString(), iss.str()) << "toString or input failed";
        }
    }
    {
        for (const auto& number : random_strings) {
            std::istringstream iss(number);
            std::ostringstream oss;
            BigInteger temp;
            iss >> temp;
            oss << temp;
            if (RemoveLeadingZeroes(number) == "-0") {
                ASSERT_EQ(oss.str(), "0") << "Input or output failed";
                ASSERT_EQ(temp, 0) << "Input failed";
            } else {
                ASSERT_EQ(RemoveLeadingZeroes(iss.str()), oss.str()) << "Input or output failed";
                ASSERT_EQ(temp.ToString(), RemoveLeadingZeroes(iss.str()))
                    << "toString or input failed";
            }
        }
    }
    {
        BigInteger a, b;
        std::istringstream iss("26 5");
        iss >> a >> b;
        std::ostringstream oss;
        oss << b << ' ' << a << ' ';
        ASSERT_EQ(oss.str(), "5 26 ") << "Stream input or output failed.";
    }
    {
        BigInteger a, b;
        std::istringstream iss("1000000000000000000000000000000000 -1000000");
        iss >> a >> b;
        std::ostringstream oss;
        oss << b << a;
        ASSERT_EQ(oss.str(), "-10000001000000000000000000000000000000000")
            << "Input or Output failed";
    }
}

TEST(BigInteger, Int64ConstructorTest) {
    for (auto number : random_numbers) {
        ASSERT_EQ(BigInteger(number).ToString(), std::to_string(number))
            << "Constructor from int64_t or toString failed";
    }
}

TEST(BigInteger, UInt64ConstructorTest) {
    for (uint64_t number : random_numbers) {
        ASSERT_EQ(BigInteger(number).ToString(), std::to_string(number))
            << "Constructor from uint64_t or toString failed";
    }
    std::vector<uint64_t> random_unsigned_numbers = {
        16421687186872853490ull, 17371761720683570183ull, 17679776673990048811ull,
        13403181725568852700ull, 12646789336105892068ull, 11316624572219997813ull,
        12586924117551718709ull, 10423349927617686943ull, 18446744073709551615ull};
    for (uint64_t number : random_unsigned_numbers) {
        ASSERT_EQ(BigInteger(number).ToString(), std::to_string(number))
            << "Constructor from uint64_t or toString failed";
    }
}

TEST(BigInteger, OperatorBoolTest) {
    for (auto number : random_numbers) {
        ASSERT_EQ(static_cast<bool>(number), BigInteger(number).operator bool())
            << "operator bool failed";
    }
    for (const auto& str : random_strings) {
        auto x = operator""_bi(str.c_str(), str.size());
        if (!x) {
            ASSERT_EQ(0, std::stoll(str)) << "operator bool failed";
        } else {
            auto tmp = RemoveLeadingZeroes(str);
            ASSERT_TRUE((tmp[0] == '-' ? tmp[1] != '0' : tmp[0] != '0')) << "operator bool failed";
        }
    }
}

TEST(BigInteger, StringConstructorTest) {
    for (const auto& number : random_strings) {
        if (RemoveLeadingZeroes(number) == "-0") {
            continue;
        }
        ASSERT_EQ(BigInteger(number).ToString(), RemoveLeadingZeroes(number))
            << "Constructor from string or toString failed";
    }
}

TEST(BigInteger, LiteralSuffixTest) {
    {
        std::string_view error = "literal suffix or to_string failed";
        for (auto number : random_numbers) {
            std::string num = std::to_string(number);
            BigInteger bi = operator""_bi(num.c_str(), num.size());
            ASSERT_EQ(bi.ToString(), num) << error;
        }
        for (const auto& num : random_strings) {
            if (RemoveLeadingZeroes(num) == "-0") {
                continue;
            }
            BigInteger bi = operator""_bi(num.c_str(), num.size());
            ASSERT_EQ(bi.ToString(), RemoveLeadingZeroes(num)) << error;
        }

        ASSERT_EQ("123"_bi, 123) << error;
        ASSERT_EQ("-123"_bi, -123) << error;
        ASSERT_EQ("0"_bi, 0) << error;
        ASSERT_EQ("1234567890"_bi, 1234567890) << error;
        ASSERT_EQ("-1234567890"_bi, -1234567890) << error;
        ASSERT_EQ("437624234"_bi, 437624234) << error;
        ASSERT_EQ("-1237193"_bi, -1237193) << error;
        ASSERT_EQ("-0"_bi, 0) << error;
        ASSERT_EQ("2147483647"_bi, 2147483647) << error;
    }
    {
        std::string_view error = "literal suffix or to_string failed";
        for (int64_t number : random_numbers) {
            if (number < 0) {
                number = -number;
            }
            BigInteger bi = operator""_bi(std::to_string(number).c_str());
            ASSERT_EQ(bi.ToString(), std::to_string(number)) << error;
        }
        ASSERT_EQ(123_bi, 123) << error;
        ASSERT_EQ(-123_bi, -123) << error;
        ASSERT_EQ(0_bi, 0) << error;
        ASSERT_EQ(1234567890_bi, 1234567890) << error;
        ASSERT_EQ(-1234567890_bi, -1234567890) << error;
        ASSERT_EQ(437624234_bi, 437624234) << error;
        ASSERT_EQ(-1237193_bi, -1237193) << error;
        ASSERT_EQ(-0_bi, 0) << error;
        ASSERT_EQ(2147483647_bi, 2147483647) << error;
    }
}

TEST(BigInteger, EqualityTest) {
    std::string_view error = "Operator == failed";
    for (auto number : random_numbers) {
        for (auto other_number : random_numbers) {
            ASSERT_EQ((BigInteger(number) == BigInteger(other_number)), (number == other_number))
                << error;
        }
    }
    for (const auto& number : random_strings) {
        for (const auto& other_number : random_strings) {
            BigInteger a = operator""_bi(number.c_str(), number.size());
            BigInteger b = operator""_bi(other_number.c_str(), other_number.size());
            if ((RemoveLeadingZeroes(number) == "0" && RemoveLeadingZeroes(other_number) == "-0") ||
                (RemoveLeadingZeroes(number) == "-0" && RemoveLeadingZeroes(other_number) == "0")) {
                ASSERT_EQ(a, b) << error;
                continue;
            }
            ASSERT_EQ((a == b), (RemoveLeadingZeroes(number) == RemoveLeadingZeroes(other_number)))
                << error;
        }
    }
}

TEST(BigInteger, ComparisonFromNumTest) {
    for (auto number : random_numbers) {
        for (auto other_number : random_numbers) {
            BigInteger a(number), b(other_number);
            ASSERT_EQ((a <=> b), (number <=> other_number)) << "Operator <=> failed";
        }
    }
}

TEST(BigInteger, ComparisonFromStrTest) {
    for (const auto& number : random_strings) {
        for (const auto& other_number : random_strings) {
            BigInteger a(number), b(other_number);
            ASSERT_EQ((a <=> b), CompareStringsAsNums(number, other_number))
                << "Operator <=> failed";
        }
    }
}

TEST(BigInteger, IncrementTest) {
    {
        for (auto number : random_numbers) {
            BigInteger num = number, num2 = number;
            ASSERT_EQ(num2, num++) << "PostIncrement failed";
            ASSERT_EQ(num2, --num) << "PreDecrement failed";
            ASSERT_EQ(num2, num--) << "PostDecrement failed";
            ASSERT_EQ(num2, ++num) << "PreIncrement failed";

            ASSERT_NE(num2, ++num) << "PreIncrement failed";
            ASSERT_NE(num2, num--) << "PostDecrement failed";
        }
        for (const auto& number : random_strings) {
            BigInteger num = operator""_bi(number.c_str(), number.size());
            BigInteger num2 = num;
            ASSERT_EQ(num2, num++) << "PostIncrement failed";
            ASSERT_EQ(num2, --num) << "PreDecrement failed";
            ASSERT_EQ(num2, num--) << "PostDecrement failed";
            ASSERT_EQ(num2, ++num) << "PreIncrement failed";

            ASSERT_NE(num2, ++num) << "PreIncrement failed";
            ASSERT_NE(num2, num--) << "PostDecrement failed";
        }
    }
    BigInteger a = 0;
    {
        for (auto i = 0; i < 100; ++i, ++a) {
            ASSERT_EQ(a, i) << "PreIncrement failed";
        }
    }

    {
        ++a = 100;
        ASSERT_EQ(a, 100) << "PreIncrement return something wrong";
        --a = 1000;
        ASSERT_EQ(a, 1000) << "PreDecrement return something wrong";
    }
}

TEST(BigInteger, ArithmeticsTest1) {
    {
        BigInteger r = "6341086751599967347193489"_bi;
        r *= "1389194157770557901982669"_bi;
        ASSERT_EQ(r, BigInteger("8809000669238959543714953875073714437805867642141"_bi))
            << "Operation *";
        r %= "-6956634494956181641385527"_bi;
        ASSERT_EQ(r, BigInteger("2591921886068727455207854"_bi)) << "Operation %";
        r %= "-7890460085193212629710254"_bi;
        ASSERT_EQ(r, BigInteger("2591921886068727455207854"_bi)) << "Operation %";
    }
}

TEST(BigInteger, ArithmeticsTest2) {
    {
        BigInteger r = "6657415013798714353004181"_bi;
        r /= "1520624359848632827903047"_bi;
        ASSERT_EQ(r, BigInteger("4"_bi)) << "Operation /";
        r /= "2028533358321142558078422"_bi;
        ASSERT_EQ(r, BigInteger("0"_bi)) << "Operation /";
        r -= "-7322184067054838499157345"_bi;
        ASSERT_EQ(r, BigInteger("7322184067054838499157345"_bi)) << "Operation -";
    }
}

TEST(BigInteger, ArithmeticsTest3) {
    {
        BigInteger r = "6767013461753677975268730"_bi;
        r += "-2480883393976087591656495"_bi;
        ASSERT_EQ(r, BigInteger("4286130067777590383612235"_bi)) << "Operation +";
        r *= "3785829436632023221692025"_bi;
        ASSERT_EQ(r, BigInteger("16226557379826010509457581418772477364928191925875"_bi))
            << "Operation *";
        r -= "5089499475515840830013363"_bi;
        ASSERT_EQ(r, BigInteger("16226557379826010509457576329273001849087361912512"_bi))
            << "Operation -";
    }
}

TEST(BigInteger, ArithmeticsTest4) {
    {
        BigInteger r = "2972532736151357499734165"_bi;
        r += "-5094590519879405036099024"_bi;
        ASSERT_EQ(r, BigInteger("-2122057783728047536364859"_bi)) << "Operation +";
        r *= "8342665071780268210322499"_bi;
        ASSERT_EQ(r, BigInteger("-17703617352607428573769123782328819041262520662641"_bi))
            << "Operation *";
        r -= "-8315588255582518348425380"_bi;
        ASSERT_EQ(r, BigInteger("-17703617352607428573769115466740563458744172237261"_bi))
            << "Operation -";
    }
}

TEST(BigInteger, ArithmeticsTest5) {
    {
        BigInteger r = "-1055381144022456101342163"_bi;
        r *= "-2200715871672781763723498"_bi;
        ASSERT_EQ(r, BigInteger("2322594034314397110100433589958019033782221246174"_bi))
            << "Operation *";
        r -= "-6087862424604249658450872"_bi;
        ASSERT_EQ(r, BigInteger("2322594034314397110100439677820443638031879697046"_bi))
            << "Operation -";
        r %= "6871341973002343922696869"_bi;
        ASSERT_EQ(r, BigInteger("4100760386525114913650854"_bi)) << "Operation %";
    }
}

TEST(BigInteger, ArithmeticsTest6) {
    {
        BigInteger r = "3963843248861530844721812"_bi;
        r -= "1192271604572981958145293"_bi;
        ASSERT_EQ(r, BigInteger("2771571644288548886576519"_bi)) << "Operation -";
        r /= "1106489900387298483981228"_bi;
        ASSERT_EQ(r, BigInteger("2"_bi)) << "Operation /";
        r -= "2405266617925517342856507"_bi;
        ASSERT_EQ(r, BigInteger("-2405266617925517342856505"_bi)) << "Operation -";
    }
}

TEST(BigInteger, ArithmeticsTest7) {
    {
        BigInteger r = "-8842574659701250391978903"_bi;
        r %= "-606215102275935431180977"_bi;
        ASSERT_EQ(r, BigInteger("-355563227838154355445225"_bi)) << "Operation %";
        r += "6843472134878286724758077"_bi;
        ASSERT_EQ(r, BigInteger("6487908907040132369312852"_bi)) << "Operation +";
        r += "-5532976266453391366112993"_bi;
        ASSERT_EQ(r, BigInteger("954932640586741003199859"_bi)) << "Operation +";
    }
}

TEST(BigInteger, ArithmeticsTest8) {
    {
        BigInteger r = "-7076032484461336174721841"_bi;
        r %= "5711585029421880028365003"_bi;
        ASSERT_EQ(r, BigInteger("-1364447455039456146356838"_bi)) << "Operation %";
        r *= "-4211793082231374900667562"_bi;
        ASSERT_EQ(r, BigInteger("5746770352203386328643971725693464180540463488956"_bi))
            << "Operation *";
        r /= "-3650662620589425824292448"_bi;
        ASSERT_EQ(r, BigInteger("-1574171855759031708327304"_bi)) << "Operation /";
    }
}

TEST(BigInteger, ArithmeticsTest9) {
    {
        BigInteger r = "-9669758739254485731280887"_bi;
        r += "-3751733428500441053361240"_bi;
        ASSERT_EQ(r, BigInteger("-13421492167754926784642127"_bi)) << "Operation +";
        r *= "6779906538422098552335515"_bi;
        ASSERT_EQ(r, BigInteger("-90996462503542613303098282468854077905049307240405"_bi))
            << "Operation *";
        r -= "-732063175816037524779769"_bi;
        ASSERT_EQ(r, BigInteger("-90996462503542613303098281736790902089011782460636"_bi))
            << "Operation -";
    }
}

TEST(BigInteger, ArithmeticsTest10) {
    {
        BigInteger r = "2524039703812775907739329"_bi;
        r -= "7639443698178769436604471"_bi;
        ASSERT_EQ(r, BigInteger("-5115403994365993528865142"_bi)) << "Operation -";
        r /= "-3682164004305312980138079"_bi;
        ASSERT_EQ(r, BigInteger("1"_bi)) << "Operation /";
        r *= "6935269747561080453830473"_bi;
        ASSERT_EQ(r, BigInteger("6935269747561080453830473"_bi)) << "Operation *";
    }
}

TEST(BigInteger, DefaultTest) {
    for (auto number : random_numbers) {
        ASSERT_EQ(BigInteger(number).ToString(), std::to_string(number))
            << "Constructor from int or toString failed";
    }
}

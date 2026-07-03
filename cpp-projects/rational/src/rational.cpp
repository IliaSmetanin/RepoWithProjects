#include "../rational.hpp"

#include "biginteger.hpp"

#include <cmath>
#include <sstream>
#include <string>

Rational::Rational(const BigInteger& numerator, const BigInteger& denominator)
    : numerator_(numerator), denominator_(denominator) {
    ToIrreducible();
}

Rational::Rational(uint64_t numerator) : Rational(BigInteger(numerator)) {
}

Rational::Rational(int64_t numerator) : Rational(BigInteger(numerator)) {
}

Rational::Rational(int32_t numerator) : Rational(static_cast<int64_t>(numerator)) {
}

Rational::Rational(const std::string& rat_str) {
    if (rat_str[0] == '-') {
        negative_ = true;
    }

    size_t slash_position = rat_str.find('/');
    if (slash_position != std::string::npos) {
        FromSlashStr(rat_str, slash_position);
        return;
    }

    size_t dot_position = rat_str.find('.');
    if (dot_position != std::string::npos) {
        FromDotStr(rat_str, dot_position);
        return;
    }

    numerator_ = rat_str.substr(negative_, rat_str.size() - negative_);
    denominator_ = 1;

    ToIrreducible();
}

Rational& Rational::operator+=(const Rational& right) {
    if (right.numerator_ == 0) {
        return *this;
    }

    if (numerator_ == 0) {
        *this = right;
        return *this;
    }

    int32_t left_sign = (negative_) ? -1 : 1;
    int32_t right_sign = (right.negative_) ? -1 : 1;
    negative_ = false;

    numerator_ =
        left_sign * numerator_ * right.denominator_ + right_sign * right.numerator_ * denominator_;
    denominator_ *= right.denominator_;

    ToIrreducible();

    return *this;
}

Rational& Rational::operator-=(const Rational& right) {
    if (right.numerator_ == 0) {
        return *this;
    }

    if (numerator_ == 0) {
        *this = right;
        negative_ = !right.negative_;
        return *this;
    }

    int32_t left_sign = (negative_) ? -1 : 1;
    int32_t right_sign = (right.negative_) ? -1 : 1;
    negative_ = false;

    numerator_ =
        left_sign * numerator_ * right.denominator_ - right_sign * right.numerator_ * denominator_;
    denominator_ *= right.denominator_;

    ToIrreducible();

    return *this;
}

Rational& Rational::operator*=(const Rational& right) {
    if (right.numerator_ == 0 || numerator_ == 0) {
        *this = 0;
        return *this;
    }

    numerator_ *= right.numerator_;
    denominator_ *= right.denominator_;

    ToIrreducible();
    negative_ = (negative_ != right.negative_);
    return *this;
}

Rational& Rational::operator/=(const Rational& right) {
    if (numerator_ == 0) {
        return *this;
    }

    numerator_ *= right.denominator_;
    denominator_ *= right.numerator_;

    ToIrreducible();
    negative_ = (negative_ != right.negative_);
    return *this;
}

Rational Rational::operator+() const {
    return *this;
}

Rational Rational::operator-() const {
    if (numerator_ == 0) {
        return 0;
    }
    Rational temp = *this;
    temp.negative_ = !negative_;
    return temp;
}

Rational::operator double() const {
    return std::stod(AsDecimal(kPrecision));
}

void Rational::ToIrreducible() {
    if (numerator_ == 0) {
        denominator_ = 1;
        negative_ = false;
        return;
    }

    if (numerator_.GetSign() != denominator_.GetSign()) {
        negative_ = !negative_;
    }
    numerator_ = numerator_.Abs();
    denominator_ = denominator_.Abs();

    if (numerator_ == denominator_) {
        numerator_ = denominator_ = 1;
        return;
    }
    BigInteger gcd = GCD(numerator_, denominator_);
    numerator_ /= gcd;
    denominator_ /= gcd;
}

void Rational::FromSlashStr(const std::string slash_str, size_t slash_pos) {
    std::string numerator_str = slash_str.substr(negative_, slash_pos - negative_);
    std::string denominator_str =
        slash_str.substr(slash_pos + 1, slash_str.size() - numerator_str.size() - negative_);

    numerator_ = numerator_str;
    denominator_ = denominator_str;

    ToIrreducible();
}

void Rational::FromDotStr(const std::string dot_str, size_t dot_pos) {
    std::string integer_str = dot_str.substr(negative_, dot_pos - negative_);
    std::string fractional_str =
        dot_str.substr(dot_pos + 1, dot_str.size() - integer_str.size() - negative_);

    while (!fractional_str.empty() && fractional_str.back() == '0') {
        fractional_str.pop_back();
    }

    if (fractional_str.empty()) {
        numerator_ = BigInteger(integer_str);
        denominator_ = BigInteger(1);
        return;
    }

    denominator_ = 1;
    size_t fractional_amount = fractional_str.size();
    for (size_t i = 0; i < fractional_amount; ++i) {
        denominator_ *= 10;
    }
    numerator_ = denominator_ * BigInteger(integer_str) + BigInteger(fractional_str);

    ToIrreducible();
}

std::string Rational::ToString() const {
    std::string final{};
    if (negative_) {
        final += '-';
    }
    final += numerator_.ToString();
    if (denominator_ != 1) {
        final += '/';
        final += denominator_.ToString();
    }
    return final;
}

BigInteger Rational::GetInteger() const {
    return numerator_ / denominator_;
}

std::string Rational::FractionalStr(size_t precision) const {
    std::string fractional_str{};
    BigInteger carry = numerator_ % denominator_;
    carry *= 10;
    size_t i = 0;
    while (i < precision && carry < denominator_) {
        fractional_str += '0';
        carry *= 10;
        ++i;
    }

    if (i < precision) {
        size_t move_bits_amount = (precision - i) / BigInteger::kDegree;
        carry.MoveBits(move_bits_amount);

        Degree ten_degree = (precision - i) % BigInteger::kDegree;
        uint64_t multiplier = 1;
        for (size_t i = 0; i < ten_degree; ++i) {
            multiplier *= 10;
        }
        carry *= multiplier;
    }

    BigInteger result = carry / denominator_;
    fractional_str += result.ToString();

    int32_t last_digit = fractional_str.back() - '0';
    if (last_digit >= 5) {
        size_t i = 0;
        while (i < precision && fractional_str[precision - 1 - i] == '9') {
            fractional_str[precision - 1 - i] = '0';
            ++i;
        }
        if (i != precision) {
            fractional_str[precision - 1 - i] += 1;
        } else {
            fractional_str = "1" + fractional_str;
        }
    }
    fractional_str.pop_back();

    return fractional_str;
}

std::string Rational::AsDecimal(size_t precision) const {
    std::ostringstream final;
    BigInteger integer = GetInteger();
    std::string fractional_str = FractionalStr(precision);

    if (fractional_str.size() > precision && fractional_str.front() == '1') {
        integer += 1;
        fractional_str = fractional_str.substr(1, precision);
    }

    if (negative_ && (fractional_str != std::string(precision, '0') || integer != 0)) {
        final << '-';
    }
    final << integer.ToString();
    final << '.';
    final << fractional_str;

    return final.str();
}

BigInteger Rational::GCD(const BigInteger& numerator, const BigInteger& denominator) {
    BigInteger big;
    BigInteger small;

    if (denominator > numerator) {
        big = denominator;
        small = numerator;
    } else {
        big = numerator;
        small = denominator;
    }

    while (small != 0) {
        big %= small;
        std::swap(big, small);
    }
    return big;
}

std::istream& operator>>(std::istream& is, Rational& right) {
    std::string temp;
    is >> temp;
    right = Rational(temp);
    return is;
}

std::ostream& operator<<(std::ostream& os, const Rational& right) {
    os << right.ToString();
    return os;
}

Rational operator+(const Rational& left, const Rational& right) {
    Rational temp = left;
    temp += right;
    return temp;
}

Rational operator-(const Rational& left, const Rational& right) {
    Rational temp = left;
    temp -= right;
    return temp;
}

Rational operator*(const Rational& left, const Rational& right) {
    Rational temp = left;
    temp *= right;
    return temp;
}

Rational operator/(const Rational& left, const Rational& right) {
    Rational temp = left;
    temp /= right;
    return temp;
}

std::strong_ordering operator<=>(const Rational& left, const Rational& right) {
    bool left_sign = left.GetSign();
    bool right_sign = right.GetSign();

    if (left_sign != right_sign) {
        return right_sign <=> left_sign;
    }

    BigInteger left_num = left.GetNumerator();
    BigInteger left_denom = left.GetDenominator();
    BigInteger right_num = right.GetNumerator();
    BigInteger right_denom = right.GetDenominator();

    if (left_num == 0 && right_num == 0) {
        return std::strong_ordering::equal;
    }
    if (left_num == 0) {
        return std::strong_ordering::less;
    }
    if (right_num == 0) {
        return std::strong_ordering::greater;
    }

    if (left_sign) {
        return (right_num * left_denom) <=> (left_num * right_denom);
    }
    return left_num * right_denom <=> right_num * left_denom;
}

bool operator==(const Rational& left, const Rational& right) {
    return left.GetSign() == right.GetSign() && left.GetNumerator() == right.GetNumerator() &&
           left.GetDenominator() == right.GetDenominator();
}

Rational operator""_rat(const char* str, size_t) {
    return Rational(str);
}

Rational operator""_rat(const char* str) {
    return Rational(str);
}
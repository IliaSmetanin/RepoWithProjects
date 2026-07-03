#pragma once

#include "biginteger.hpp"

#include <math.h>
#include <string>

class Rational {
public:
    Rational() = default;

    Rational(const BigInteger& numerator, const BigInteger& denominator = 1);

    Rational(uint64_t numerator);

    Rational(int64_t numerator);

    Rational(int32_t numerator);

    Rational(const Rational& right) = default;

    Rational(const std::string& rat_str);

    Rational& operator=(const Rational& right) = default;

    Rational& operator+=(const Rational& right);

    Rational& operator-=(const Rational& right);

    Rational& operator*=(const Rational& right);

    Rational& operator/=(const Rational& right);

    Rational operator+() const;

    Rational operator-() const;

    explicit operator double() const;

    BigInteger& GetNumerator() {
        return numerator_;
    }

    const BigInteger& GetNumerator() const {
        return numerator_;
    }

    BigInteger& GetDenominator() {
        return denominator_;
    }

    const BigInteger& GetDenominator() const {
        return denominator_;
    }

    bool GetSign() const {
        return negative_;
    }

    void SetNumerator(const BigInteger& numerator) {
        numerator_ = numerator;
    }

    void SetDenominator(const BigInteger& denominator) {
        denominator_ = denominator;
    }

    void SetSign(bool negative) {
        negative_ = negative;
    }

    void ToIrreducible();

    void FromSlashStr(const std::string slash_str, size_t slash_pos);

    void FromDotStr(const std::string dot_str, size_t dot_pos);

    std::string ToString() const;

    BigInteger GetInteger() const;

    std::string FractionalStr(size_t precision = 1) const;

    std::string AsDecimal(size_t precision = 1) const;

private:
    static BigInteger GCD(const BigInteger& numerator, const BigInteger& denominator);

    static const uint16_t kPrecision{15};

    BigInteger numerator_{0};
    BigInteger denominator_{1};

    bool negative_ = false;
};

std::istream& operator>>(std::istream& is, Rational& right);

std::ostream& operator<<(std::ostream& os, const Rational& right);

Rational operator+(const Rational& left, const Rational& right);

Rational operator-(const Rational& left, const Rational& right);

Rational operator*(const Rational& left, const Rational& right);

Rational operator/(const Rational& left, const Rational& right);

std::strong_ordering operator<=>(const Rational& left, const Rational& right);

bool operator==(const Rational& left, const Rational& right);

Rational operator""_rat(const char* str, size_t);

Rational operator""_rat(const char* str);
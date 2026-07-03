#pragma once

#include <compare>
#include <iostream>
#include <string>
#include <vector>

enum class TypePlusMinus { kPlusPlus, kMinusMinus, kPlusMinus, kMinusPlus };

enum class TypeString { kWithZeros, kNoZeros };

using Degree = uint16_t;

class BigInteger {
public:
    BigInteger();

    BigInteger(const std::string& str_digits);

    BigInteger(const int right);

    BigInteger(const uint64_t right);

    BigInteger(const int64_t right);

    ~BigInteger() = default;

    BigInteger(const BigInteger& right) = default;

    BigInteger& operator=(const BigInteger& right) = default;

    BigInteger& operator+=(const BigInteger& right);

    BigInteger& operator-=(const BigInteger& right);

    BigInteger& operator*=(const BigInteger& right);

    BigInteger& operator/=(const BigInteger& right);

    BigInteger& operator%=(const BigInteger& right);

    BigInteger& operator++();

    BigInteger& operator--();

    BigInteger operator++(int);

    BigInteger operator--(int);

    BigInteger operator-() const;

    BigInteger operator+() const;

    explicit operator bool() const;

    explicit operator uint32_t() const;

    const std::vector<uint32_t>& GetDigits() const {
        return digits_;
    }

    std::vector<uint32_t>& GetDigits() {
        return digits_;
    }

    bool GetSign() const {
        return negative_;
    }

    void SetSign(bool negative) {
        negative_ = negative;
    }

    size_t GetSize() const {
        return digits_.size();
    }

    BigInteger Abs() const;

    std::string ToString(TypeString type = TypeString::kNoZeros) const;

    void MoveBits(size_t amount);

    inline static const uint16_t kDegree = 6;

private:
    static void SimpleOneDiv(const BigInteger& left, const BigInteger& right, BigInteger& result,
                             BigInteger& remainder);

    static void SimpleDiv(const BigInteger& denorm_left, const BigInteger& denorm_right,
                          BigInteger& result, BigInteger& remainder);

    static size_t GetMaxSize(const BigInteger& left, const BigInteger& right);

    static void ParseBigDigit(BigInteger& a, BigInteger& b, const BigInteger& right,
                              size_t bit_depth, size_t n);

    static bool FutureSign(const BigInteger& left, const BigInteger& right);

    static BigInteger SimpleMul(const BigInteger& left, const BigInteger& right);

    static BigInteger DeducPositive(BigInteger& left, const BigInteger& right);

    static BigInteger SumPositive(BigInteger& left, const BigInteger& right);

    static BigInteger& PlusMinusPattern(BigInteger& left, const BigInteger& right,
                                        TypePlusMinus type);

    static void CarrySumming(BigInteger& right);

    void FillStrWithZeros(std::string& temp, std::string& final) const;

    BigInteger& MinusOne();

    BigInteger& PlusOne();

    void RemoveTopZeros();

    void FillWithZeros(size_t amount);

    void Expand(size_t amount);

    void ChangeSign();

    std::string ParseStr(const std::string& str_digits);

    void CheckNegativeStr(const std::string& str_digits);

    inline static const uint64_t kBit = 1'000'000;

    std::vector<uint32_t> digits_;
    bool negative_{false};
};

std::strong_ordering operator<=>(const BigInteger& left, const BigInteger& right);

bool operator==(const BigInteger& left, const BigInteger& right);

BigInteger operator+(const BigInteger& left, const BigInteger& right);

BigInteger operator-(const BigInteger& left, const BigInteger& right);

BigInteger operator*(const BigInteger& left, const BigInteger& right);

BigInteger operator/(const BigInteger& left, const BigInteger& right);

BigInteger operator%(const BigInteger& left, const BigInteger& right);

std::ostream& operator<<(std::ostream& os, const BigInteger& right);

std::istream& operator>>(std::istream& is, BigInteger& right);

BigInteger operator""_bi(const char* str, size_t);

BigInteger operator""_bi(const char* str);

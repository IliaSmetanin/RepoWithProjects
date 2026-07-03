#include "../biginteger.hpp"

#include <cstddef>
#include <cstring>

enum class TypeOperators { kPlus, kMinus, kMul, kDiv, kRem };

BigInteger::BigInteger() : digits_{0}, negative_(false) {
}

BigInteger::BigInteger(const std::string& str_digits) {
    if (str_digits.empty() || str_digits == "0" || str_digits == "-0") {
        digits_.push_back(0);
        return;
    }
    CheckNegativeStr(str_digits);
    std::string parsed_str = ParseStr(str_digits);
    size_t left_digits = parsed_str.size();
    std::string one_digit_str;
    while (left_digits > kDegree) {
        one_digit_str = parsed_str.substr(left_digits - kDegree, kDegree);
        digits_.push_back(static_cast<uint32_t>(std::stoull(one_digit_str)));
        left_digits -= kDegree;
    }
    one_digit_str = parsed_str.substr(0, left_digits);
    digits_.push_back(static_cast<uint32_t>(std::stoull(one_digit_str)));
}

BigInteger::BigInteger(const int32_t right) : BigInteger(static_cast<int64_t>(right)) {
}

BigInteger::BigInteger(const uint64_t right) {
    uint64_t temp = right;
    while (temp / kBit > 0) {
        digits_.push_back(temp % kBit);
        temp /= kBit;
    }
    digits_.push_back(static_cast<uint32_t>(temp));
}

BigInteger::BigInteger(const int64_t right) {
    negative_ = right < 0;

    uint64_t temp;
    if (right == INT64_MIN) {
        temp = static_cast<uint64_t>(INT64_MAX) + 1;
    } else if (negative_) {
        temp = static_cast<uint64_t>(-right);
    } else {
        temp = static_cast<uint64_t>(right);
    }
    if (temp < kBit) {
        digits_.push_back(static_cast<uint32_t>(temp));
        return;
    }
    while (temp > 0) {
        digits_.push_back(static_cast<uint32_t>(temp % kBit));
        temp /= kBit;
    }
}

BigInteger& BigInteger::operator+=(const BigInteger& right) {
    if (*this == 0) {
        *this = right;
        return *this;
    }
    if (right == 0) {
        return *this;
    }
    if (*this < 0 && right > 0) {
        return PlusMinusPattern(*this, right, TypePlusMinus::kMinusPlus);
    } else if (*this > 0 && right < 0) {
        return PlusMinusPattern(*this, right, TypePlusMinus::kPlusMinus);
    } else if (*this < 0 && right < 0) {
        return PlusMinusPattern(*this, right, TypePlusMinus::kMinusMinus);
    }

    return PlusMinusPattern(*this, right, TypePlusMinus::kPlusPlus);
}

BigInteger& BigInteger::operator-=(const BigInteger& right) {
    if (*this == 0) {
        *this = right;
        if (right != 0) {
            negative_ = !right.negative_;
        }
        return *this;
    }
    if (right == 0) {
        return *this;
    }
    if (*this < 0 && right > 0) {
        return PlusMinusPattern(*this, right, TypePlusMinus::kMinusMinus);
    } else if (*this > 0 && right < 0) {
        return PlusMinusPattern(*this, right, TypePlusMinus::kPlusPlus);
    } else if (*this < 0 && right < 0) {
        return PlusMinusPattern(*this, right, TypePlusMinus::kMinusPlus);
    }

    return PlusMinusPattern(*this, right, TypePlusMinus::kPlusMinus);
}

BigInteger& BigInteger::operator*=(const BigInteger& right) {
    if (*this == 0 || right == 0) {
        *this = 0;
        return *this;
    } else if (*this == 1) {
        *this = right;
        return *this;
    } else if (right == 1) {
        return *this;
    } else if (*this == -1) {
        *this = right;
        ChangeSign();
        return *this;
    } else if (right == -1) {
        ChangeSign();
        return *this;
    } else if (this->GetSize() == 1 || right.GetSize() == 1) {
        *this = SimpleMul(*this, right);
        return *this;
    }
    bool result_sign = FutureSign(*this, right);
    size_t bit_depth = GetMaxSize(*this, right);
    size_t n = (bit_depth + 1) / 2;

    BigInteger a;
    BigInteger b;
    ParseBigDigit(a, b, *this, GetSize(), n);

    BigInteger c;
    BigInteger d;
    ParseBigDigit(c, d, right, right.GetSize(), n);

    BigInteger ab_cd = SimpleMul(a + b, c + d);
    BigInteger x = SimpleMul(a, c);
    BigInteger y = SimpleMul(b, d);
    BigInteger z = ab_cd - x - y;
    x.MoveBits(n * 2);
    z.MoveBits(n);

    size_t previous = GetSize();
    FillWithZeros(previous + right.GetSize());
    *this += y;
    *this += z;
    *this += x;
    RemoveTopZeros();
    negative_ = result_sign;
    return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& right) {
    if (right == *this) {
        *this = 1;
        return *this;
    } else if (Abs() == right.Abs() && right.negative_) {
        *this = 1;
        ChangeSign();
        return *this;
    } else if (right.Abs() > Abs()) {
        *this = 0;
        return *this;
    } else if (right == 1) {
        return *this;
    } else if (right == -1) {
        ChangeSign();
        return *this;
    }
    BigInteger result;
    BigInteger remainder;
    SimpleDiv(*this, right, result, remainder);
    *this = result;

    return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& right) {
    if (Abs() == right.Abs()) {
        *this = 0;
        return *this;
    } else if (Abs() < right.Abs()) {
        return *this;
    } else if (*this == 0) {
        return *this;
    } else if (right == 1 || right == -1) {
        *this = 0;
        return *this;
    }
    BigInteger result;
    BigInteger remainder;
    SimpleDiv(*this, right, result, remainder);
    *this = remainder;

    return *this;
}

BigInteger& BigInteger::operator++() {
    if (GetSize() == 1 && digits_[0] == 1 && negative_ == true) {
        negative_ = false;
        digits_[0] = 0;
        return *this;
    }
    if (negative_ == false) {
        return PlusOne();
    }
    return MinusOne();
}

BigInteger& BigInteger::operator--() {
    if (GetSize() == 1 && digits_[0] == 0 && negative_ == false) {
        negative_ = true;
        digits_[0] = 1;
        return *this;
    }
    if (negative_ == false) {
        return MinusOne();
    }
    return PlusOne();
}

BigInteger BigInteger::operator++(int) {
    BigInteger temp = *this;
    ++*this;
    return temp;
}

BigInteger BigInteger::operator--(int) {
    BigInteger temp = *this;
    --*this;
    return temp;
}

BigInteger BigInteger::operator-() const {
    if (*this == 0) {
        return 0;
    }
    BigInteger temp = *this;
    temp.ChangeSign();
    return temp;
}

BigInteger BigInteger::operator+() const {
    return *this;
}

BigInteger::operator bool() const {
    return !(*this == static_cast<BigInteger>(0));
}

BigInteger::operator uint32_t() const {
    return digits_[0];
}

BigInteger BigInteger::Abs() const {
    BigInteger temp = *this;
    temp.negative_ = false;
    return temp;
}

std::string BigInteger::ToString(TypeString type) const {
    std::string final;
    if (negative_) {
        final += '-';
    }

    std::string temp = std::to_string(digits_[GetSize() - 1]);
    if (type == TypeString::kWithZeros) {
        FillStrWithZeros(temp, final);
    } else {
        final += temp;
    }

    size_t size = GetSize();
    for (size_t i = 1; i < size; ++i) {
        size = GetSize();
        temp = std::to_string(digits_[size - 1 - i]);
        FillStrWithZeros(temp, final);
    }
    return final;
}

void BigInteger::SimpleOneDiv(const BigInteger& left, const BigInteger& right, BigInteger& result,
                              BigInteger& remainder) {
    bool result_sign = FutureSign(left, right);
    bool remainder_sign = left.negative_;
    size_t left_size = left.GetSize();
    size_t result_size = left_size - right.GetSize() + 1;
    result.Expand(result_size - 1);

    uint64_t rem = 0;
    uint32_t divisor = right.digits_[0];
    for (size_t i = 0; i < result_size; ++i) {
        size_t current_index = result_size - 1 - i;
        uint32_t current_digit = left.digits_[left_size - 1 - i];
        result.digits_[current_index] =
            static_cast<uint32_t>((rem * kBit + current_digit) / divisor);
        rem = (rem * kBit + current_digit) % divisor;
    }

    remainder = rem;
    if (remainder != 0) {
        remainder.negative_ = remainder_sign;
    }
    result.RemoveTopZeros();
    result.negative_ = result_sign;
}

void BigInteger::SimpleDiv(const BigInteger& denorm_left, const BigInteger& denorm_right,
                           BigInteger& result, BigInteger& remainder) {
    if (denorm_right.GetSize() == 1) {
        return SimpleOneDiv(denorm_left, denorm_right, result, remainder);
    }

    uint64_t normalizer = kBit / (denorm_right.digits_[denorm_right.GetSize() - 1] + 1);
    BigInteger left = normalizer * denorm_left.Abs();
    BigInteger right = normalizer * denorm_right.Abs();

    size_t left_size = left.GetSize();
    size_t right_size = right.GetSize();

    left.digits_.push_back(0);
    ++left_size;

    size_t result_size = left_size - right_size;
    result.Expand(result_size - 1);

    for (size_t j = 0; j < result_size; ++j) {
        uint64_t two_digits_left =
            static_cast<uint64_t>(left.digits_[result_size - 1 - j + right_size]) * kBit +
            left.digits_[result_size - j + right_size - 2];
        uint64_t first_right = right.digits_[right_size - 1];
        uint64_t second_right = (right_size >= 2) ? right.digits_[right_size - 2] : 0;
        uint64_t third_left = (left_size - j >= 3) ? left.digits_[left_size - j - 3] : 0;
        uint64_t possible_digit = two_digits_left / first_right;
        uint64_t rem = two_digits_left % first_right;

        if (possible_digit >= kBit) {
            possible_digit = kBit - 1;
            rem = two_digits_left - possible_digit * first_right;
        }
        while (possible_digit * second_right > kBit * rem + third_left) {
            --possible_digit;
            rem += first_right;
            if (rem >= kBit) {
                break;
            }
        }

        BigInteger comparable = right * possible_digit;
        bool borrow = false;
        for (size_t i = 0; i <= right_size; ++i) {
            uint64_t left_digit = (result_size - 1 - j + i < left.digits_.size())
                                      ? left.digits_[result_size - 1 - j + i]
                                      : 0;
            uint64_t comparable_digit = (i < comparable.GetSize()) ? comparable.digits_[i] : 0;

            if (borrow) {
                if (left_digit == 0) {
                    left_digit = kBit - 1;
                } else {
                    --left_digit;
                    borrow = false;
                }
            }
            if (left_digit < comparable_digit) {
                left_digit += kBit;
                borrow = true;
            }
            if (result_size - 1 - j + i < left.digits_.size()) {
                left.digits_[result_size - 1 - j + i] =
                    static_cast<uint32_t>(left_digit - comparable_digit);
            }
        }
        result.digits_[result_size - 1 - j] = static_cast<uint32_t>(possible_digit);

        if (borrow) {
            --result.digits_[result_size - 1 - j];
            uint64_t carry = 0;
            for (size_t i = 0; i <= right_size; ++i) {
                uint64_t left_digit = (result_size - 1 - j + i < left.digits_.size())
                                          ? left.digits_[result_size - 1 - j + i]
                                          : 0;
                uint64_t right_digit = (i < right.GetSize()) ? right.digits_[i] : 0;

                uint64_t sum = left_digit + right_digit + carry;
                if (result_size - 1 - j + i < left.digits_.size()) {
                    left.digits_[result_size - 1 - j + i] = static_cast<uint32_t>(sum % kBit);
                }
                carry = sum / kBit;
            }
        }
    }

    remainder.digits_.clear();
    for (size_t i = 0; i < right_size; ++i) {
        remainder.digits_.push_back(left.digits_[i]);
    }
    remainder.RemoveTopZeros();
    if (normalizer > 1) {
        remainder /= normalizer;
        remainder.RemoveTopZeros();
    }
    result.negative_ = (denorm_left.negative_ != denorm_right.negative_);
    remainder.negative_ = denorm_left.negative_;
    if (remainder.GetSize() == 1 && remainder.digits_[0] == 0) {
        remainder.negative_ = false;
    }
    result.RemoveTopZeros();
}

size_t BigInteger::GetMaxSize(const BigInteger& left, const BigInteger& right) {
    size_t left_size = left.GetDigits().size();
    size_t right_size = right.GetDigits().size();
    return (left_size > right_size) ? left_size : right_size;
}

void BigInteger::ParseBigDigit(BigInteger& a, BigInteger& b, const BigInteger& right,
                               size_t right_size, size_t n) {
    a.FillWithZeros(n);
    b.FillWithZeros(n);
    for (size_t i = 0, j = 0; i < right_size; ++i) {
        if (i < n) {
            b.digits_[i] = right.digits_[i];
            continue;
        }
        a.digits_[j++] = right.digits_[i];
    }
    a.RemoveTopZeros();
    b.RemoveTopZeros();
}

bool BigInteger::FutureSign(const BigInteger& left, const BigInteger& right) {
    if (left.negative_ != right.negative_) {
        return true;
    }
    return false;
}

BigInteger BigInteger::SimpleMul(const BigInteger& left, const BigInteger& right) {
    bool result_sign = FutureSign(left, right);
    size_t left_size = left.GetSize();
    size_t right_size = right.GetSize();
    BigInteger temp;
    temp.FillWithZeros(left_size + right_size);
    for (size_t i = 0; i < right_size; ++i) {
        for (size_t j = 0; j < left_size; ++j) {
            uint64_t res =
                static_cast<uint64_t>(left.digits_[j]) * static_cast<uint64_t>(right.digits_[i]);
            temp.digits_[j + i] += res % kBit;
            temp.digits_[j + i + 1] += res / kBit;
        }
    }
    temp.negative_ = result_sign;
    CarrySumming(temp);
    temp.RemoveTopZeros();
    return temp;
}

BigInteger BigInteger::DeducPositive(BigInteger& left, const BigInteger& right) {
    if (left == right) {
        return 0;
    }
    if (left < right) {
        BigInteger temp = right;
        return DeducPositive(temp, left);
    }
    size_t left_size = left.GetSize();
    size_t right_size = right.GetSize();
    for (size_t i = 0; i < right_size; ++i) {
        if (left.digits_[i] < right.digits_[i]) {
            size_t j = 1;
            while (i + j < left_size && left.digits_[i + j] == 0) {
                left.digits_[i + j] = kBit - 1;
                ++j;
            }
            left.digits_[i + j] -= 1;
            left.digits_[i] = kBit + left.digits_[i] - right.digits_[i];
        } else {
            left.digits_[i] = left.digits_[i] - right.digits_[i];
        }
    }
    left.RemoveTopZeros();
    return left;
}

BigInteger BigInteger::SumPositive(BigInteger& left, const BigInteger& right) {
    size_t left_size = left.GetSize();
    size_t right_size = right.GetSize();
    if (left_size < right_size) {
        BigInteger temp = right;
        return SumPositive(temp, left);
    }
    size_t i = 0;
    while (i < right_size) {
        left.digits_[i] += right.digits_[i];
        if (left.digits_[i] >= kBit) {
            size_t j = 1;
            while (i + j < left_size && left.digits_[i + j] == kBit - 1) {
                left.digits_[i + j] = 0;
                ++j;
            }
            if (i + j == left_size) {
                left.digits_.push_back(0);
            }
            left.digits_[i + j] += 1;
        }
        left.digits_[i] %= kBit;
        ++i;
    }
    left.RemoveTopZeros();
    return left;
}

BigInteger& BigInteger::PlusMinusPattern(BigInteger& left, const BigInteger& right,
                                         TypePlusMinus type) {
    BigInteger left_abs = left.Abs();
    BigInteger right_abs = right.Abs();
    switch (type) {
        bool neg_flag;
        case TypePlusMinus::kPlusPlus:
            left = SumPositive(left_abs, right_abs);
            break;
        case TypePlusMinus::kMinusMinus:
            left = SumPositive(left_abs, right_abs);
            left.negative_ = true;
            break;
        case TypePlusMinus::kPlusMinus:
            neg_flag = 0;
            if (left_abs < right_abs) {
                neg_flag = 1;
            }
            left = DeducPositive(left_abs, right_abs);
            left.negative_ = neg_flag;
            break;
        case TypePlusMinus::kMinusPlus:
            neg_flag = 1;
            if (left_abs <= right_abs) {
                neg_flag = 0;
            }
            left = DeducPositive(left_abs, right_abs);
            left.negative_ = neg_flag;
            break;
    }
    return left;
}

void BigInteger::CarrySumming(BigInteger& right) {
    uint32_t carry = 0;
    for (size_t i = 0; i < right.digits_.size(); ++i) {
        uint64_t current = static_cast<uint64_t>(right.digits_[i]) + carry;
        right.digits_[i] = static_cast<uint32_t>(current % kBit);
        carry = static_cast<uint32_t>(current / kBit);
    }
    while (carry > 0) {
        right.digits_.push_back(carry % kBit);
        carry /= kBit;
    }
}

void BigInteger::FillStrWithZeros(std::string& temp, std::string& final) const {
    size_t left_zeros = kDegree - temp.size();
    if (left_zeros > 0) {
        std::string zeros(left_zeros, '0');
        zeros += temp;
        final += zeros;
    } else {
        final += temp;
    }
}

BigInteger& BigInteger::MinusOne() {
    size_t size = GetSize();
    if (digits_[0] == 0) {
        size_t j = 1;
        while (j < size && digits_[j] == 0) {
            digits_[j] = kBit - 1;
            ++j;
        }
        digits_[j] -= 1;
        digits_[0] = kBit - 1;
    } else {
        digits_[0] = digits_[0] - 1;
    }
    RemoveTopZeros();
    return *this;
}

BigInteger& BigInteger::PlusOne() {
    size_t size = GetSize();
    digits_[0] += 1;
    if (digits_[0] == kBit) {
        size_t j = 1;
        while (j < size && digits_[j] == kBit - 1) {
            digits_[j] = 0;
            ++j;
        }
        if (j == size) {
            digits_.push_back(0);
        }
        digits_[j] += 1;
    }
    digits_[0] %= kBit;
    return *this;
}

void BigInteger::RemoveTopZeros() {
    size_t i = 0;
    size_t size = GetSize();
    while (i < size - 1 && digits_[size - 1 - i++] == 0) {
        digits_.pop_back();
    }
}

void BigInteger::FillWithZeros(size_t amount) {
    *this = 0;
    digits_.resize(amount, 0);
}

void BigInteger::Expand(size_t amount) {
    if (amount == 0) {
        return;
    }
    digits_.resize(GetSize() + amount, 0);
}

void BigInteger::MoveBits(size_t amount) {
    if (amount == 0) {
        return;
    }
    RemoveTopZeros();
    size_t old_size = GetSize();
    size_t new_size = old_size + amount;
    digits_.resize(new_size);
    for (size_t i = 0; i < old_size && i < new_size; ++i) {
        digits_[new_size - 1 - i] = digits_[old_size - 1 - i];
        digits_[old_size - 1 - i] = 0;
    }
}

void BigInteger::ChangeSign() {
    if (*this == 0) {
        return;
    }
    negative_ = !negative_;
}

std::string BigInteger::ParseStr(const std::string& str_digits) {
    const char* ptr_ch = str_digits.data();
    size_t count = 0;
    while ((*ptr_ch == '-' || *ptr_ch == '+' || *ptr_ch == '0') && strlen(ptr_ch) > 1) {
        ++ptr_ch;
        ++count;
    }
    std::string parsed_str = str_digits.substr(count, str_digits.size() - count);
    return parsed_str;
}

void BigInteger::CheckNegativeStr(const std::string& str_digits) {
    if (str_digits[0] == '-') {
        negative_ = true;
    }
}

std::strong_ordering operator<=>(const BigInteger& left, const BigInteger& right) {
    bool left_sign = left.GetSign();
    bool right_sign = right.GetSign();

    const auto& left_digits = left.GetDigits();
    const auto& right_digits = right.GetDigits();

    if (left_sign != right_sign) {
        return right_sign <=> left_sign;
    }

    size_t left_size = left_digits.size();
    size_t right_size = right_digits.size();
    if (left_size != right_size) {
        if (left_sign) {
            return right_size <=> left_size;
        }
        return left_size <=> right_size;
    }
    size_t max_size = (left_size > right_size) ? left_size : right_size;
    for (size_t i = 0; i < max_size; ++i) {
        if (left_digits[max_size - 1 - i] != right_digits[max_size - 1 - i]) {
            if (left_sign) {
                return right_digits[max_size - 1 - i] <=> left_digits[max_size - 1 - i];
            }
            return left_digits[max_size - 1 - i] <=> right_digits[max_size - 1 - i];
        }
    }
    return std::strong_ordering::equal;
}

bool operator==(const BigInteger& left, const BigInteger& right) {
    return (left <=> right) == std::strong_ordering::equal;
}

BigInteger ChooseOperator(const BigInteger& left, const BigInteger& right, TypeOperators type) {
    BigInteger temp = left;
    switch (type) {
        case TypeOperators::kPlus:
            temp += right;
            break;
        case TypeOperators::kMinus:
            temp -= right;
            break;
        case TypeOperators::kMul:
            temp *= right;
            break;
        case TypeOperators::kDiv:
            temp /= right;
            break;
        case TypeOperators::kRem:
            temp %= right;
            break;

        default:
            break;
    }
    return temp;
}

BigInteger operator+(const BigInteger& left, const BigInteger& right) {
    return ChooseOperator(left, right, TypeOperators::kPlus);
}

BigInteger operator-(const BigInteger& left, const BigInteger& right) {
    return ChooseOperator(left, right, TypeOperators::kMinus);
}

BigInteger operator*(const BigInteger& left, const BigInteger& right) {
    return ChooseOperator(left, right, TypeOperators::kMul);
}

BigInteger operator/(const BigInteger& left, const BigInteger& right) {
    return ChooseOperator(left, right, TypeOperators::kDiv);
}

BigInteger operator%(const BigInteger& left, const BigInteger& right) {
    return ChooseOperator(left, right, TypeOperators::kRem);
}

std::ostream& operator<<(std::ostream& os, const BigInteger& right) {
    os << right.ToString();
    return os;
}

std::istream& operator>>(std::istream& is, BigInteger& right) {
    std::string temp_str;
    is >> temp_str;
    right = BigInteger(temp_str);
    return is;
}

BigInteger operator""_bi(const char* str, size_t) {
    return BigInteger(str);
}

BigInteger operator""_bi(const char* str) {
    return BigInteger(str);
}

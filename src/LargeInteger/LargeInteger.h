#pragma once

#include "LargeIntegerData.h"
#include <cmath>
#include <initializer_list>
#include <type_traits>

namespace mag
{
    namespace multiprecision {
        inline bool is_zero(const LargeIntegerData& val)
        {
            return val.dim == 1 && val.data[0] == 0;
        }
        inline bool is_one(const LargeIntegerData& val)
        {
            return val.dim == 1 && val.data[0] == 1;
        }

        bool is_equal_to(const LargeIntegerData& val, const LargeIntegerData& val2);
        bool is_less_than(const LargeIntegerData& val, const LargeIntegerData& val2);
        bool is_greater_than(const LargeIntegerData& val, const LargeIntegerData& val2);

        void plus(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
        int subtract(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);

        void multiply(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
        void divide(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
        void modulo(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);

        void inverse(LargeIntegerData& result, const LargeIntegerData& val);
        void xor(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
        void and(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
        void or(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
        void lshift(LargeIntegerData& result, const LargeIntegerData& val, int shift);
        void rshift(LargeIntegerData& result, const LargeIntegerData& val, int shift);
    }

    class LargeInteger
    {
    public:
        LargeIntegerDataStorage m_data;
        int m_sign;
        inline void sign(int sign);

    public:
        inline LargeInteger();
        inline LargeInteger(int val);
        inline LargeInteger(int sign, std::initializer_list<LargeIntegerData::IntegerType> list);
        inline LargeInteger(const LargeInteger& copy);
        inline LargeInteger(LargeInteger&& copy);
        inline ~LargeInteger();

        inline LargeInteger& operator =(const LargeInteger& copy);
        inline LargeInteger& operator =(LargeInteger&& copy);

        inline operator bool() const;
        inline LargeInteger operator +() const;
        inline LargeInteger operator -() const;
        inline LargeInteger operator !() const;
        inline LargeInteger& operator ++();
        inline LargeInteger operator ++(int);
        inline LargeInteger& operator --();
        inline LargeInteger operator --(int);

        inline bool operator ==(const LargeInteger& other) const;
        inline bool operator !=(const LargeInteger& other) const;
        inline bool operator <(const LargeInteger& other) const;
        inline bool operator >(const LargeInteger& other) const;
        inline bool operator <=(const LargeInteger& other) const;
        inline bool operator >=(const LargeInteger& other) const;

        inline LargeInteger operator +(const LargeInteger& other) const;
        inline LargeInteger operator -(const LargeInteger& other) const;
        inline LargeInteger operator *(const LargeInteger& other) const;
        inline LargeInteger operator /(const LargeInteger& other) const;
        inline LargeInteger operator %(const LargeInteger& other) const;

        inline LargeInteger operator ~() const;
        inline LargeInteger operator ^(const LargeInteger& other) const;
        inline LargeInteger operator &(const LargeInteger& other) const;
        inline LargeInteger operator |(const LargeInteger& other) const;
        inline LargeInteger operator <<(int shift) const;
        inline LargeInteger operator >>(int shift) const;

        inline LargeInteger& operator +=(const LargeInteger& other);
        inline LargeInteger& operator -=(const LargeInteger& other);
        inline LargeInteger& operator *=(const LargeInteger& other);
        inline LargeInteger& operator /=(const LargeInteger& other);
        inline LargeInteger& operator %=(const LargeInteger& other);
        inline LargeInteger& operator ^=(const LargeInteger& other);
        inline LargeInteger& operator &=(const LargeInteger& other);
        inline LargeInteger& operator |=(const LargeInteger& other);
        inline LargeInteger& operator <<=(int shift);
        inline LargeInteger& operator >>=(int shift);
    };

    void LargeInteger::sign(int sign)
    {
        m_sign = (multiprecision::is_zero(m_data) ? 0 : sign);
    }

    LargeInteger::LargeInteger()
        : m_data(LargeIntegerData::IntegerZero)
        , m_sign(0)
    {}

    LargeInteger::LargeInteger(int val)
        : m_data(std::abs(val))
        , m_sign(val < 0)
    {}

    LargeInteger::LargeInteger(int sign, std::initializer_list<LargeIntegerData::IntegerType> list)
        : m_data(list)
        , m_sign(!!sign)
    {}

    LargeInteger::LargeInteger(const LargeInteger& copy)
        : m_data(copy.m_data)
        , m_sign(copy.m_sign)
    {}

    LargeInteger::LargeInteger(LargeInteger&& copy)
        : m_data(std::move(copy.m_data))
        , m_sign(copy.m_sign)
    {}

    LargeInteger::~LargeInteger()
    {}

    LargeInteger& LargeInteger::operator =(const LargeInteger& copy)
    {
        m_data = copy.m_data;
        m_sign = copy.m_sign;
        return *this;
    }

    LargeInteger& LargeInteger::operator =(LargeInteger&& copy)
    {
        m_data = std::move(copy.m_data);
        m_sign = copy.m_sign;
        return *this;
    }

    LargeInteger::operator bool() const
    {
        return !multiprecision::is_zero(m_data);
    }

    LargeInteger LargeInteger::operator +() const
    {
        return LargeInteger(*this);
    }

    LargeInteger LargeInteger::operator -() const
    {
        LargeInteger result;
        result.sign(!m_sign);
        return result;
    }

    LargeInteger LargeInteger::operator !() const
    {
        return LargeInteger(multiprecision::is_zero(m_data) ? 1 : 0);
    }

    LargeInteger& LargeInteger::operator ++()
    {
        if (m_sign && multiprecision::is_one(m_data)) {
            m_sign = 0;
            m_data.data[0] = 0;
        } else {
            LargeIntegerDataStorage ret, tmp(1);
            (m_sign ? (void)multiprecision::subtract(ret, m_data, tmp) : multiprecision::plus(ret, m_data, tmp));
            m_data = std::move(ret);
        }
        return *this;
    }

    LargeInteger LargeInteger::operator ++(int)
    {
        LargeInteger result(*this);
        operator ++();
        return result;
    }

    LargeInteger& LargeInteger::operator --()
    {
        if (multiprecision::is_zero(m_data)) {
            m_data.data[0] = 1;
            m_sign = 1;
        } else {
            LargeIntegerDataStorage ret, tmp(1);
            (m_sign ? multiprecision::plus(ret, m_data, tmp) : (void)multiprecision::subtract(ret, m_data, tmp));
            m_data = std::move(ret);
        }
        return *this;
    }

    LargeInteger LargeInteger::operator --(int)
    {
        LargeInteger result(*this);
        operator --();
        return result;
    }

    bool LargeInteger::operator ==(const LargeInteger& other) const
    {
        return m_sign == other.m_sign && multiprecision::is_equal_to(m_data, other.m_data);
    }

    bool LargeInteger::operator !=(const LargeInteger& other) const
    {
        return !operator ==(other);
    }

    bool LargeInteger::operator <(const LargeInteger& other) const
    {
        if (m_sign ^ other.m_sign) {
            return !!m_sign;
        } else {
            return (m_sign ? multiprecision::is_greater_than(m_data, other.m_data) : multiprecision::is_less_than(m_data, other.m_data));
        }
    }

    bool LargeInteger::operator >(const LargeInteger& other) const
    {
        if (m_sign ^ other.m_sign) {
            return !m_sign;
        } else {
            return (m_sign ? multiprecision::is_less_than(m_data, other.m_data) : multiprecision::is_greater_than(m_data, other.m_data));
        }
    }

    bool LargeInteger::operator <=(const LargeInteger& other) const
    {
        if (m_sign ^ other.m_sign) {
            return !!m_sign;
        } else {
            return (m_sign ? !multiprecision::is_less_than(m_data, other.m_data) : !multiprecision::is_greater_than(m_data, other.m_data));
        }
    }

    bool LargeInteger::operator >=(const LargeInteger& other) const
    {
        if (m_sign ^ other.m_sign) {
            return !m_sign;
        } else {
            return (m_sign ? !multiprecision::is_greater_than(m_data, other.m_data) : !multiprecision::is_less_than(m_data, other.m_data));
        }
    }

    LargeInteger LargeInteger::operator +(const LargeInteger& other) const
    {
        LargeInteger result;
        if (m_sign ^ other.m_sign) {
            int v = multiprecision::subtract(result.m_data, m_data, other.m_data);
            result.sign(v ^ m_sign);
        } else {
            multiprecision::plus(result.m_data, m_data, other.m_data);
            result.sign(m_sign);
        }
        return result;
    }

    LargeInteger LargeInteger::operator -(const LargeInteger& other) const
    {
        LargeInteger result;
        if (m_sign ^ other.m_sign) {
            multiprecision::plus(result.m_data, m_data, other.m_data);
            result.sign(m_sign);
        } else {
            int v = multiprecision::subtract(result.m_data, m_data, other.m_data);
            result.sign(v ^ m_sign);
        }
        return result;
    }

    LargeInteger LargeInteger::operator *(const LargeInteger& other) const
    {
        LargeInteger result;
        multiprecision::multiply(result.m_data, m_data, other.m_data);
        result.sign(m_sign ^ other.m_sign);
        return result;
    }

    LargeInteger LargeInteger::operator /(const LargeInteger& other) const
    {
        LargeInteger result;
        multiprecision::divide(result.m_data, m_data, other.m_data);
        result.sign(m_sign ^ other.m_sign);
        return result;
    }

    LargeInteger LargeInteger::operator %(const LargeInteger& other) const
    {
        LargeInteger result;
        multiprecision::modulo(result.m_data, m_data, other.m_data);
        result.sign(m_sign);
        return result;
    }

    LargeInteger LargeInteger::operator ~() const
    {
        LargeInteger result;
        multiprecision::inverse(result.m_data, m_data);
        result.sign(!m_sign);
        return result;
    }
    
    LargeInteger LargeInteger::operator ^(const LargeInteger& other) const
    {
        LargeInteger result;
        multiprecision::xor(result.m_data, m_data, other.m_data);
        result.sign(m_sign ^ other.m_sign);
        return result;
    }

    LargeInteger LargeInteger::operator &(const LargeInteger& other) const
    {
        LargeInteger result;
        multiprecision::and(result.m_data, m_data, other.m_data);
        result.sign(m_sign & other.m_sign);
        return result;
    }

    LargeInteger LargeInteger::operator |(const LargeInteger& other) const
    {
        LargeInteger result;
        multiprecision::or(result.m_data, m_data, other.m_data);
        result.sign(m_sign | other.m_sign);
        return result;
    }

    LargeInteger LargeInteger::operator <<(int shift) const
    {
        LargeInteger result;
        multiprecision::lshift(result.m_data, m_data, shift);
        result.sign(m_sign);
        return result;
    }

    LargeInteger LargeInteger::operator >>(int shift) const
    {
        LargeInteger result;
        multiprecision::rshift(result.m_data, m_data, shift);
        result.sign(m_sign);
        return result;
    }

    LargeInteger& LargeInteger::operator +=(const LargeInteger& other)
    {
        return operator =(operator +(other));
    }

    LargeInteger& LargeInteger::operator -=(const LargeInteger& other)
    {
        return operator =(operator -(other));
    }

    LargeInteger& LargeInteger::operator *=(const LargeInteger& other)
    {
        return operator =(operator *(other));
    }

    LargeInteger& LargeInteger::operator /=(const LargeInteger& other)
    {
        return operator =(operator /(other));
    }

    LargeInteger& LargeInteger::operator %=(const LargeInteger& other)
    {
        return operator =(operator %(other));
    }

    LargeInteger& LargeInteger::operator ^=(const LargeInteger& other)
    {
        return operator =(operator ^(other));
    }

    LargeInteger& LargeInteger::operator &=(const LargeInteger& other)
    {
        return operator =(operator &(other));
    }

    LargeInteger& LargeInteger::operator |=(const LargeInteger& other)
    {
        return operator =(operator |(other));
    }

    LargeInteger& LargeInteger:: operator <<=(int shift)
    {
        return operator =(operator <<(shift));
    }

    LargeInteger& LargeInteger:: operator >>=(int shift)
    {
        return operator =(operator >>(shift));
    }

}

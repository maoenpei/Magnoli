#pragma once

#include "LargeIntegerData.h"
#include <cmath>
#include <type_traits>

namespace mag
{
    namespace multiprecision {
        inline bool is_zero(const LargeIntegerData& val)
        {
            return val.dim == 1 && val.data[0] == LargeIntegerData::IntegerZero;
        }

        bool is_equal_to(const LargeIntegerData& val, const LargeIntegerData& val2);
        bool is_less_than(const LargeIntegerData& val, const LargeIntegerData& val2);
        bool is_greater_than(const LargeIntegerData& val, const LargeIntegerData& val2);

        void plus(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
        void subtract(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);

        void multiply(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
        void divide(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
        void modulo(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2);
    }

    class LargeInteger
    {
    public:
        LargeIntegerDataStorage m_data;
        int m_sign;
    public:
        inline LargeInteger();
        inline LargeInteger(int val);
        inline LargeInteger(const LargeInteger& copy);
        inline LargeInteger(LargeInteger&& copy);
        inline ~LargeInteger();

        inline LargeInteger& operator =(const LargeInteger& copy);
        inline LargeInteger& operator =(LargeInteger&& copy);

        inline bool operator ==(const LargeInteger& other);
        inline bool operator !=(const LargeInteger& other);

        inline LargeInteger& operator ++();
        inline LargeInteger operator ++(int);

        inline LargeInteger operator +(const LargeInteger& other);
        inline LargeInteger operator -(const LargeInteger& other);
        inline LargeInteger operator *(const LargeInteger& other);
        inline LargeInteger operator /(const LargeInteger& other);
        inline LargeInteger operator %(const LargeInteger& other);
    };

    LargeInteger::LargeInteger()
        : m_data(LargeIntegerData::IntegerZero)
        , m_sign(0)
    {}

    LargeInteger::LargeInteger(int val)
        : m_data(std::abs(val))
        , m_sign(val >= 0)
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

    bool LargeInteger::operator ==(const LargeInteger& other)
    {
        return m_sign == other.m_sign && multiprecision::is_equal_to(m_data, other.m_data);
    }

    bool LargeInteger::operator !=(const LargeInteger& other)
    {
        return !operator ==(other);
    }

    LargeInteger& LargeInteger::operator ++()
    {
        if (m_sign && m_data.dim == 1 && m_data.data[0] == 1) {
            m_sign = 0;
            m_data.data[0] = 0;
        } else {
            LargeIntegerDataStorage ret, tmp(1);
            (m_sign ? multiprecision::subtract(ret, m_data, tmp) : multiprecision::plus(ret, m_data, tmp));
            m_data = std::move(ret);
        }
        return *this;
    }

    LargeInteger LargeInteger::operator ++(int)
    {
        LargeInteger tmp(*this);
        operator ++();
        return tmp;
    }

    LargeInteger LargeInteger::operator +(const LargeInteger& other)
    {
        LargeInteger result;
        if (m_sign ^ other.m_sign) {
            multiprecision::subtract(result.m_data, other.m_data, m_data);
            result.m_sign = (multiprecision::is_less_than(m_data, other.m_data) ? !m_sign : m_sign);
        } else {
            multiprecision::plus(result.m_data, m_data, other.m_data);
            result.m_sign = m_sign;
        }
        return result;
    }

    LargeInteger LargeInteger::operator -(const LargeInteger& other)
    {
        LargeInteger result;
        if (m_sign ^ other.m_sign) {
            multiprecision::plus(result.m_data, m_data, other.m_data);
            result.m_sign = m_sign;
        } else {
            multiprecision::subtract(result.m_data, m_data, other.m_data);
            result.m_sign = (multiprecision::is_less_than(m_data, other.m_data) ? !m_sign : m_sign);
        }
        return result;
    }

    LargeInteger LargeInteger::operator /(const LargeInteger& other)
    {
        LargeInteger result;
        multiprecision::divide(result.m_data, m_data, other.m_data);
        result.m_sign = m_sign ^ other.m_sign;
        return result;
    }

    LargeInteger LargeInteger::operator %(const LargeInteger& other)
    {
        LargeInteger result;
        multiprecision::modulo(result.m_data, m_data, other.m_data);
        result.m_sign = m_sign;
        return result;
    }
}

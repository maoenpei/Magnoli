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

        bool is_same(const LargeIntegerData& val, const LargeIntegerData& val2);
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

        inline bool operator == (const LargeInteger& other);
        inline bool operator != (const LargeInteger& other);

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
        return m_sign == other.m_sign && multiprecision::is_same(m_data, other.m_data);
    }

    bool LargeInteger::operator !=(const LargeInteger& other)
    {
        return m_sign != other.m_sign || !multiprecision::is_same(m_data, other.m_data);
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

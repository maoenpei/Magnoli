#pragma once

#include "LargeIntegerData.h"

namespace mag
{
    namespace multiprecision {
        inline bool is_zero(const LargeIntegerData& val)
        {
            return val.dim == 1 && val.data[0] == LargeIntegerData::IntegerZero;
        }

        void divide(LargeIntegerData& result, LargeIntegerData& val, LargeIntegerData& val2);
        void modulo(LargeIntegerData& result, LargeIntegerData& val, LargeIntegerData& val2);
    }

    class LargeInteger
    {
        LargeIntegerDataStorage m_data;
        int m_sign;
    public:
        inline LargeInteger();
        inline LargeInteger(int val);
        inline LargeInteger(const LargeInteger& copy);
        inline ~LargeInteger();

        inline LargeInteger operator /(const LargeInteger& other);
        inline LargeInteger operator %(const LargeInteger& other);
    };
}

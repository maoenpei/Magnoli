#pragma once

#include "LargeIntegerData.h"

namespace mag
{
    namespace multiprecision {
        void divide(LargeIntegerData& result, LargeIntegerData& val, LargeIntegerData& val2);
        void modulo(LargeIntegerData& result, LargeIntegerData& val, LargeIntegerData& val2);
    }

    class LargeInteger
    {
    public:
    };
}

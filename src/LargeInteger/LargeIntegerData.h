#pragma once

namespace mag
{
    struct LargeIntegerData
    {
        using IntegerType = unsigned int;
        using DoubleIntType = std::conditional<sizeof(int) == sizeof(long), long long, long>::type;
        static const int IntBitCount = sizeof(IntegerType) * 8;

        IntegerType* data;
        int dim;
    };
}

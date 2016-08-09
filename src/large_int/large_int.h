#pragma once

#include <type_traits>

struct LargeIntegerData
{
    using Integer = unsigned int;
    using DoubleInt = std::conditional<sizeof(int) == sizeof(long), long long, long>::type;
    static const int IntBitCount = sizeof(Integer) * 8;

    Integer* data;
    int dim;
    Integer all_data[1024];

    LargeIntegerData();
    LargeIntegerData(const LargeIntegerData& copy);
    ~LargeIntegerData();
    LargeIntegerData& operator =(const LargeIntegerData& copy);
    LargeIntegerData& operator =(LargeIntegerData&& ref);

    void reset(int dim);
    void finish();
};

void divide_internal(LargeIntegerData& result, LargeIntegerData& remain, const LargeIntegerData& x, const LargeIntegerData& y);
void multi_internal(LargeIntegerData& result, const LargeIntegerData& x, const LargeIntegerData& y);

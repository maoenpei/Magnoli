#include "large_int.h"

#include <assert.h>
#include <memory.h>

#include <algorithm>
#include <utility>

#define SIZ(I)          ((I) * sizeof(LargeIntegerData::Integer))

LargeIntegerData::LargeIntegerData()
    : data(all_data/*new Integer[1]*/)
    , dim(1)
{
    data[0] = 0;
}

LargeIntegerData::LargeIntegerData(const LargeIntegerData& copy)
    : data(all_data/*new Integer[copy.dim]*/)
    , dim(copy.dim)
{
    memcpy(data, copy.data, SIZ(dim));
}

LargeIntegerData::~LargeIntegerData()
{
    //delete [] data;
}

LargeIntegerData& LargeIntegerData::operator =(const LargeIntegerData& copy)
{
    if (copy.dim > dim) {
        //delete [] data;
        data = all_data/*new Integer[copy.dim]*/;
    }
    dim = copy.dim;
    memcpy(data, copy.data, SIZ(dim));
    return *this;
}

LargeIntegerData& LargeIntegerData::operator =(LargeIntegerData&& ref)
{
    std::swap(data, ref.data);
    std::swap(dim, ref.dim);
    return *this;
}

void LargeIntegerData::reset(int dim)
{
    if (dim > this->dim) {
        //delete [] this->data;
        this->data = all_data/*new Integer[dim]*/;
    }
    memset(this->data, 0, SIZ(dim));
    this->dim = dim;
}

void LargeIntegerData::finish()
{
    for (int i = dim - 1; i > 0; --i) {
        if (data[i] == 0) {
            --dim;
        } else {
            break;
        }
    }
}

static void divide_internal_single(LargeIntegerData& result, LargeIntegerData& remain, const LargeIntegerData& x, LargeIntegerData::Integer y)
{
    LargeIntegerData::DoubleInt carry = 0;

    result.reset(x.dim);
    for (int i = x.dim - 1; i >= 0; --i) {
        carry = (carry << LargeIntegerData::IntBitCount) | x.data[i];
        result.data[i] = static_cast<LargeIntegerData::Integer>(carry / y);
        carry = carry % y;
    }
    result.finish();

    remain.reset(1);
    remain.data[0] = static_cast<LargeIntegerData::Integer>(carry);
}

static void divide_internal_lshift_int(LargeIntegerData& result, const LargeIntegerData&x, int shift)
{
    result.reset(x.dim + shift);
    memmove(&result.data[shift], &x.data[0], SIZ(result.dim));
}

static int divide_internal_reduce_fraction(LargeIntegerData& x, LargeIntegerData& y)
{
    int dim = std::min(x.dim, y.dim);
    for (int i = 0; i < dim; ++i) {
        if (x.data[i] != 0 || y.data[i] != 0) {
            memmove(&x.data[0], &x.data[i], SIZ(i));
            memmove(&y.data[0], &y.data[i], SIZ(i));
            x.dim -= i;
            y.dim -= i;
            return i;
        }
    }
    // Shall not go here.
    return 0;
}

struct DataPos
{
    int p;
    int o;
    DataPos();
    DataPos(int p, int o);
    void next();
};

DataPos::DataPos()
    : p(0)
    , o(0)
{}

DataPos::DataPos(int p, int o)
    : p(p)
    , o(o)
{}

void DataPos::next()
{
    if (p == 0 && o == 0) {
        return;
    }

    if (o == 0) {
        --p;
        o = LargeIntegerData::IntBitCount-1;
        return;
    }

    --o;
}

template <typename T, int Offset = 0, int BitCount = sizeof(T) * 8>
struct SeekBit
{
    static const int TotalCount = sizeof(T) * 8;
    static const int HalfCount = BitCount / 2;
    static int highest_one(T t) {
        return ((t >> (Offset + HalfCount)) & (~((~((T)0)) << HalfCount)) ?
                SeekBit<T, Offset + HalfCount, HalfCount>::highest_one(t) :
                SeekBit<T, Offset, HalfCount>::highest_one(t));
    }
};

template <typename T, int Offset>
struct SeekBit<T, Offset, 1>
{
    static int highest_one(T t) {
        return Offset;
    }
};

DataPos divide_internal_pos(const LargeIntegerData& x)
{
    DataPos pos;
    for (int i = x.dim-1; i >= 0; --i) {
        if (x.data[i]) {
            pos.p = i;
            pos.o = SeekBit<LargeIntegerData::Integer>::highest_one(x.data[i]);
            break;
        }
    }
    return pos;
}

void divide_internal_lshift_bits(LargeIntegerData& result, const LargeIntegerData& x, const DataPos& offset)
{
    LargeIntegerData::Integer first = x.data[x.dim - 1] >> (LargeIntegerData::IntBitCount - offset.o);
    result.reset(x.dim + offset.p + (first ? 1 : 0));
    if (first) {
        result.data[result.dim - 1] = first;
    }
    for (int i = x.dim - 1; i > 0; --i) {
        result.data[i + offset.p] = (x.data[i] << offset.o) | (x.data[i-1] >> (LargeIntegerData::IntBitCount - offset.o));
    }
    result.data[offset.p] = x.data[0] << offset.o;
}

DataPos divide_internal_guess_sub(LargeIntegerData& tobeSub, const LargeIntegerData& divided, const LargeIntegerData& divisor, const DataPos& posDiv)
{
    DataPos offset;
    DataPos pos = divide_internal_pos(divided);

    offset.p = pos.p - posDiv.p;
    offset.o = pos.o - posDiv.o;
    if (offset.o < 0) {
        --offset.p;
        offset.o += LargeIntegerData::IntBitCount;
    }

    divide_internal_lshift_bits(tobeSub, divisor, offset);
    return offset;
}

//
bool divide_internal_greater_than(const LargeIntegerData& x, const LargeIntegerData& y)
{
    if (x.dim != y.dim) {
        return x.dim > y.dim;
    }

    for (int i = x.dim-1; i >= 0; --i) {
        if (x.data[i] != y.data[i]) {
            return x.data[i] > y.data[i];
        }
    }
    return false;
}

//
bool divide_internal_less_than(const LargeIntegerData& x, const LargeIntegerData& y)
{
    if (x.dim != y.dim) {
        return x.dim < y.dim;
    }

    for (int i = x.dim-1; i >= 0; --i) {
        if (x.data[i] != y.data[i]) {
            return x.data[i] < y.data[i];
        }
    }
    return false;
}

//
void divide_internal_sub(LargeIntegerData& result, const LargeIntegerData& x, const LargeIntegerData& y)
{
    LargeIntegerData::Integer abdicate = 0;
    result.reset(x.dim);
    for (int i = 0; i < x.dim; ++i) {
        LargeIntegerData::Integer yi = (i < y.dim ? y.data[i] : 0);
        result.data[i] = x.data[i] - abdicate - yi;
        if (x.data[i] < abdicate || x.data[i] - abdicate < yi) {
            abdicate = 1;
        }
    }
    result.finish();
}

void divide_internal(LargeIntegerData& result, LargeIntegerData& remain, const LargeIntegerData& x, const LargeIntegerData& y)
{
    if (divide_internal_greater_than(y, x)) {
        result = LargeIntegerData();
        remain = x;
        return;
    }

    LargeIntegerData divided = x;
    LargeIntegerData divisor = y;

    int shift = divide_internal_reduce_fraction(divided, divisor);

    if (divisor.dim == 1) {
        LargeIntegerData subResult;
        divide_internal_single(result, subResult, divided, divisor.data[0]);
        divided = std::move(subResult);
    } else {
        int diml = x.dim - y.dim + 1;
        result.reset(diml);

        DataPos posDiv = divide_internal_pos(divisor);
        LargeIntegerData tobeSub, subResult;
        do {
            DataPos offset = divide_internal_guess_sub(tobeSub, divided, divisor, posDiv);
            if (divide_internal_less_than(divided, tobeSub)) {
                offset.next();
                divide_internal_lshift_bits(tobeSub, divisor, offset);
            }
            result.data[offset.p] |= ((LargeIntegerData::Integer)1) << offset.o;
            divide_internal_sub(subResult, divided, tobeSub);
            divided = std::move(subResult);
        } while(!divide_internal_less_than(divided, divisor));

        result.finish();
    }

    divide_internal_lshift_int(remain, divided, shift);
}

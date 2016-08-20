#pragma once

#include "LargeIntegerData.h"

namespace mag
{
    namespace util
    {
        struct DataPos
        {
            int p;
            int o;
            inline DataPos();
            inline DataPos(int shift);
            inline DataPos(int p, int o);
            inline DataPos(const DataPos& other);

            inline bool operator ==(const DataPos& other);
            inline DataPos& operator --();
            inline DataPos operator -(const DataPos& other);
        };

        DataPos::DataPos()
            : p(0)
            , o(0)
        {}

        DataPos::DataPos(int shift)
            : p(shift / LargeIntegerData::IntBitCount)
            , o(shift % LargeIntegerData::IntBitCount)
        {}

        DataPos::DataPos(int p, int o)
            : p(p)
            , o(o)
        {}

        DataPos::DataPos(const DataPos& other)
            : p(other.p)
            , o(other.o)
        {}

        bool DataPos::operator ==(const DataPos& other)
        {
            return o == other.o && p == other.p;
        }

        DataPos& DataPos::operator --()
        {
            if (p == 0 && o == 0) {
                return *this;
            }

            if (o == 0) {
                --p;
                o = LargeIntegerData::IntBitCount-1;
                return *this;
            }

            --o;
            return *this;
        }

        DataPos DataPos::operator -(const DataPos& other)
        {
            DataPos d(p - other.p, o - other.o);

            if (d.o < 0) {
                -- d.p;
                d.o += LargeIntegerData::IntBitCount;
            }

            return d;
        }

        template <typename T, int Offset = 0, int BitCount = sizeof(T) * 8>
        struct SeekBit
        {
            static const int TotalCount = sizeof(T) * 8;
            static const int HalfCount = BitCount / 2;
            inline static int highest_one(T t) {
                return ((t >> (Offset + HalfCount)) & (~((~((T)0)) << HalfCount)) ?
                        SeekBit<T, Offset + HalfCount, HalfCount>::highest_one(t) :
                        SeekBit<T, Offset, HalfCount>::highest_one(t));
            }
        };

        template <typename T, int Offset>
        struct SeekBit<T, Offset, 1>
        {
            inline static int highest_one(T t) {
                return Offset;
            }
        };
    }
}

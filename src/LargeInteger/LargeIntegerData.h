#pragma once

#include <type_traits>
#include <memory>

namespace mag
{
    struct LargeIntegerData
    {
        using IntegerType = unsigned int;
        using DoubleIntType = std::conditional<sizeof(int) == sizeof(long), unsigned long long, unsigned long>::type;
        static const int IntBitCount = sizeof(IntegerType) * 8;
        static const IntegerType IntegerZero = IntegerType(0);
        static const IntegerType IntegerMax = ~IntegerZero;
        static const DoubleIntType DoubleIntZero = DoubleIntType(0);
        static const DoubleIntType DoubleIntMax = ~DoubleIntZero;

        IntegerType* data;
        int dim;

        inline LargeIntegerData();

        inline void reset(int dim);
        inline void reset(const LargeIntegerData& copy);
        inline void finalize();

    protected:
        virtual void alloc(int dim) = 0;
    };

#define SIZ(I)          ((I) * sizeof(LargeIntegerData::IntegerType))

    LargeIntegerData::LargeIntegerData()
        : data(nullptr)
        , dim(0)
    {}

    void LargeIntegerData::reset(int dim)
    {
        if (dim > this->dim) {
            alloc(dim);
        }
        this->dim = dim;
        memset(data, 0, SIZ(dim));
    }

    void LargeIntegerData::reset(const LargeIntegerData& copy)
    {
        if (copy.dim > this->dim) {
            alloc(copy.dim);
        }
        this->dim = copy.dim;
        memcpy(data, copy.data, SIZ(dim));
    }

    void LargeIntegerData::finalize()
    {
        for (int i = dim - 1; i > 0; --i) {
            if (data[i]) {
                break;
            }
            -- dim;
        }
    }

    struct LargeIntegerDataStorage : public LargeIntegerData
    {
        int size;
        LargeIntegerDataStorage();
        LargeIntegerDataStorage(IntegerType val);
        LargeIntegerDataStorage(const LargeIntegerDataStorage& copy);
        LargeIntegerDataStorage(LargeIntegerDataStorage&& copy);
        ~LargeIntegerDataStorage();

        LargeIntegerDataStorage& operator =(const LargeIntegerDataStorage& copy);
        LargeIntegerDataStorage& operator =(LargeIntegerDataStorage&& copy);

    private:
        void* threadEnv;

    protected:
        virtual void alloc(int dim) override;
    };

    struct LargeIntegerDataTmp : public LargeIntegerData
    {
        LargeIntegerDataTmp& swap(LargeIntegerDataTmp&& copy);

        static void clearCache();

    protected:
        virtual void alloc(int dim) override;
    };

}

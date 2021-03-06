#include "LargeInteger.h"

#include "PosUtil.h"
#include <algorithm>
#include <functional>

namespace mag
{
    namespace multiprecision
    {
        template <typename OP>
        inline static bool internal_compare(const LargeIntegerData& val, const LargeIntegerData& val2, const OP& op)
        {
            if (val.dim != val2.dim) {
                return op(val.dim, val2.dim);
            }

            for (int i = val.dim-1; i >= 0; --i) {
                if (val.data[i] != val2.data[i]) {
                    return op(val.data[i], val2.data[i]);
                }
            }
            return false;
        }

        inline static int internal_reduce_fraction(LargeIntegerData& val, LargeIntegerData& val2)
        {
            int dim = std::min(val.dim, val2.dim);
            for (int i = 0; i < dim; ++i) {
                if (val.data[i] != 0 || val2.data[i] != 0) {
                    memmove(&val.data[0], &val.data[i], SIZ(val.dim - i));
                    memmove(&val2.data[0], &val2.data[i], SIZ(val2.dim - i));
                    val.dim -= i;
                    val2.dim -= i;
                    return i;
                }
            }
            // Shall not go here.
            return 0;
        }

        inline static void internal_lshift_int(LargeIntegerData& result, const LargeIntegerData&val, int shift)
        {
            result.reset(val.dim + shift);
            memcpy(&result.data[shift], &val.data[0], SIZ(val.dim));
        }

        inline static void internal_lshift_bits(LargeIntegerData& result, const LargeIntegerData& val, const util::DataPos& offset)
        {
            if (offset.o == 0) {
                internal_lshift_int(result, val, offset.p);
                return;
            }

            LargeIntegerData::IntegerType first = val.data[val.dim - 1] >> (LargeIntegerData::IntBitCount - offset.o);

            if (first) {
                result.reset(val.dim + offset.p + 1);
                result.data[result.dim - 1] = first;
            } else {
                result.reset(val.dim + offset.p);
            }
            for (int i = val.dim - 1; i > 0; --i) {
                result.data[i + offset.p] = (val.data[i] << offset.o) | (val.data[i - 1] >> (LargeIntegerData::IntBitCount - offset.o));
            }
            result.data[offset.p] = val.data[0] << offset.o;
        }

        inline static void internal_rshift_int(LargeIntegerData& result, const LargeIntegerData&val, int shift)
        {
            if (val.dim <= shift) {
                result.reset(1);
                return;
            }

            result.reset(val.dim - shift);
            memcpy(&result.data[0], &val.data[shift], SIZ(result.dim));
        }

        inline static void internal_rshift_bits(LargeIntegerData& result, const LargeIntegerData& val, const util::DataPos& offset)
        {
            if (offset.o == 0) {
                internal_rshift_int(result, val, offset.p);
                return;
            }

            LargeIntegerData::IntegerType first = val.data[val.dim - 1] >> offset.o;

            if ((first && val.dim <= offset.p) || (!first && val.dim <= offset.p - 1)) {
                result.reset(1);
                return;
            }

            if (first) {
                result.reset(val.dim - offset.p);
                result.data[result.dim - 1] = first;
            } else {
                result.reset(val.dim - offset.p - 1);
            }
            for (int i = val.dim - offset.p - 2; i >= 0; --i) {
                result.data[i] = (val.data[i + offset.p] >> offset.o) | (val.data[i + offset.p + 1] << (LargeIntegerData::IntBitCount - offset.o));
            }
        }

        inline static void internal_subtract(LargeIntegerData& val, const LargeIntegerData& val2)
        {
            LargeIntegerData::IntegerType abdicate = LargeIntegerData::IntegerZero;
            for (int i = 0; i < val.dim; ++i) {
                LargeIntegerData::IntegerType tmp = (i < val2.dim ? val2.data[i] : LargeIntegerData::IntegerZero);
                LargeIntegerData::IntegerType tmp2 = val.data[i] - abdicate - tmp;
                abdicate = (val.data[i] < tmp || val.data[i] - tmp < abdicate);
                val.data[i] = tmp2;
            }
            val.finalize();
        }

        // Not mentioned
        inline static util::DataPos internal_pos(const LargeIntegerData& val)
        {
            return util::DataPos(
                val.dim,
                util::SeekBit<LargeIntegerData::IntegerType>::highest_one(val.data[val.dim - 1])
                );
        }

        // Not mentioned
        template <bool hasQuot, bool hasRemain>
        inline static void internal_divide_multiple(LargeIntegerData& quot, LargeIntegerData& val, const LargeIntegerData& val2)
        {
            util::DataPos pos2 = internal_pos(val2);

            LargeIntegerDataTmp tmp;
            util::DataPos lastTmp(-1, -1);
            bool first = true;
            std::less<> ls;
            do {
                util::DataPos pos = internal_pos(val);
                util::DataPos posTmp = pos - pos2;
                if (lastTmp == posTmp) {
                    -- posTmp;
                    internal_lshift_bits(tmp, val2, posTmp);
                } else {
                    internal_lshift_bits(tmp, val2, posTmp);
                    if (internal_compare(val, tmp, ls)) {
                        -- posTmp;
                        internal_lshift_bits(tmp, val2, posTmp);
                    }
                }
                lastTmp = posTmp;
                if (hasQuot) {
                    if (first) {
                        quot.reset(posTmp.p + 1);
                        first = false;
                    }
                    quot.data[posTmp.p] |= LargeIntegerData::IntegerType(1) << posTmp.o;
                }
                internal_subtract(val, tmp);
            } while(!internal_compare(val, val2, ls));
        }

        inline static void internal_plus_lshift(LargeIntegerData& result, LargeIntegerData::IntegerType val, int shift)
        {
            LargeIntegerData::IntegerType carry = LargeIntegerData::IntegerZero;
            for (int i = shift; i < result.dim; ++i) {
                LargeIntegerData::IntegerType tmp = i == shift ? val : LargeIntegerData::IntegerZero;
                LargeIntegerData::IntegerType tmp2 = result.data[i] + carry + tmp;
                carry = (result.data[i] > LargeIntegerData::IntegerMax - tmp || result.data[i] + tmp > LargeIntegerData::IntegerMax - carry);
                result.data[i] = tmp2;
                if (!carry) {
                    break;
                }
            }
        }

        inline static void internal_subtract_lshift(LargeIntegerData& result, LargeIntegerData::IntegerType val, int shift)
        {
            LargeIntegerData::IntegerType abdicate = LargeIntegerData::IntegerZero;
            for (int i = shift; i < result.dim; ++i) {
                LargeIntegerData::IntegerType tmp = (i == shift ? val : LargeIntegerData::IntegerZero);
                LargeIntegerData::IntegerType tmp2 = result.data[i] - abdicate - tmp;
                abdicate = (result.data[i] < tmp || result.data[i] - tmp < abdicate);
                result.data[i] = tmp2;
                if (!abdicate) {
                    break;
                }
            }
        }

        inline static void internal_multiply_single_lshift(LargeIntegerData& result, const LargeIntegerData& val, LargeIntegerData::IntegerType val2, int shift)
        {
            result.reset(val.dim + shift + 1);
            for (int i = 0; i < val.dim; ++i) {
                LargeIntegerData::DoubleIntType tmp = LargeIntegerData::DoubleIntType(val.data[i]) * LargeIntegerData::DoubleIntType(val2);
                result.data[i + shift + 1] += tmp >> LargeIntegerData::IntBitCount;
                result.data[i + shift] += tmp & LargeIntegerData::IntegerMask;
            }
            result.finalize();
        }

        template <bool hasQuot, bool hasRemain>
        inline static void internal_divide_multiple2(LargeIntegerData& quot, LargeIntegerDataTmp& val, const LargeIntegerData& val2)
        {
            LargeIntegerDataTmp tmp;
            int nag = 0;

            if (hasQuot) {
                quot.reset(val.dim - val2.dim + 1);
            }
            std::less<> ls;
            do {
                int diml = val.dim - val2.dim;
                LargeIntegerData::IntegerType guess;
                LargeIntegerData::DoubleIntType& dvd = *reinterpret_cast<LargeIntegerData::DoubleIntType*>(&val.data[val.dim - 2]);
                if (val.data[val.dim - 1] == val2.data[val2.dim - 1]) {
                    if (val.data[val.dim - 2] >= val2.data[val2.dim - 2]) {
                        LargeIntegerData::DoubleIntType& pdsr = *reinterpret_cast<LargeIntegerData::DoubleIntType*>(&val2.data[val2.dim - 2]);
                        guess = static_cast<LargeIntegerData::IntegerType>(dvd / pdsr);
                    } else {
                        -- diml;
                        guess = static_cast<LargeIntegerData::IntegerType>(dvd / (val2.data[val2.dim - 1] + 1));
                        guess += (LargeIntegerData::IntegerMax - guess) >> 1;
                    }
                } else if (val.data[val.dim - 1] > val2.data[val2.dim - 1]) {
                    LargeIntegerData::DoubleIntType& pdsr = *reinterpret_cast<LargeIntegerData::DoubleIntType*>(&val2.data[val2.dim - 2]);
                    guess = static_cast<LargeIntegerData::IntegerType>(dvd / pdsr);
                } else {
                    -- diml;
                    LargeIntegerData::IntegerType& pdsr = val2.data[val2.dim - 1];
                    guess = static_cast<LargeIntegerData::IntegerType>(dvd / pdsr);
                }
                if (hasQuot) {
                    (nag ? internal_subtract_lshift(quot, guess, diml) : internal_plus_lshift(quot, guess, diml));
                }
                internal_multiply_single_lshift(tmp, val2, guess, diml);
                if (internal_compare(val, tmp, ls)) {
                    nag = !nag;
                    val.swap(std::move(tmp));
                }
                internal_subtract(val, tmp);
            } while(!internal_compare(val, val2, ls));

            if (nag) {
                if (hasQuot) {
                    internal_subtract_lshift(quot, LargeIntegerData::IntegerType(1), 0);
                }
                if (hasRemain) {
                    tmp.reset(val2);
                    val.swap(std::move(tmp));
                    internal_subtract(val, tmp);
                }
            }
            if (hasQuot) {
                quot.finalize();
            }
        }

        template <bool hasQuot, bool hasRemain>
        inline static void internal_divide_single(LargeIntegerData& quot, LargeIntegerData& val, LargeIntegerData::IntegerType val2)
        {
            if (hasQuot) {
                if (val.data[val.dim - 1] >= val2) {
                    quot.reset(val.dim);
                    quot.data[val.dim - 1] = val.data[val.dim - 1] / val2;
                } else {
                    quot.reset(val.dim - 1);
                }
            }
            val.data[val.dim - 1] %= val2;
            
            for (int i = val.dim - 2; i >= 0; --i) {
                LargeIntegerData::DoubleIntType& temp = *reinterpret_cast<LargeIntegerData::DoubleIntType*>(&val.data[i]);
                if (hasQuot) {
                    quot.data[i] = static_cast<LargeIntegerData::IntegerType>(temp / val2);
                }
                val.data[i] = static_cast<LargeIntegerData::IntegerType>(temp % val2);
                if (hasRemain) {
                    val.data[i+1] = LargeIntegerData::IntegerZero;
                }
            }

            if (hasRemain) {
                val.finalize();
            }
        }

        template <bool hasQuot, bool hasRemain>
        inline static void internal_divide(LargeIntegerData& quot, LargeIntegerData& remain, const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            if (internal_compare(val, val2, std::less<>())) {
                if (hasQuot) {
                    quot.reset(1);
                }
                if (hasRemain) {
                    remain.reset(val);
                }
                return;
            }

            LargeIntegerDataTmp divided, divisor;
            divided.reset(val);
            divisor.reset(val2);

            int shift = 0;
            if (!divided.data[0] && !divisor.data[0]) {
                shift = internal_reduce_fraction(divided, divisor);
            }

            if (divisor.dim == 1) {
                internal_divide_single<hasQuot, hasRemain>(quot, divided, divisor.data[0]);
            } else {
                internal_divide_multiple2<hasQuot, hasRemain>(quot, divided, divisor);
            }

            if (hasRemain) {
                if (is_zero(divided)) {
                    remain.reset(divided);
                } else {
                    internal_lshift_int(remain, divided, shift);
                }
            }
        }

        void divide(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            LargeIntegerDataTmp tmp;
            internal_divide<true, false>(result, tmp, val, val2);
            LargeIntegerDataTmp::clearCache();
        }

        void modulo(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            LargeIntegerDataTmp tmp;
            internal_divide<false, true>(tmp, result, val, val2);
            LargeIntegerDataTmp::clearCache();
        }

        bool is_equal_to(const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            return val.dim == val2.dim && 0 == memcmp(val.data, val2.data, SIZ(val.dim));
        }

        bool is_less_than(const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            return internal_compare(val, val2, std::less<>());
        }

        bool is_greater_than(const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            return internal_compare(val, val2, std::greater<>());
        }

        void plus(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            int dim = std::max(val.dim, val2.dim);
            LargeIntegerData::IntegerType carry = LargeIntegerData::IntegerZero;
            result.reset(dim + 1);
            for (int i = 0; i < dim; ++i) {
                LargeIntegerData::IntegerType tmp = (i < val.dim ? val.data[i] : LargeIntegerData::IntegerZero);
                LargeIntegerData::IntegerType tmp2 = (i < val2.dim ? val2.data[i] : LargeIntegerData::IntegerZero);
                result.data[i] = tmp + tmp2 + carry;
                carry = (tmp > LargeIntegerData::IntegerMax - tmp2 ||
                         tmp + tmp2 > LargeIntegerData::IntegerMax - carry);
            }
            result.data[dim] = carry;
            result.finalize();
        }

        int subtract(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            if (internal_compare(val, val2, std::less<>())) {
                result.reset(val2);
                internal_subtract(result, val);
                return 1;
            } else {
                result.reset(val);
                internal_subtract(result, val2);
                return 0;
            }
        }

        void multiply(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            result.reset(val.dim + val2.dim);
            for (int i = 0; i < val.dim; ++i) {
                for (int j = 0; j < val2.dim; ++j) {
                    LargeIntegerData::DoubleIntType tmp = LargeIntegerData::DoubleIntType(val.data[i]) * LargeIntegerData::DoubleIntType(val2.data[j]);
                    result.data[i + j + 1] = tmp >> LargeIntegerData::IntBitCount;
                    result.data[i + j] = tmp & LargeIntegerData::IntegerMask;
                }
            }
            result.finalize();
        }

        void inverse(LargeIntegerData& result, const LargeIntegerData& val)
        {
            result.reset(val.dim);
            for (int i = 0; i < result.dim; ++i) {
                result.data[i] = ~val.data[i];
            }
            result.finalize();
        }

        void xor(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            result.reset(std::max(val.dim, val2.dim));
            for (int i = 0; i < result.dim; ++i) {
                result.data[i] = val.data[i] ^ val2.data[i];
            }
        }

        void and(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            result.reset(std::min(val.dim, val2.dim));
            for (int i = 0; i < result.dim; ++i) {
                result.data[i] = val.data[i] & val2.data[i];
            }
            result.finalize();
        }

        void or(LargeIntegerData& result, const LargeIntegerData& val, const LargeIntegerData& val2)
        {
            result.reset(std::max(val.dim, val2.dim));
            for (int i = 0; i < result.dim; ++i) {
                result.data[i] = val.data[i] | val2.data[i];
            }
        }

        void lshift(LargeIntegerData& result, const LargeIntegerData& val, int shift)
        {
            internal_lshift_bits(result, val, shift);
        }

        void rshift(LargeIntegerData& result, const LargeIntegerData& val, int shift)
        {
            internal_rshift_bits(result, val, shift);
        }
    }
}

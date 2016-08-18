
#include "large_int/large_int.h"
#include "LargeInteger/LargeInteger.h"

int main(int argc, char** argv)
{
    mag::LargeIntegerDataStorage v1, v2, v3, v4;
    v3.reset(5);
    v3.data[4] = 101;
    v4.reset(3);
    v4.data[2] = 101;
    v4.data[0] = 0xffffffff;
    //for (int i = 0; i<10000000; ++i) {
        mag::multiprecision::divide(v1, v3, v4);
        mag::multiprecision::modulo(v2, v3, v4);
    //}
    mag::LargeInteger l(22), r(-33);
    auto i1 = l / r;
    auto i2 = l % r;
    auto i3 = l + r;
    auto i4 = l - r;
    auto i5 = l * r;
	return 0;
}

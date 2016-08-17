
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
    mag::LargeInteger l3, l4;
    l3.m_data.reset(5);
    l3.m_data.data[4] = 101;
    l4.m_data.reset(3);
    l4.m_data.data[2] = 101;
    l4.m_data.data[0] = 0xffffffff;
    auto l1 = l3 / l4;
    auto l2 = l3 % l4;
    l1++;
	return 0;
}

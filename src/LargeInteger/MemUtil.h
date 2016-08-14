#pragma once

#include <list>

namespace mag
{
    namespace util
    {
        class MemUtil
        {
            std::list<unsigned char*> m_memList;
            unsigned char* m_currPtr;
            int m_currLimit;

        public:
            MemUtil(int initSize);
            ~MemUtil();

            void* query(int siz);
            void reset();
        };
    }
}

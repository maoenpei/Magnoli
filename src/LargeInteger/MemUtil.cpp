#include "MemUtil.h"

#include <algorithm>

namespace mag
{
    namespace util
    {
        MemUtil::MemUtil(int initSize)
            : m_currLimit(initSize)
        {
            m_memList.push_front(new unsigned char[initSize]);
            m_currPtr = m_memList.front();
        }

        MemUtil::~MemUtil()
        {
            for (auto ptr : m_memList) {
                delete [] ptr;
            }
        }

        void* MemUtil::query(int siz)
        {
            unsigned char* ptr = m_currPtr;
            if (ptr + siz > m_memList.front() + m_currLimit) {
                int tmpVal = std::max(siz, m_currLimit * 2);
                m_memList.push_front(new unsigned char[tmpVal]);
                ptr = m_currPtr = m_memList.front();
                m_currLimit = tmpVal;
            }
            m_currPtr += siz;
            return ptr;
        }

        void MemUtil::reset()
        {
            m_currPtr = m_memList.front();
            auto itr = m_memList.begin();
            ++itr;
            while (itr != m_memList.end()) {
                delete [] *itr;
                itr = m_memList.erase(itr);
            }
        }
    }
}

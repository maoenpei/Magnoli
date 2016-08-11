#pragma once

#include <functional>

namespace mag
{
    namespace util
    {
        using TlsCreateFunc = std::function<void*(void)>;
        using TlsDestroyFunc = std::function<void(void*)>;

        class TlsData
        {
        public:
            void* get();

            TlsData(TlsCreateFunc createFunctor, TlsDestroyFunc destroyFunctor);
            ~TlsData();

        protected:
            static void tls_destructor(void* data);
            void* m_Key;
            TlsCreateFunc m_CreateFunctor;
            TlsDestroyFunc m_DestroyFunctor;
        };
    }
}

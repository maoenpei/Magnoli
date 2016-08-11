#include "TlsUtil.h"

#include <pthread.h>
static void* tlsCreateKey(void (*destructor) (void *)) {
    pthread_key_t tlsKey;
    pthread_key_create(&tlsKey, destructor);
    return reinterpret_cast<void*>(tlsKey);
}
static void tlsDeleteKey(void* key) {
    pthread_key_t tlsKey = reinterpret_cast<pthread_key_t>(key);
    pthread_key_delete(tlsKey);
}
static void* tlsGet(void* key) {
    pthread_key_t tlsKey = reinterpret_cast<pthread_key_t>(key);
    return pthread_getspecific(tlsKey);
}
static void tlsSet(void* key, void* val) {
    pthread_key_t tlsKey = reinterpret_cast<pthread_key_t>(key);
    pthread_setspecific(tlsKey, val);
}

namespace mag
{
    namespace util
    {
        struct TlsUserData
        {
            TlsData* pKey;
            void* pVal;
        };

        void* TlsData::get()
        {
            void* val = tlsGet(m_Key);
            if (!val) {
                TlsUserData* userData = new TlsUserData();
                userData->pKey = this;
                userData->pVal = m_CreateFunctor();
                tlsSet(m_Key, userData);
            }
            return val;
        }

        void TlsData::tls_destructor(void* data) {
            TlsUserData* userData = reinterpret_cast<TlsUserData*>(data);
            userData->pKey->m_DestroyFunctor(userData->pVal);
            delete userData;
        }

        TlsData::TlsData(TlsCreateFunc createFunctor, TlsDestroyFunc destroyFunctor)
            : m_CreateFunctor(createFunctor)
            , m_DestroyFunctor(destroyFunctor)
        {
            m_Key = tlsCreateKey(tls_destructor);
        }

        TlsData::~TlsData()
        {
            tlsDeleteKey(m_Key);
        }
    }
}

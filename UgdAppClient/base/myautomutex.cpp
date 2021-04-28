#include "myautomutex.h"

CMyAutoMutex::CMyAutoMutex(QMutex *pMutex)
{
    m_pMutex = pMutex;
    if (m_pMutex != nullptr)
    {
        m_pMutex->lock();
    }
}


CMyAutoMutex::~CMyAutoMutex()
{
    if (m_pMutex != nullptr)
    {
        m_pMutex->unlock();
        m_pMutex = nullptr;
    }
}

#include "mythread.h"
#include "myautomutex.h"

CMyThread::CMyThread(QObject *parent) : QThread(parent)
{

}

void CMyThread::setQuitFlag(bool flag)
{
    CMyAutoMutex aMutex(&m_quitLock);
    m_quitFlag = flag;
}

bool CMyThread::getQuitFlag()
{
    CMyAutoMutex aMutex(&m_quitLock);
    return m_quitFlag;
}

void CMyThread::startRun()
{
    setQuitFlag(false);
    start();
}

void CMyThread::stopRun()
{
    if(isRunning())
    {
        setQuitFlag(true);
        quit();
        wait();
    }
}

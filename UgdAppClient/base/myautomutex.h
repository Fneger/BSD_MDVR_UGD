#ifndef CMYAUTOMUTEX_H
#define CMYAUTOMUTEX_H

#include <QMutex>

class CMyAutoMutex
{
public:
    CMyAutoMutex(QMutex *pMutex);
    virtual ~CMyAutoMutex();

private:
    QMutex *m_pMutex;
};

#endif // CMYAUTOMUTEX_H

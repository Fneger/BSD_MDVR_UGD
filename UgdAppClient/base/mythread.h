#ifndef CMYTHREAD_H
#define CMYTHREAD_H

#include <QThread>
#include <QMutex>

class CMyThread : public QThread
{
    Q_OBJECT
public:
    explicit CMyThread(QObject *parent = nullptr);
    void setQuitFlag(bool flag);
    bool getQuitFlag();
    void startRun();
    void stopRun();

signals:

public slots:

private:
    bool m_quitFlag; //true:quit
    QMutex m_quitLock;

};

#endif // CMYTHREAD_H

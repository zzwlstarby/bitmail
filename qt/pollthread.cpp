#include <QDebug>

#include "pollthread.h"

#include <bitmailcore/bitmail.h>

static
int PollEventHandler(unsigned int count, void * p);

static
int PollProgressHandler(RTxState, const char * info, void * userptr);
PollThread::PollThread(BitMail * bm)
    : m_bitmail(bm)
    , m_lastCount(0)
    , m_reIdleInterval(30*1000) // RFC recommended 30 minutes, here less than that.
    , m_fStopFlag(false)
{

}

PollThread::~PollThread()
{

}

void PollThread::run()
{
    m_bitmail->OnPollEvent(PollEventHandler, this);

    while(!m_fStopFlag){

        if (bmOk != m_bitmail->StartIdle(m_reIdleInterval, PollProgressHandler, this)){
            QThread::sleep(5000);
        }
    }

    emit done();
}

void PollThread::stop()
{
    m_fStopFlag = true;
}

void PollThread::NotifyInboxPollEvent(unsigned int count)
{
    if (m_lastCount == count){
        return ;
    }

    m_lastCount = count;

    emit inboxPollEvent();

    return ;
}

void PollThread::NotifyProgress(const QString &info)
{
    emit inboxPollProgress(info);
}

int PollEventHandler(unsigned int count, void * p)
{
    (void)count;    (void)p;
    PollThread * self = (PollThread *)p;
    self->NotifyInboxPollEvent(count);
    return 0;
}

int PollProgressHandler(RTxState, const char *info, void *userptr)
{
    PollThread * self = (PollThread *)userptr;
    if (!self){
        return bmInvalidParam;
    }
    self->NotifyProgress(QString::fromLatin1(info));
    return bmOk;
}

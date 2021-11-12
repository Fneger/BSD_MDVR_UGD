#include "st_client_event.h"
#include <QDebug>
#include <QDateTime>

namespace BdUgdServer {
st_client_event *st_client_event::S_pThis = nullptr;
st_client_event::st_client_event(QObject *parent) : QObject(parent)
{
    m_eventSnRef = 0;
}

st_client_event::~st_client_event()
{

}

st_client_event *st_client_event::Instance()
{
    static QMutex s_dataMutex;
    s_dataMutex.lock();
    if(S_pThis == nullptr)
        S_pThis = new st_client_event;
    s_dataMutex.unlock();
    return S_pThis;
}

void st_client_event::AddClientEvent(EVENT_INFO_S &eventInfo)
{
    eventInfo.eventSn = m_eventSnRef++;
    RemoveClientEvent(eventInfo.eventSn);
    m_eventsMutex.lock();
    m_events[eventInfo.eventSn] = eventInfo;
    m_eventsMutex.unlock();
}

bool st_client_event::RemoveClientEvent(quint64 eventSn)
{
    bool res = false;
    m_eventsMutex.lock();
    if((m_events.find(eventSn) != m_events.end()))
    {
        EVENT_INFO_S &event = m_events[eventSn];
        if(event.status != EventStatusExecuting_E)
        {
            m_events.erase(eventSn);
            res = true;
        }
    }
    m_eventsMutex.unlock();
    return res;
}

void st_client_event::RemoveClientAllEvent()
{
    m_eventsMutex.lock();
    m_events.clear();
    m_eventsMutex.unlock();
}

QMap<quint64,EVENT_INFO_S> st_client_event::GetUuidEvents(uint64_t nUuid)
{
    QMap<quint64,EVENT_INFO_S> uuidEvents;
    m_eventsMutex.lock();
    for(auto& event:m_events)
    {
        if(event.second.ownerUuid == nUuid)
            uuidEvents[event.second.eventSn] = event.second;
    }
    m_eventsMutex.unlock();
    return uuidEvents;
}

QMap<quint64,EVENT_INFO_S> st_client_event::GetTerminalEvents(const TERMINAL_INFO_S &tmnInfo)
{
    QMap<quint64,EVENT_INFO_S> events;
    m_eventsMutex.lock();
    for(auto& event:m_events)
    {
        QString id;
        switch (event.second.idType) {
        case EventRevDeviceNum_E:
            id = tmnInfo.deviceNum;
            break;
        case EventRevVideoId_E:
            id = tmnInfo.videoId;
            break;
        case EventRevPhoneNum_E:
            id = tmnInfo.phoneNumber;
            break;
        case EventRevLicenseNum_E:
            id = tmnInfo.licenseNum;
            break;
        default:

            break;
        }
        if(event.second.id == id && (event.second.status == EventStatusWaitting_E || event.second.status == EventStatusExecuting_E))
        {
            events[event.second.eventSn] = event.second;
        }
    }
    m_eventsMutex.unlock();
    return events;
}

int st_client_event::GetEventStatus(quint64 eventSn)
{
    int status = -1;
    m_eventsMutex.lock();
    if(m_events.find(eventSn) != m_events.end())
    {
        EVENT_INFO_S &event = m_events[eventSn];
        status = event.status;
    }
    m_eventsMutex.unlock();
    return status;
}

void st_client_event::SetEventStatus(BD_REQUEST_SET_EVENT_STATUS_S status)
{
    m_eventsMutex.lock();
    if(m_events.find(status.eventSn) != m_events.end())
    {
        EVENT_INFO_S event = m_events[status.eventSn];
        event.status = status.status;
        event.ackPara1 = status.ackPara1;
        event.ackPara2 = status.ackPara2;
        m_events[status.eventSn] = event;
    }
    m_eventsMutex.unlock();
}

}


#include "xframesender.h"

#include <QDebug>

#define CCHR_SPL '#'    // split char of command string
#define CCHR_IND '@'    // indicate char

const char g_keyEnable[] = "en@";
const char g_keyId[] = "id@";
const char g_keyBus[] = "bus@";
const char g_keyData[] = "data@";
const char g_keyTrigger[] = "tr@";
const char g_keyModifier[] = "mo@";

XFrameSender::XFrameSender(QObject *parent) : QObject(parent)
{

}

FrameSendData *XFrameSender::findSendDataById(quint32 id)
{
    QList<FrameSendData>::iterator end = m_sendingData.end();
    QList<FrameSendData>::iterator it;

    for (it = m_sendingData.begin(); it != end; ++it) {
        if ((*it).id() == id)
            return &(*it);
    }

    return NULL;
}

// en@0
// #id@0x061
// #bus@0
// #data@00112233
// #tr@id0x200 5ms 10x bus0,1000ms
// #mo@id:0x200:D7,D1=ID:0x200:D3+ID:0x200:D4&0xF0
void XFrameSender::slotCmdParser(const QString &cmdString)
{
    QString tokEnable, tokId, tokBus, tokData;
    QString tokTr, tokMod;

    if (cmdString.isEmpty())
        return;

    QStringList sl = cmdString.simplified().split(CCHR_SPL);
    for (int i = 0; i < sl.size(); ++i) {
        const QString &s = sl.at(i);

        if (s.startsWith(g_keyEnable)) {
            tokEnable = s.right(s.length() - strlen(g_keyEnable));
#ifndef F_NO_DEBUG
            qDebug() << tr("enable = %1").arg(tokEnable);
#endif
        } else if (s.startsWith(g_keyId)) {
            tokId = s.right(s.length() - strlen(g_keyId));
#ifndef F_NO_DEBUG
            qDebug() << tr("id = %1").arg(tokId);
#endif
        } else if (s.startsWith(g_keyBus)) {
            tokBus = s.right(s.length() - strlen(g_keyBus));
#ifndef F_NO_DEBUG
            qDebug() << tr("bus = %1").arg(tokBus);
#endif
        } else if (s.startsWith(g_keyData)) {
            tokData = s.right(s.length() - strlen(g_keyData));
#ifndef F_NO_DEBUG
            qDebug() << tr("data = %1").arg(tokData);
#endif
        } else if (s.startsWith(g_keyTrigger)) {
            tokTr = s.right(s.length() - strlen(g_keyTrigger));
#ifndef F_NO_DEBUG
            qDebug() << tr("trigger = %1").arg(tokTr);
#endif
        } else if (s.startsWith(g_keyModifier)) {
            tokMod = s.right(s.length() - strlen(g_keyModifier));
#ifndef F_NO_DEBUG
            qDebug() << tr("modifier = %1").arg(tokMod);
#endif
        }
    }

    if (tokId.isEmpty())
        return;

    bool ok;
    quint32 id = tokId.toUInt(&ok, 0);
    if (!ok) return;

    FrameSendData *pSd = findSendDataById(id);
    if (pSd == NULL) {
        FrameSendData tempSendData;
        m_sendingData.append(tempSendData);
        pSd = &m_sendingData.last();
    }




}

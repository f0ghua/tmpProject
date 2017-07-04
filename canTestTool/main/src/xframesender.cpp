#include "xframesender.h"
#include "utils.h"

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

    pSd->setId(id);
    if (!tokData.isEmpty()) {
        QByteArray ba = QByteArray::fromHex(tokData.toLatin1());
        pSd->setPayload(ba);
    }
    if (!tokBus.isEmpty()) {
        int bus = tokBus.toInt();
        pSd->setBus(bus);
    }
    if (!tokEnable.isEmpty()) {
        int en = tokEnable.toInt();
        pSd->enabled = en;
    }

}

void XFrameSender::processModifierText(const QString &tokMod, FrameSendData *pSd)
{
    QString modString;
    //bool firstOp = true;
    bool abort = false;
    QString token;
    ModifierOp thisOp;

    //Example line:
    //d0 = D0 + 1,d1 = id:0x200:d3 + id:0x200:d4 AND 0xF0 - Original version
    //D0=D0+1,D1=ID:0x200:D3+ID:0x200:D4&0xF0
    //This is certainly much harder to parse than the trigger definitions.
    //the left side of the = has to be D0 to D7. After that there is a string of
    //data. Spaces used to be required but no longer are. This makes parsing harder but data entry easier

    //yeah, lots of operations on this one line but it's for a good cause. Removes the convenience English versions of the
    //logical operators and replaces them with the math equivs. Also uppercases and removes all superfluous whitespace
    modString = tokMod.toUpper().trimmed().replace("AND", "&").replace("XOR", "^").replace("OR", "|").replace(" ", "");
    if (modString != "")
    {
        QStringList mods = modString.split(',');
        pSd->modifiers.clear();
        pSd->modifiers.reserve(mods.length());
        for (int i = 0; i < mods.length(); i++)
        {
            Modifier thisMod;
            thisMod.destByte = 0;
            //firstOp = true;

            QString leftSide = Utils::Base::grabAlphaNumeric(mods[i]);
            if (leftSide.startsWith("D") && leftSide.length() == 2)
            {
                thisMod.destByte = leftSide.right(1).toInt();
                thisMod.operations.clear();
            }
            else
            {
                qDebug() << "Something wrong with lefthand val";
                continue;
            }
            if (!(Utils::Base::grabOperation(mods[i]) == "="))
            {
                qDebug() << "Err: No = after lefthand val";
                continue;
            }
            abort = false;

            token = Utils::Base::grabAlphaNumeric(mods[i]);
            if (token[0] == '~')
            {
                thisOp.first.notOper = true;
                token = token.remove(0, 1); //remove the ~ character
            }
            else thisOp.first.notOper = false;
            parseOperandString(token.split(":"), thisOp.first);

            if (mods[i].length() < 2) {
                abort = true;
                thisOp.operation = ADDITION;
                thisOp.second.ID = 0;
                thisOp.second.databyte = 0;
                thisOp.second.notOper = false;
                thisMod.operations.append(thisOp);
            }

            while (!abort)
            {
                QString operation = Utils::Base::grabOperation(mods[i]);
                if (operation == "")
                {
                    abort = true;
                }
                else
                {
                    thisOp.operation = parseOperation(operation);
                    QString secondOp = Utils::Base::grabAlphaNumeric(mods[i]);
                    if (mods[i][0] == '~')
                    {
                        thisOp.second.notOper = true;
                        mods[i] = mods[i].remove(0, 1); //remove the ~ character
                    }
                    else thisOp.second.notOper = false;
                    thisOp.second.bus = pSd->bus();
                    thisOp.second.ID = pSd->id();
                    parseOperandString(secondOp.split(":"), thisOp.second);
                    thisMod.operations.append(thisOp);
                }

                thisOp.first.ID = -1; //shadow register
                if (mods[i].length() < 2) abort = true;
            }

            pSd->modifiers.append(thisMod);
        }
    }
    //there is no else for the modifiers. We'll accept there not being any
}

void XFrameSender::processTriggerText(const QString &tokTr, FrameSendData *pSd)
{
    QString trigger;

    //Example line:
    //id0x200 5ms 10x bus0,1000ms
    //trigger has two levels of syntactic parsing. First you split by comma to get each
    //actual trigger. Then you split by spaces to get the tokens within each trigger
    //trigger = ui->tableSender->item(line, 5)->text().toUpper().trimmed().replace(" ", "");
    trigger = tokTr.toUpper();
    if (trigger != "")
    {
        QStringList triggers = trigger.split(',');
        pSd->triggers.clear();
        pSd->triggers.reserve(triggers.length());
        for (int k = 0; k < triggers.length(); k++)
        {
            Trigger thisTrigger;
            //start out by setting defaults - should be moved to constructor for class Trigger.
            thisTrigger.bus = -1; //-1 means we don't care which
            thisTrigger.ID = -1; //the rest of these being -1 means nothing has changed it
            thisTrigger.maxCount = -1;
            thisTrigger.milliseconds = -1;
            thisTrigger.currCount = 0;
            thisTrigger.msCounter = 0;
            thisTrigger.readyCount = true;

            QStringList trigToks = triggers[k].split(' ');
            for (int x = 0; x < trigToks.length(); x++)
            {
                QString tok = trigToks.at(x);
                if (tok.left(2) == "ID")
                {
                    thisTrigger.ID = Utils::Base::parseStringToNum(tok.right(tok.length() - 3));
                    if (thisTrigger.maxCount == -1) thisTrigger.maxCount = 10000000;

                    if (thisTrigger.milliseconds == -1) thisTrigger.milliseconds = 0; //by default don't count, just send it upon trigger
                    thisTrigger.readyCount = false; //won't try counting until trigger hits
                }
                else if (tok.endsWith("MS"))
                {
                    thisTrigger.milliseconds = Utils::Base::parseStringToNum(tok.left(tok.length()-2));
                    if (thisTrigger.maxCount == -1) thisTrigger.maxCount = 10000000;
                    if (thisTrigger.ID == -1) thisTrigger.ID = 0;
                }
                else if (tok.endsWith("X"))
                {
                    thisTrigger.maxCount = Utils::Base::parseStringToNum(tok.left(tok.length() - 1));
                    if (thisTrigger.ID == -1) thisTrigger.ID = 0;
                    if (thisTrigger.milliseconds == -1) thisTrigger.milliseconds = 10;
                }
                else if (tok.startsWith("BUS"))
                {
                    thisTrigger.bus = Utils::Base::parseStringToNum(tok.right(tok.length() - 3));
                }
            }
            //now, find anything that wasn't set and set it to defaults
            if (thisTrigger.maxCount == -1) thisTrigger.maxCount = 10000000;
            if (thisTrigger.milliseconds == -1) thisTrigger.milliseconds = 100;
            if (thisTrigger.ID == -1) thisTrigger.ID = 0;
            pSd->triggers.append(thisTrigger);
        }
    }
    else //setup a default single shot trigger
    {
        Trigger thisTrigger;
        thisTrigger.bus = -1;
        thisTrigger.ID = 0;
        thisTrigger.maxCount = 1;
        thisTrigger.milliseconds = 10;
        pSd->triggers.append(thisTrigger);
    }
}

void XFrameSender::parseOperandString(QStringList tokens, ModifierOperand &operand)
{
    qDebug() << "parseOperandString";
    //example string -> bus:0:id:200:d3

    operand.bus = -1;
    operand.ID = -2;
    operand.databyte = 0;

    for (int i = 0; i < tokens.length(); i++)
    {
        if (tokens[i] == "BUS")
        {
            operand.bus = Utils::Base::parseStringToNum(tokens[++i]);
        }
        else if (tokens[i] == "ID")
        {
            operand.ID = Utils::Base::parseStringToNum(tokens[++i]);
        }
        else if (tokens[i].length() == 2 && tokens[i].startsWith("D"))
        {
            operand.databyte = Utils::Base::parseStringToNum(tokens[i].right(tokens[i].length() - 1));
        }
        else
        {
            operand.databyte = Utils::Base::parseStringToNum(tokens[i]);
            operand.ID = 0; //special ID to show this is a number not a look up.
        }
    }
}

ModifierOperationType XFrameSender::parseOperation(QString op)
{
    if (op == "+") return ADDITION;
    if (op == "-") return SUBTRACTION;
    if (op == "*") return MULTIPLICATION;
    if (op == "/") return DIVISION;
    if (op == "&") return AND;
    if (op == "|") return OR;
    if (op == "^") return XOR;
    if (op == "%") return MOD;
    return ADDITION;
}

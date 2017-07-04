#ifndef XFRAMESENDER_H
#define XFRAMESENDER_H

#include "xbusframe.h"

#include <QObject>

//Stores a single trigger.
class Trigger
{
public:
    bool readyCount; //ready to do millisecond ticks?
    int ID; //which ID to match against
    int milliseconds; //interval for triggering
    int msCounter; //how many MS have ticked since last trigger
    int maxCount; //max # of these frames to trigger for
    int currCount; //how many we have triggered for so far.
    int bus; //which bus to monitor (-1 if we aren't picky)
};

//referece for a source location for a single modifier.
//If ID is zero then this is a numeric operand. The number is then
//stored in databyte
//If ID is -1 then this is the temporary storage register. This is a shadow
//register used to accumulate the results of a multi operation modifier.
//if ID is -2 then this is a look up of our own data bytes stored in the class data.
class ModifierOperand
{
public:
    int ID;
    int bus;
    int databyte;
    bool notOper; //should a bitwise NOT be applied to this prior to doing the actual calculation?
};

//list of operations that can be done between the two operands
enum ModifierOperationType
{
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    AND,
    OR,
    XOR,
    MOD
};

//A single modifier operation
class ModifierOp
{
public:
    ModifierOperand first;
    ModifierOperand second;
    ModifierOperationType operation;
};

//All the operations that entail a single modifier. For instance D0+1+D2 is two operations
class Modifier
{
public:
    int destByte;
    QList<ModifierOp> operations;
};

//A single line from the data grid. Inherits from CANFrame so it stores a canbus frame
//plus the extra stuff for the data grid
class FrameSendData : public XBusFrame
{
public:
    bool enabled;
    int count;
    QList<Trigger> triggers;
    QList<Modifier> modifiers;
};

class XFrameSender : public QObject
{
    Q_OBJECT
public:
    explicit XFrameSender(QObject *parent = 0);

signals:

public slots:
    void slotCmdParser(const QString &cmdString);

private:
    FrameSendData *findSendDataById(quint32 id);

    QList<FrameSendData> m_sendingData;
};

#endif // XFRAMESENDER_H

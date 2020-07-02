#include "mbed.h"
#include "PinDetect.h"
#include "flushCount.h"


FlushCounter::FlushCounter(PinName switchA, PinName switchB):
_pdFlushCounterA(new PinDetect (switchA)),
_pdFlushCounterB(new PinDetect (switchB))

{
    _switchA = switchA;
    _switchB = switchB;

    _flushCurrentState = STOP;
    _rotationCount = 0;
    _flushCount = 0;
    _usesCount = 0;

    LastSwitchTime = 0;

    _pdFlushCounterA->mode(PullUp);
    _pdFlushCounterA->setAssertValue(0);
    _pdFlushCounterA->attach_asserted(this, &FlushCounter::_FlushCounterA_Pressed);
    _pdFlushCounterA->attach_deasserted(this, &FlushCounter::_FlushCounterA_Released);

    _pdFlushCounterB->mode(PullUp);
    _pdFlushCounterB->setAssertValue(0);
    _pdFlushCounterB->attach_asserted(this, &FlushCounter::_FlushCounterB_Pressed);
    _pdFlushCounterB->attach_deasserted(this, &FlushCounter::_FlushCounterB_Released);


    _pdFlushCounterA->setSampleFrequency(20000);
    _pdFlushCounterB->setSampleFrequency(20000);



}

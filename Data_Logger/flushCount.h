#include "mbed.h"
#include "PinDetect.h"
#include "QEI.h"
#include "eeprom.h"

typedef enum
{
    RUN,
    STOP
}flushState_t;

extern Timer topTimer;
extern Timer buttomTimer;

class FlushCounter
{

    public:

    FlushCounter(PinName switchA, PinName switchB);

    uint32_t getCount(void);

    float getFlushCount(void);
    float getUsesCount(void);

    float getServiceCount(void);

    uint8_t dailyUses(void);

    void writeEEPROMData(void);
    void clearEEPROMData(void);
    void readEEPROMData(void);

    void averageSpeed(void);
    void returnSpeed(void);

    uint32_t LastSwitchTime;


    private:

    int32_t _rotationCount;
    int32_t _previousRotationCount;

    int32_t _flushCount;

    int32_t _usesCount;

    PinName _switchA;
    PinName _switchB;

    PinDetect* _pdFlushCounterA;
    PinDetect* _pdFlushCounterB;


    flushState_t _flushCurrentState;

    void _FlushCounterA_Pressed (void);
    void _FlushCounterA_Released (void);

    void _FlushCounterB_Pressed(void);
    void _FlushCounterB_Released(void);

};
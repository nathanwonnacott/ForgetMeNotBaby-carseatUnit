
#include "CarseatUnitStateMachine.h"

//Initialize static variables

CarseatUnitStateMachine* CarseatUnitStateMachine::singleton = NULL;
volatile unsigned char CarseatUnitStateMachine::interruptFlags = 0;

//Class methods

CarseatUnitStateMachine::CarseatUnitStateMachine()
{
  timerController = TimerOneMulti::getTimerController();
  state = INACTIVE;
  seatUpWaitTimer = NULL;
  pulseTimer = NULL;
  serialPort = new SoftwareSerial(RX, TX);
  serialPort->begin(9600);
}

void CarseatUnitStateMachine::receiveMessage(char* message, int count)
{
  
  if ( count == strlen("FMNB:Reconnect\n") && strncmp(message,"FMNB:Reconnect\n",count) )
  {
    receivedReconnect();
  }
}

void CarseatUnitStateMachine::receivedReconnect()
{
  if( state == INACTIVE)
  {
    serialPort->println("FMNB:SeatUp");
  }
  if( state == ACTIVE || state == SEAT_UP_WAIT)
  {
    serialPort->println("FMNB:SeatDown");
  }
}

SoftwareSerial* CarseatUnitStateMachine::getSerialPort()
{
  return serialPort;
}

CarseatUnitStateMachine* CarseatUnitStateMachine::getStateMachine()
{
  if(singleton == NULL)
    singleton = new CarseatUnitStateMachine();
  
  return singleton;
}

void CarseatUnitStateMachine::seatDown()
{
  if(state == SEAT_UP_WAIT)
  {
    if(seatUpWaitTimer != NULL)
    {
      timerController->cancelEvent(seatUpWaitTimer);
      seatUpWaitTimer = NULL;
    }
  }
  
  digitalWrite(LED1,HIGH);
  state = ACTIVE;
  
  //Start heartbeat pulses if  not already started
  if ( pulseTimer == NULL)
  {
    pulseTimer = timerController->addEvent(HEARTBEAT_PERIOD, timerISR, true, (void*) TIMER_FLAG_HEARTBEAT);
  }
  
}
void CarseatUnitStateMachine::seatUp()
{
  state = SEAT_UP_WAIT;
  if ( seatUpWaitTimer != NULL)
    timerController->cancelEvent(seatUpWaitTimer);
  seatUpWaitTimer = timerController->addEvent(SEATUP_WAIT_TIMEOUT, timerISR, false, (void*) TIMER_FLAG_SEAT_UP_WAIT);
}

void CarseatUnitStateMachine::seatUpWaitTimerExpired()
{
  state = INACTIVE;
  digitalWrite(LED1,LOW);

  timerController->cancelEvent(pulseTimer);
  pulseTimer = NULL;
  
  serialPort->println("FMNB:SeatUp");
}

void CarseatUnitStateMachine::heartbeatPulse()
{
  serialPort->println("FMNB:SeatDown");
}

void CarseatUnitStateMachine::seatStatusChange(int val)
{
  if(val)
  {
    seatDown();
  }
  else
  {
    seatUp();
  }
}

//ISRs
void timerISR(void* timerType)
{
  CarseatUnitStateMachine::interruptFlags |= (unsigned char) (int) timerType;
}
void seatSensorChangeISR()
{
  CarseatUnitStateMachine::interruptFlags |= SEAT_SENSOR_CHANGE_ISR_FLAG;
}


#include "Arduino.h"
#include "TimerOneMulti.h"
#include <SoftwareSerial.h>

/*
 This class represents a state machine for the Pressure sensing unit in the carseat.
 This is the main controller for this program.
*/

#ifndef CARSEAT_UNIT_STATE_MACHINE_H
  #define CARSEAT_UNIT_STATE_MACHINE_H
 
  
#define ID 0x1234
#define MESSAGE_BUFFER_SIZE 100

//ISR flag values
#define SEAT_SENSOR_CHANGE_ISR_FLAG  0b10000000
#define TIMER_FLAG_HEARTBEAT         0b01000000
#define TIMER_FLAG_SEAT_UP_WAIT      0b00100000

#define SEAT_SENSOR_IRQ  0  //use 0 for Uno, I believe we need 1 for Nano

//PIN definitions
#define SEAT_SENSOR  2
#define LED1         13  //Not sure if we need an LED in production, but it helps for debugging
#define LED2         12
#define TX           5
#define RX           6

//Timeout values
#define SEATUP_WAIT_TIMEOUT 1000000
#define HEARTBEAT_PERIOD    1000000//MAX_PERIOD
enum CarseatState
{
  INACTIVE,
  ACTIVE,
  SEAT_UP_WAIT
};

class CarseatUnitStateMachine
{
public:
  //Flags indicating which interrupts have occurred. Should not be accessed with interrupts enabled in case later I decide to make it
  //a datatype that requires multiple instructions to read
  //It really doesn't belong inside the class, but for some reason I keep getting multiple definition errors when I bring it outside
  volatile static unsigned char interruptFlags;
  
private:
  static CarseatUnitStateMachine* singleton;
  TimerOneMulti* timerController;
  SoftwareSerial* serialPort;
  
  void seatDown();
  void seatUp();
  CarseatState state;
  TimerEvent* seatUpWaitTimer;
  TimerEvent* pulseTimer;
  
public:
  static CarseatUnitStateMachine* getStateMachine();
  SoftwareSerial* getSerialPort();
  
  void receiveMessage(char* message, int count);
  
  void receivedReconnect();
  
  void seatStatusChange(int val);
  
  void seatUpWaitTimerExpired();
  
  void heartbeatPulse();

private:  
  CarseatUnitStateMachine();
  
};

//ISRs
void timerISR(void* timerType);  //This is called by TimerOne library, so it can take an argument
void seatSensorChangeISR();


#endif

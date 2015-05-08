#include <TimerOne.h>
#include "TimerOneMulti.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <SoftwareSerial.h>


#include "CarseatUnitStateMachine.h"

CarseatUnitStateMachine* stateMachine;
TimerOneMulti* timerController;

char serialReceiveBuffer[MESSAGE_BUFFER_SIZE];

void sleep()
{
  /* Disable all of the unused peripherals. This will reduce power
   * consumption further and, more importantly, some of these
   * peripherals may generate interrupts that will wake our Arduino from
   * sleep!
   * Don't dissable spi because we want that to wake us up
   */
  power_adc_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();  

  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the timer timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}

void setup() {
  
  //Set up pins
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(SEAT_SENSOR, INPUT);
  pinMode(TX,OUTPUT);
  
  digitalWrite(TX,LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  
  
  //Set up serial
  
  Serial.begin(9600);
  
  //Set up objects
  timerController = TimerOneMulti::getTimerController();
  stateMachine = CarseatUnitStateMachine::getStateMachine();
  
  //Set up interrupts
  attachInterrupt(SEAT_SENSOR_IRQ, seatSensorChangeISR, CHANGE);
  
  //Serial.println("Adding timer events");
  /*timerController->addEvent(7000000,setLED,false, (void*) 0);
  timerController->addEvent(4000000,setLED,false, (void*) 1);
  timerController->addEvent(3000000,setLED,false, (void*) 0);
  timerController->addEvent(2000000,setLED,false, (void*) 1);
  timerController->addEvent(5000000,setLED,false, (void*) 0);
  timerController->addEvent(6000000,setLED,false, (void*) 1);
  timerController->addEvent(8000000,setLED,false, (void*) 1);
  timerController->addEvent(8500000,setLED,false, (void*) 0);
  timerController->addEvent(2100000,setLED,false, (void*) 0);
  timerController->addEvent(2200000,setLED,false, (void*) 1);
  timerController->addEvent(28500000,setLED,false, (void*) 1);
  //timerController->addEvent(10000000,myToggleLight,true, (void*) NULL);*/
  
  //Enable sleeping
  //We can't go any deeper than IDLE, otherwise Timer1 won't wake us up
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  
}

void loop() 
{
  sleep();
  unsigned char interruptFlagsCopy = 0;
  
  noInterrupts();
  interruptFlagsCopy = CarseatUnitStateMachine::interruptFlags;
  CarseatUnitStateMachine::interruptFlags = 0;
  interrupts();
  
  if ( interruptFlagsCopy & SEAT_SENSOR_CHANGE_ISR_FLAG)
  {
    stateMachine->seatStatusChange(digitalRead(SEAT_SENSOR));
  }
  if ( interruptFlagsCopy & TIMER_FLAG_SEAT_UP_WAIT)
  {
    stateMachine->seatUpWaitTimerExpired();
  }
  if (interruptFlagsCopy & TIMER_FLAG_HEARTBEAT)
  {
    stateMachine->heartbeatPulse();
  }
  while(stateMachine->getSerialPort()->available())
  {
    byte bytesRead = stateMachine->getSerialPort()->readBytesUntil('\n',serialReceiveBuffer,MESSAGE_BUFFER_SIZE);
    stateMachine->receiveMessage(serialReceiveBuffer,bytesRead);
  }
  
  digitalWrite(LED2,! digitalRead(LED2));
  
}

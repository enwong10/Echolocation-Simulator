/**********************************************************************
  DUAL SENSOR ECHO LOCATOR
 dual_sensor_echo_locator.ino
  Copyright (C) by lingib
  https://www.instructables.com/member/lingib/instructables/
  Last update 28 June 2018

  This code uses the task scheduler described in
  https://www.instructables.com/id/Multi-task-Your-Arduino/
  to control the transmitter pulse rate.

  Place two ultrasonic transducers on a fixed base-line. The baseline is
  defined in the corresponding Processing 3 sketch.
  
  Both sensors are triggered at the same time. Sensor1 hears its own
  echo. Sensor2 thinks it has sent a pulse but actually receives the
  pulse from Sensor1. The distances are then sent to the display for
  triangulation.

  This dual sensor circuit requires that the #2 transmit transducer is
  muffled/disabled.
  
  ----------
  COPYRIGHT
  ----------
  This code is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License. If
  not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

// ===============================
// globals
// ===============================

// ----- arduino pinouts
#define Trig1 4                       //sensor A "Trig" pin
#define Echo1 5                       //sensor A "Echo" pin

#define Trig2 6                       //sensor B "Trig" pin          //IS THIS NECESSARY????
#define Echo2 7                       //sensor B "Echo" pin

#define Trig3 8                       //sensor C "Trig" pin
#define Echo3 9                       //sensor C "Echo" pin

#define Trig4 10                      //sensor D "Trig" pin          //IS THIS NECESSARY????
#define Echo4 11                      //sensor D "Echo" pin

// ----- results
float Baseline = 100;                  //distance between the transducers (cm)
float Distance1;                      //from active sender (cm)
float Distance2;                      //from passive receiver (cm)
float Distance3;
float Distance4;

  boolean echo_flag1;                 //flags reflect state of echo line
  boolean echo_flag2;
  boolean echo_flag3;
  boolean echo_flag4;
  unsigned long start_time;

// ----- task scheduler
int TaskTimer1 = 0;                   //task 1 (see ISR(TIMER2_COMPA_vect)
bool TaskFlag1 = false;               //flag 1
int state = 0;


// ===============================
// setup
// ===============================
void setup() {

  // ----- configure serial port
  Serial.begin(9600);

  // ----- configure arduino pinouts
  pinMode(Echo1, INPUT);              //make echo pins inputs
  pinMode(Echo2, INPUT);
  pinMode(Trig1, OUTPUT);             //make trig pins OUTPUT
  pinMode(Trig2, OUTPUT);
  digitalWrite(Trig1, LOW);           //set trig pins LOW
  digitalWrite(Trig2, LOW);

  // ----- configure Timer 2 to generate a compare-match interrupt every 1mS
  noInterrupts();                     //disable interrupts
  TCCR2A = 0;                         //clear control registers
  TCCR2B = 0;
  TCCR2B |= (1 << CS22) |             //16MHz/128=8uS
            (1 << CS20) ;
  TCNT2 = 0;                          //clear counter
  OCR2A = 125 - 1;                    //8uS*125=1mS (allow for clock propagation)
  TIMSK2 |= (1 << OCIE2A);            //enable output compare interrupt
  attachInterrupt(0, interrupt_handler, RISING);
  interrupts();                       //enable interrupts
}

// ===============================
// loop()
// ===============================
void loop()
{
  // ----- measure object distances
  if (TaskFlag1)
  {
    TaskFlag1 = false;
    measure();

    while (!(echo_flag1 && echo_flag2));
    
    Serial.print(Distance1); Serial.print(","); Serial.println(Distance2 - Distance1); Serial.print(","); Serial.println(Distance3); Serial.print(","); Serial.println(Distance4 - Distance3);
  }
}

// ===============================
// task scheduler (1mS interrupt)
// ===============================
ISR(TIMER2_COMPA_vect)
{
  // ----- timers
  TaskTimer1++;                       //task 1 timer

  // ----- task1
  if (TaskTimer1 > 5000 && state == 0)               //interval between pings (50mS=423cm)
  {
    Serial.print("STARTED");
    TaskTimer1 = 0;                   //reset timer
    TaskFlag1 = true;                 //signal loop() to perform task'
    state++;
  }
}

// ===============================
// measure distances
// ===============================
void measure()
{
  // ----- locals
             //microseconds
  unsigned long finish_time1;         //microseconds
  unsigned long finish_time2;         //microseconds
  unsigned long finish_time3;
  unsigned long finish_time4;
  unsigned long time_taken;           //microseconds


  // ----- record start time
  start_time = micros();

  // ----- reset the flags
  echo_flag1 = false;
  echo_flag2 = false;
  echo_flag3 = false;
  echo_flag4 = false; 
}

float calculate1()
{
      float finish_time = micros();
      float time_taken = finish_time - start_time;
      return ((float)time_taken) / 59;
}

float calculate2()
{
      float finish_time = micros();
      float time_taken = finish_time - start_time;
      return ((float)time_taken) / 29.5;
}

void interrupt_handler()
{
  if (digitalRead(Echo1))
  {
    Serial.println ("Interrupt Triggered");
    Distance1 = calculate1();
    echo_flag1 = true;
  }
  else if (digitalRead(Echo2))
  {
    Distance2 = calculate2();
    echo_flag2 = true;
  }
  else if (digitalRead(Echo3))
  {
    Distance3 = calculate1();
    echo_flag3 = true;
  }
  else if (digitalRead(Echo4))
  {
    Distance4 = calculate2();
    echo_flag4 = true; 
  }
}


// ===============================
// GLOBAL VARIABLES
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
float Baseline = 100;                 //distance between the transducers (cm)
float Distance1;                      //from active sender (cm)
float Distance2;                      //from passive receiver (cm)
float Distance3;                      //from sender2 (cm)
float Distance4;                      //from passive2 (cm)
boolean echo_flag1;                 //flags reflect state of echo line
boolean echo_flag2;
boolean echo_flag3;
boolean echo_flag4;
unsigned long start_time;           //microseconds

// ----- task scheduler
int TaskTimer1 = 0;                   //task 1 (see ISR(TIMER2_COMPA_vect)
bool TaskFlag1 = false;               //flag 1
int state = 0;
int cycle;

// ===============================
// SETUP
// ===============================
void setup() {
  // ----- configure serial port
//  Serial.begin(115200);
  Serial.begin(9600);

  // ----- configure arduino pinouts
  pinMode(Echo1, INPUT);              //make echo pins inputs
  pinMode(Echo2, INPUT);
  pinMode(Echo3, INPUT);
  pinMode(Echo4, INPUT);
  pinMode(Trig1, OUTPUT);             //make trig pins OUTPUT
  pinMode(Trig2, OUTPUT);             //IS THIS NECESSARY????
  pinMode(Trig3, OUTPUT);
  pinMode(Trig4, OUTPUT);             //IS THIS NECESSARY????
  digitalWrite(Trig1, LOW);           //set trig pins LOW
  digitalWrite(Trig2, LOW);           //IS THIS NECESSARY????
  digitalWrite(Trig3, LOW);
  digitalWrite(Trig4, LOW);           //IS THIS NECESSARY????
  attachInterrupt(1, state_change, CHANGE);
  attachInterrupt(0, echo_recieved, FALLING);

  // ----- configure Timer 2 to generate a compare-match interrupt every 1mS ***WHICH TIMER IS THE MOST ACCURATE? 
  noInterrupts();                     //disable interrupts
  TCCR2A = 0;                         //clear control registers
  TCCR2B = 0;
  TCCR2B |= (1 << CS22) |             //16MHz/128=8uS
            (1 << CS20) ;
  TCNT2 = 0;                          //clear counter
  OCR2A = 125;                        //8uS*125=1mS (allow for clock propagation)
  TIMSK2 |= (1 << OCIE2A);            //enable output compare interrupt
  interrupts();                       //enable interrupts
} 

// ===============================
// LOOP()
// ===============================
void loop()
{
  // ----- measure object distances
  if (TaskFlag1)
  {
    TaskFlag1 = false;
    measure();

    // -----Distance readings to the display
    Serial.print(Distance1/2); Serial.print(","); Serial.print(Distance2 - Distance1); Serial.print(","); Serial.print(Distance3/2); Serial.print(","); Serial.print(Distance4 - Distance3); Serial.print(","); Serial.println(state);
  }
}

// ===============================
// INTERRUPT
// ===============================
ISR(TIMER2_COMPA_vect)
{
  // ----- timers
  TaskTimer1++;                       //task 1 timer

  // ----- task1
  if (TaskTimer1 > 1000 && state == 0)               //interval between pings (50mS=423cm)
  {
    TaskTimer1 = 0;                   //reset timer
    TaskFlag1 = true;                 //signal loop() to perform task
  }
}

// ===============================
// MEASURE
// ===============================
void measure()
{
 // ----- send 10uS trigger pulse
  digitalWrite(Trig1, HIGH);
  digitalWrite(Trig2, HIGH);        
  delayMicroseconds(10);
  digitalWrite(Trig1, LOW);
  digitalWrite(Trig2, LOW);
  cycle = 0;         

  // ----- wait for both echo lines to go high
  while (!digitalRead(Echo1));
  while (!digitalRead(Echo2));

  // ----- reset Timer0
//  TCNT0 = 0;
  start_time = micros();

  // ----- reset the flags
  echo_flag1 = false;
  echo_flag2 = false;

  // ----- measure echo times //SHOULD THIS BE INTERRUPT BASED? CONSIDER DISABLING THEM TEMPORARILY?
  while ((!echo_flag1) || (!echo_flag2));

  //REPEAT FOR SECOND SET
  digitalWrite(Trig3, HIGH);
  digitalWrite(Trig4, HIGH);        //IS THIS NECESSARY????
  delayMicroseconds(10);
  digitalWrite(Trig3, LOW);
  digitalWrite(Trig4, LOW);         //IS THIS NECESSARY????
  cycle = 1;

  // ----- wait for both echo lines to go high
  while (!digitalRead(Echo3));
  while (!digitalRead(Echo4));

// ----- reset Timer2
//  TCNT0 = 0;
  start_time = micros();

  // ----- reset the flags
  echo_flag3 = false;
  echo_flag4 = false;

  // ----- measure echo times
  while ( (!echo_flag3) || (!echo_flag4));
}

float getTime()
{
  int clockValue = TCNT0;
  return (float)clockValue*0.000004;
}

void state_change()
{
  if (state == 0)
    state = 1;
  else
    state = 0;
}

void echo_recieved()
{
  if (!digitalRead(Echo1) && cycle == 0)
  {
      echo_flag1 = false;
      Distance1 = calculate(); 
  }
  else if (!digitalRead(Echo2) && cycle == 0)
  {
      echo_flag2 = false;
      Distance2 = calculate(); 
  } 
  else if (!digitalRead(Echo3) && cycle == 1)
  {
      echo_flag3 = false;
      Distance3 = calculate(); 
  }
  else if (!digitalRead(Echo4) && cycle == 1)
  {
      echo_flag4 = false;
      Distance4 = calculate(); 
  } 
}

float calculate()
{
      unsigned long finish_time = micros();
      unsigned long time_taken = finish_time - start_time;
      return ((float)time_taken) / 29.5;
}


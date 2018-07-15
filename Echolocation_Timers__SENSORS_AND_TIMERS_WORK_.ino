// ===============================
// GLOBAL VARIABLES
// ===============================

// ----- arduino pinouts
#define Trig1 4                       //sensor A "Trig" pin
#define Echo1 5                       //sensor A "Echo" pin

#define Trig2 6                       //sensor B "Trig" pin          
#define Echo2 7                       //sensor B "Echo" pin

#define Trig3 8                       //sensor C "Trig" pin
#define Echo3 9                       //sensor C "Echo" pin

#define Trig4 10                      //sensor D "Trig" pin          
#define Echo4 11                      //sensor D "Echo" pin

// ----- results
float Baseline = 100;                 //distance between the transducers (cm)
volatile float Distance1;                      //from active sender (cm)
volatile float Distance2;                      //from passive receiver (cm)
volatile float Distance3;                      //from sender2 (cm)
volatile float Distance4;                      //from passive2 (cm)
volatile boolean echo_flag;                 //flags reflect state of echo line
unsigned long volatile start_time;           //milliseconds
unsigned long volatile ovf_count = 0;

// ----- task scheduler
int TaskTimer1 = 0;                   //task 1 (see ISR(TIMER2_COMPA_vect)
bool TaskFlag1 = false;               //flag 1
volatile int state = 0;
volatile int sensor;

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
  pinMode(Trig2, OUTPUT);             
  pinMode(Trig3, OUTPUT);
  pinMode(Trig4, OUTPUT);             
  digitalWrite(Trig1, LOW);           //set trig pins LOW
  digitalWrite(Trig2, LOW);           
  digitalWrite(Trig3, LOW);
  digitalWrite(Trig4, LOW);           
  attachInterrupt(1, state_change, CHANGE);
  attachInterrupt(0, echo_recieved, FALLING);

  // ----- configure Timer 2 to generate a compare-match interrupt every 1mS 
  noInterrupts();                     //disable interrupts
  TCCR2A = 0;                         //clear control registers
  TCCR2B = 0;
  TCCR2B |= (1 << CS22) |             //16MHz/128=8uS
            (1 << CS20) ;
  TCNT2 = 0;                          //clear counter
  OCR2A = 125 - 1;                        //8uS*125=1mS (allow for clock propagation)
  TIMSK2 |= (1 << OCIE2A);            //enable output compare interrupt

    TCCR0B = 0b11;  //Timer settings for interrupt at every millisecond
    OCR0A = 256 - 1;
    TIMSK0 |= (1 << OCIE0A);            //enable output compare interrupt
  
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
  if (TaskTimer1 > 2000 && state == 0)               //interval between pings (50mS=423cm)
  {
    TaskTimer1 = 0;                   //reset timer
    TaskFlag1 = true;                 //signal loop() to perform task
//    Serial.println("Start");
  }
}

  ISR(TIMER0_COMPA_vect) {        //Timer interrupt ISR
    ovf_count++;
  }

// ===============================
// MEASURE
// ===============================
void measure()
{
  //DISTANCE1
  sensor = 1;
 // ----- send 10uS trigger pulse
  digitalWrite(Trig1, HIGH);        
  delayMicroseconds(10);
  digitalWrite(Trig1, LOW);    

//  Serial.println("Pulse1");

  // ----- wait for both echo lines to go high
  while (!digitalRead(Echo1)){}

//  Serial.println("Test");
  
  // ----- reset Timer0
//  TCNT0 = 0;
  start_time = getTime();

  // ----- reset the flags
  echo_flag = false;

  while (!echo_flag){}

  //DISTANCE2
    sensor = 2;
 // ----- send 10uS trigger pulse
  digitalWrite(Trig2, HIGH);        
  delayMicroseconds(10);
  digitalWrite(Trig2, LOW);        

//  Serial.println("Pulse2");

  // ----- wait for both echo lines to go high
  while (!digitalRead(Echo2));

  // ----- reset Timer0
//  TCNT0 = 0;
  start_time = getTime();

  // ----- reset the flags
  echo_flag = false;

  while (!echo_flag){}

//  //DISTANCE3
  sensor = 3;
 // ----- send 10uS trigger pulse
  digitalWrite(Trig3, HIGH);        
  delayMicroseconds(10);
  digitalWrite(Trig3, LOW);        

//Serial.println("Pulse3");
  // ----- wait for both echo lines to go high
  while (!digitalRead(Echo3));

  // ----- reset Timer0
//  TCNT0 = 0;
  start_time = micros();

  // ----- reset the flags
  echo_flag = false;

  while (!echo_flag){}

    //DISTANCE4
  sensor = 4;
 // ----- send 10uS trigger pulse
  digitalWrite(Trig4, HIGH);        
  delayMicroseconds(10);
  digitalWrite(Trig4, LOW);        

//Serial.println("Pulse4");
  // ----- wait for both echo lines to go high
  while (!digitalRead(Echo4));

  // ----- reset Timer0
//  TCNT0 = 0;
  start_time = micros();

  // ----- reset the flags
  echo_flag = false;

  while (!echo_flag){}
    
  Serial.print(Distance1); Serial.print(","); Serial.print(Distance2); Serial.print(","); Serial.print(Distance3); Serial.print(","); Serial.print(Distance4); Serial.print(","); Serial.println(state);
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
//Serial.println("Interrupt");
float distance = calculate();
echo_flag = true;

switch (sensor)
{
  case 1: 
//  Serial.println("Interrupt1");
  Distance1 = distance;
  break;
  case 2:
//  Serial.println("Interrupt2");
  Distance2 = distance;
  break;
  case 3:
//  Serial.println("Interrupt3");
  Distance3 = distance;
  break; 
  case 4:
//  Serial.println("Interrupt4");
  Distance4 = distance;
  break;
}
}

float calculate()
{
      float finish_time = getTime();
      float time_taken = finish_time - start_time;
      Serial.println(start_time);
      Serial.println(finish_time);
      return (((float)time_taken) * 0.0343)/2;
}

float getTime()
{
  return ((ovf_count * 1024) + (TCNT0 * 4));
}


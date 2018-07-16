// ===============================
// GLOBAL VARIABLES
// ===============================

// ----- sensors to Arduino pins
#define Trig1 4                       //sensor A "Trig" pin
#define Trig2 6                       //sensor B "Trig" pin          
#define Trig3 8                       //sensor C "Trig" pin
#define Trig4 10                      //sensor D "Trig" pin          

// ----- variables
volatile float Distance1;                      //calculated distance of sensor 1 (cm)
volatile float Distance2;                      //calculated distance of sensor 2 (cm)
volatile float Distance3;                      //calculated distance of sensor 3 (cm)
volatile float Distance4;                      //calculated distance of sensor 4 (cm)
volatile boolean echo_flag;                    //ensures interrupt is triggered before starting next sensor
volatile unsigned long start_time;             //takes start time of sensor (milliseconds)
volatile int state = 0;                        //tells the processing code which display to show
volatile int sensor;                           //stores the number of the sensor currently being used

// ----- task scheduler
int TaskTimer = 0;                             //sets the timer counter to 0
bool TaskFlag = false;                         //flag for the main loop
volatile unsigned long ovf_count = 0;          //counts the amount of times Timer 0 overflows (to calculate micros)

// ===============================
// SETUP
// ===============================
void setup() {
  // ----- configure serial port (communication between code and processing or code and screen (when debugging)
//  Serial.begin(115200);
  Serial.begin(9600);

  // ----- configure arduino pin
  pinMode(Echo1, INPUT);              //make echo pins inputs
  pinMode(Echo2, INPUT);
  pinMode(Echo3, INPUT);
  pinMode(Echo4, INPUT);
  pinMode(Trig1, OUTPUT);             //make trig pins outputs
  pinMode(Trig2, OUTPUT);             
  pinMode(Trig3, OUTPUT);
  pinMode(Trig4, OUTPUT);             
  digitalWrite(Trig1, LOW);           //set trig pins LOW
  digitalWrite(Trig2, LOW);           
  digitalWrite(Trig3, LOW);
  digitalWrite(Trig4, LOW); 
            
  // ----- set interrupt pins
  attachInterrupt(0, echo_received, FALLING);         //set to trigger when an echo is received
  attachInterrupt(1, state_change, CHANGE);           //set to change between menu and display
  
  // ----- configure timers 
  noInterrupts();                     //disable interrupts
  TCCR2A = 0;                         //clear control registers
  TCCR2B = 0;
  TCCR2B |= (1 << CS22) |             //select clock 5  
            (1 << CS20) ;
  TCNT2 = 0;                          //reset counter
  OCR2A = 124;                        //interrupt triggers about every 1ms
  TIMSK2 |= (1 << OCIE2A);            //enable output compare interrupt

  TCCR0B = 0b11;                      //select clock 3
  OCR0A = 255;                        //interrupt triggers at every overflow
  TIMSK0 |= (1 << OCIE0A);            //enable output compare interrupt
  interrupts();                       //enable interrupts
} 

// ===============================
// LOOP
// ===============================
void loop()
{
  // ----- when flag is raised, measure distances
  if (TaskFlag)
  {
    TaskFlag = false;         //reset flag to false
    measure();                //call measure() method
  }
}

// ===============================
// INTERRUPTS
// ===============================
ISR(TIMER0_COMPA_vect)         //used to keep track of timer overflow to calculate micros
{        
  ovf_count++;                 //counter 
}

ISR(TIMER2_COMPA_vect)         //interrupts every millisecond
{
  TaskTimer++;                             //add to count

  // ----- loop runs every 
  if (TaskTimer > 2000 && state == 0)      //runs every 2 seconds, and only when displayed (state == 0)
  {
    TaskTimer = 0;                         //reset timer
    TaskFlag = true;                       //signal main loop to execute task
//    Serial.println("Start");             //debugging 
  }
}

// ===============================
// MEASURE
// ===============================
void measure()                //sends and receives signals from sensors
{
  //DISTANCE1
  sensor = 1;                        //getting the first distance with the first sensor

  digitalWrite(Trig1, HIGH);         //send 10uS trigger pulse
  delayMicroseconds(10);             //delay 
  digitalWrite(Trig1, LOW);          //stops pulse

//  Serial.println("Pulse1");        //debug

  while (!digitalRead(Echo1));       //wait for echo pin to go HIGH *supposed to go HIGH when signal is sent, LOW when signal is received*
  start_time = getTime();            //get the start time 
  echo_flag = false;                 //reset the flag

  while (!echo_flag){}               //wait for interrupt to be triggered (flag gets set to TRUE)

  //REPEAT FOR DISTANCE2
  sensor = 2;                        
  
  digitalWrite(Trig2, HIGH);        
  delayMicroseconds(10);
  digitalWrite(Trig2, LOW);        

//  Serial.println("Pulse2");

  while (!digitalRead(Echo2));
  start_time = getTime();
  echo_flag = false;

  while (!echo_flag){}

  //REPEAT FOR DISTANCE3
  sensor = 3;

  digitalWrite(Trig3, HIGH);        
  delayMicroseconds(10);
  digitalWrite(Trig3, LOW);        

//Serial.println("Pulse3");

  while (!digitalRead(Echo3));
  start_time = micros();
  echo_flag = false;

  while (!echo_flag){}

  //REPEAT FOR DISTANCE4
  sensor = 4;

  digitalWrite(Trig4, HIGH);        
  delayMicroseconds(10);
  digitalWrite(Trig4, LOW);        

//Serial.println("Pulse4");

  while (!digitalRead(Echo4));
  start_time = micros();
  echo_flag = false;

  while (!echo_flag){}

  // ----- AFTER ALL DISTANCES HAVE BEEN CALCULATED, OUTPUT TO SCREEN/DISPLAY
  Serial.print(Distance1); Serial.print(","); Serial.print(Distance2); Serial.print(","); Serial.print(Distance3); Serial.print(","); Serial.print(Distance4); Serial.print(","); Serial.println(state);
}

// ===============================
// INTERRUPT METHODS
// ===============================
void state_change()             //method called by interrupt to change state of display
{
  //flips the value of the state
  if (state == 0)
    state = 1;
  else
    state = 0;
}

void echo_received()                  //method called by interrupt when echo is received
{
  //Serial.println("Interrupt");      //debug
  float distance = calculate();       //calculates the distance, stored in a temporary variable
  echo_flag = true;                   //raise flag that echo has been received

  switch (sensor)                     //determine which sensor was triggered (only the sensor that emits frequency will recieve an echo)
  {
    case 1: 
//  Serial.println("Interrupt1");
    Distance1 = distance;             //store distance for sensor 1
    break;
    case 2:
//  Serial.println("Interrupt2");
    Distance2 = distance;             //store distance for sensor 2
    break;
    case 3:
//  Serial.println("Interrupt3");
    Distance3 = distance;             //store distance for sensor 3
    break;    
    case 4:
//  Serial.println("Interrupt4");
    Distance4 = distance;             //store distance for sensor 4
    break;
  }
}

// ===============================
// CALCULATE
// ===============================
float calculate()               //called in the echo interrupt method to calculate distance (cm)
{
      float finish_time = getTime();                  //gets the current time (when echo is received)
      float time_taken = finish_time - start_time;    //calculate the time elapsed
//      Serial.println(start_time);                   //debug
//      Serial.println(finish_time);
      return (((float)time_taken) * 0.0343)/2;        //calculate the distance (d = v * t) ---> v = 343 m/s = 0.0343 cm/microsecond ---> t/2 to account for time to object and time back
}

// ===============================
// GETTIME
// ===============================
float getTime()                 //returns number of microseconds
{
  return ((ovf_count * 1024) + (TCNT0 * 4));        //calculates the number of times overflowed * 1024 (since 1 overflow is 1024 microseconds) + current clock cycles * 4 (since 1 clock cycle is 4 microseconds)
}


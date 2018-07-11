// ===============================
// GLOBAL VARIABLES
// ===============================
import processing.serial.*;                      //import the serial library
Serial myPort;                                   //the Serial port object
final int Baud_rate = 115200;                    //communication speed
String Input_string;                             //used for incoming data 
float Baseline = 100;                            //triangle baseline (cm)
float X;                                         //X coordinate in (cm)
float Y;                                         //Y coordinate in (cm)
float Z;                                         //Z coordinate in (cm)
float Offset = 50;                               //assumes square display
float angle = sqrt(2)/2;
int state = 0;

// -----OBJECT
PShape Object;                                   //give the shape a name
int Frame_count = 0;                             //frame counter
boolean Frame_visible = true;                    //true=visible; false=invisible

// ----- DISPLAY
PGraphics Canvas;                                //name of drawing area to be created
PFont myFont;                                    //name of font to be created

// =========================
// SETUP
// ==========================
void setup() {

  // ----- configure screen 
  size(800, 800, P3D);                          //define window size, 3D   
  background(0);                                 //black
  frameRate(60);                                 //60 frames per second

  // ----- create a drawing area for fading the beam
  Canvas = createGraphics(width, height);                          

  // ------ create the screen font
  myFont = createFont("Arial Black", 20);

  // ----- configure the Object
  Object = createShape(ELLIPSE, 0, 0, 15, 15);   //create the Object
  Object.setFill(color(255, 0, 0, 255));         //red, opaque
  Object.setStroke(color(255, 0, 0, 255));       //red, opaque

  // ----- initialize the serial port
  printArray(Serial.list());                     //lists your COM ports on screen
  myPort = new Serial(this, Serial.list()[0], Baud_rate);         //check for new input
  myPort.bufferUntil('\n');
}

// ==========================
// DRAW
// ==========================
void draw()
{
  // ----- refresh the screen
  background(0);                      //black background
  textFont(myFont, 20);               //specify font to be used
  if (state == 0)
  {
    draw_grid();                        //draw grid
    if(X <= 100 && Y <= 100 && Z <= 100)
      draw_object();
  }
  else if (state == 1)
  {
    draw_menu();
  }
}

// =======================
// SERIAL EVENT  (called with each Arduino data string when available)
// =======================
void serialEvent(Serial myPort)
{
  // ----- wait for a line-feed
  Input_string = myPort.readStringUntil('\n');
  println(Input_string);                              //visual feedback

  // ----- validate
  if (Input_string != null) 
  {
    // ----- trim whitespace
    Input_string = trim(Input_string);
    String[] inputs = split(Input_string, ',');

    //// ----- gather Heron variables
    float a = float(inputs[0]);             //d1    
    float b = float(inputs[1]);             //d2                     
    float c = float(inputs[2]);             //d3
    float d = float(inputs[3]);             //d4   
    state = int(inputs[4]);
    float maxDist = sqrt(sqrt(pow(100,2) + pow(150,2)) + pow(100,2));
    float s1 = (a + b + Baseline) / 2;
    float s2 = (c + d + Baseline) / 2;

    // ----- validate distances
    /* eliminate bogus errors */
    boolean distances_valid = true; //<>//
    if 
      (
      (a < 0) ||          //d1 must be less than d2
      (a > maxDist) ||     //d1 out-of-range 
      (b > maxDist) ||     //d2 out-of-range
      (c > maxDist) ||     //d3 out-of-range 
      (d > maxDist) ||     //d4 out-of-range
      ((s1 - a) < 0) ||    //these values must be positive
      ((s1 - b) < 0) || 
      ((s2 - c) < 0) ||
      ((s2 - d) < 0) || 
      ((s1 - Baseline) < 0) ||
      ((s2 - Baseline) < 0)
      ) 
    {
      distances_valid=false;
      X=1000;             
      Y=1000;             
    }

    // ----- apply Heron's formula
    if (distances_valid)
    {
      float area1 = sqrt(s1 * (s1 - a) * (s1 - b) * (s1 - Baseline));
      float M1 = 2 * area1 / Baseline;
      X = sqrt (pow (a, 2) - pow (M1, 2));
      
      float area2 = sqrt(s2 * (s2 - c) * (s2 - d) * (s2 - Baseline));
      float M2 = 2 * area2 / Baseline;
      Y = sqrt (pow (c, 2) - pow (M2, 2));
      
      Z = 100 - sqrt (pow (M2, 2) - pow (Baseline - X + Offset, 2));
      
      // ----- display data for valid echos
      print("D1: "); 
      println(a);
      print("D2: "); 
      println(b);
      print("D3: "); 
      println(c);
      print("D4: "); 
      println(d);     
      print("Offset: "); 
      println(Offset);
      print("S1: "); 
      println(s1);
      print("Area1: "); 
      println(area1);
      print("S2: "); 
      println(s2);
      print("Area2: "); 
      println(area2);
      print("X: "); 
      println(X);
      print("Y: "); 
      println(Y);
      print("Z: "); 
      println(Z);
      println("");
    }
    myPort.clear();                                //clear the receive buffer //<>//
  }
}

// ==========================
// DRAW_GRID
// ==========================
void draw_grid()
{
  pushMatrix();
  scale(0.8);
  translate(width*0.2, height*0.10);  
  fill(0);
  stroke(255);

  // ----- border
  strokeWeight(4);
  line(0, height, 400, height);
  line(0, height, 0, height - 400);
  line(0, height, 400*angle, height - (400*angle));
  line(400*angle, height - (400*angle), 400*angle, height - (400*angle) - 400);
  line(0, height - 400, 400*angle, height - (400*angle) - 400);
  line(400*angle, height - (400*angle) - 400, 400*angle + 400, height - (400*angle) - 400);
  line(400*angle, height - (400*angle), 400*angle + 400, height - (400*angle));
  line(400*angle + 400, height - (400*angle), 400*angle + 400, height - (400*angle) - 400);
  line(400, height, 400*angle + 400, height - (400*angle));

  // ----- lines
  strokeWeight(1);
  line(0, height - 40, 400*angle, height - (400*angle) - 40);
  line(0, height - 80, 400*angle, height - (400*angle) - 80);
  line(0, height - 120, 400*angle, height - (400*angle) - 120);
  line(0, height - 160, 400*angle, height - (400*angle) - 160);
  line(0, height - 200, 400*angle, height - (400*angle) - 200);
  line(0, height - 240, 400*angle, height - (400*angle) - 240);
  line(0, height - 280, 400*angle, height - (400*angle) - 280);
  line(0, height - 320, 400*angle, height - (400*angle) - 320);
  line(0, height - 360, 400*angle, height - (400*angle) - 360);

  //// ----- more lines
  line(40*angle, height - (40*angle), 40*angle + 400, height - (40*angle));
  line(80*angle, height - (80*angle), 80*angle + 400, height - (80*angle));
  line(120*angle, height - (120*angle), 120*angle + 400, height - (120*angle));
  line(160*angle, height - (160*angle), 160*angle + 400, height - (160*angle));
  line(200*angle, height - (200*angle), 200*angle + 400, height - (200*angle));
  line(240*angle, height - (240*angle), 240*angle + 400, height - (240*angle));
  line(280*angle, height - (280*angle), 280*angle + 400, height - (280*angle));
  line(320*angle, height - (320*angle), 320*angle + 400, height - (320*angle));
  line(360*angle, height - (360*angle), 360*angle + 400, height - (360*angle));
  
  //// ----- even more lines
  line(400*angle + 40, height - (400*angle), 400*angle + 40, height - (400*angle) - 400);
  line(400*angle + 80, height - (400*angle), 400*angle + 80, height - (400*angle) - 400);
  line(400*angle + 120, height - (400*angle), 400*angle + 120, height - (400*angle) - 400);
  line(400*angle + 160, height - (400*angle), 400*angle + 160, height - (400*angle) - 400);
  line(400*angle + 200, height - (400*angle), 400*angle + 200, height - (400*angle) - 400);
  line(400*angle + 240, height - (400*angle), 400*angle + 240, height - (400*angle) - 400);
  line(400*angle + 280, height - (400*angle), 400*angle + 280, height - (400*angle) - 400);
  line(400*angle + 320, height - (400*angle), 400*angle + 320, height - (400*angle) - 400);
  line(400*angle + 360, height - (400*angle), 400*angle + 360, height - (400*angle) - 400);

  fill(255);                                    //white text
  textAlign(LEFT, TOP);
  text("0cm", -20, height+10);                   
  text("100cm", width*0.5-20, height+10);       

  textAlign(RIGHT, CENTER);
  text("100cm", -10, height - 400);                 
  text("100cm", 400*angle + 80, height - (400*angle) - 25);

  popMatrix();
}

// ==========================
// DRAW_OBJECT
// ==========================
void draw_object()
{
  pushMatrix();
  scale(0.8);
  stroke(0, 255, 0);
  strokeWeight(1);
  translate(width*0.2, height*1.1);              //(0,0) now lower-left corner
  
  // ----- make the object flash
  if ((frameCount-Frame_count)>4)
  {
    Frame_visible = !Frame_visible;
    Frame_count = frameCount;
  }

  // ----- object color scheme
  if (Frame_visible)
  {
    // ----- make object visible
    Object.setFill(color(255, 0, 0, 255));      //opaque
    Object.setStroke(color(255, 0, 0, 255));    //opaque
  } 
  else
  {
    // ----- hide the object
    Object.setFill(color(255, 0, 0, 0));        //clear
    Object.setStroke(color(255, 0, 0, 0));      //clear
  }

  // ----- draw the object
  pushMatrix();
  translate((X + Y*angle)*4, -((Z + Y*angle)*4));
  shape(Object);
   
  // ----- draw lines to corresponding axis
  line(0, 0, 0, Z*4);
  line(0, 0, -X*4, 0);
  line(0, 0, ((100 - Y)*angle)*4, -(((100 - Y)*angle)*4)); //<>//
  
  popMatrix();
  popMatrix();
}

void draw_menu()
{
  pushMatrix();
  scale(0.8);
  stroke(0, 255, 0);
  strokeWeight(1);
  textAlign(LEFT, CENTER);
  text("CURRENT VALUES: ", 100, 100);
  text("X: " + X, 100, 125);
  text("Y: " + Y, 100, 150);
  text("Z: " + Z, 100, 175);
  popMatrix();
}

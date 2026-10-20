#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <SPI.h>

//.....................................Settings.....................................

byte numMenuOptions = 2;
byte digSpeed = 80;// 0 - 100
int displayRefreshInterval = 200;
int WarningPeriod = 5;//sec
const int MAX_HAZARDS = 10; // Memory Limit

//..........................Constants and Initialisations...............................

// MAX7219 Dot Matrix Control registers
#define DECODE_MODE 9 
#define INTENSITY 0x0A
#define SCAN_LIMIT 0x0B 
#define SHUTDOWN 0x0C
#define DISPLAY_TEST 0x0F

//OLED
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//JoyStick
byte VERT_PIN = A15;
byte HORZ_PIN = A14;
byte SEL_PIN = 2;
byte BACK_PIN = 3;
int vertPos = analogRead(VERT_PIN);
int horzPos = analogRead(HORZ_PIN);

//Stepper
byte stepDir = 11;
byte stepRot = 12;
String status = "Stationary";
int steps = 0;

//Servo
Servo servo;
byte servoPin = 13;
int angle = 0;

//MPU6050
const int MPU_ADDR = 0x68;
int16_t gyroX, gyroY;
float jerk = 0;
int magJerk = 0;

//RGB-LED
byte red = 4;
byte green = 5;
byte blue = 6;

//buzzer
byte buzzPin = 22;
unsigned int hit = 0;
bool isJammed = false;

//Dot Matix
byte Din = 8;
byte CS = 9;
byte CLK = 10;

unsigned long lastAnimUpdate = 0;
bool animToggle = false;
byte hazardlevel = 0;
int l = 0;

//Hazards
int hazardSteps[MAX_HAZARDS]; 
int hazardJerks[MAX_HAZARDS]; 
int hazardCount = 0;
int viewHazardIndex = 0;

//Menu details
byte currentMenuOption = 1;
volatile bool subMenu = false;

//TIMERS
unsigned long lastMenuScroll = 0;       
unsigned long lastDisplayUpdate = 0;
unsigned long lastStepTime = 0;
bool stepPinState = LOW;

volatile unsigned long lastSelectTime = 0; 
volatile unsigned long lastBackTime = 0;

unsigned long lastScrollTime = 0; 


//...................................Menu/Display......................................
void mainMenuOptions(){
  vertPos = analogRead(VERT_PIN);
  if(vertPos > 1000){
    currentMenuOption -= 1;
  }else if(vertPos < 100){
    currentMenuOption += 1;
  }
  currentMenuOption = constrain(currentMenuOption, 1, numMenuOptions); 
}

void subMenuOptions(){
  switch(currentMenuOption){
    case 1:
    ControlDrill();
    break;
    case 2:
    Readings();
    break;
  }
}

void displayOption(int option){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.drawRoundRect(0, 0, 128, 64, 5, WHITE);
  display.setCursor(5, 20);

  switch(option){
    case 1:
      display.println("1.Control");
      display.setCursor(30, 40);
      display.println("Drill");
      display.display();
      break;
    
    case 2:
    display.setCursor(5, 20);
    display.println("2.View");
    display.setCursor(20, 40);
    display.println("Readings");
    display.display();
    break;

  }
}

void subDisplay(int option){
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.drawRoundRect(0, 0, 128, 64, 5, WHITE);

  switch(option){
    case 1:
    display.setTextSize(1);
    // Title
    display.setCursor(20, 2);
    display.println("TELEMETRY DASH");
    display.drawLine(0, 10, 128, 10, WHITE);

    // Depth
    display.setCursor(5, 20);
    display.print("Depth:  ");
    display.print(steps);
    display.println(" steps");

    // Servo tilt angle
    display.setCursor(5, 35);
    display.print("Tilt :  ");
    display.print(angle);
    display.println(" deg");

    // Direction status
    display.setCursor(5, 50);
    display.print("Status: ");
    display.println(status);

    display.display();
    break;

    case 2:
      display.setTextSize(1);
      display.setCursor(15, 2);
      display.println("LOGGED HAZARDS");
      display.drawLine(0, 10, 128, 10, WHITE);

      if (hazardCount == 0){
        display.setCursor(10, 30);
        display.println("No hazards logged...");
      }else{
        display.setCursor(5, 20);
        display.print("Hazard ");
        display.print(viewHazardIndex + 1);
        display.print(" of ");
        display.println(hazardCount);

        display.setCursor(5, 35);
        display.print("Depth: ");
        display.print(hazardSteps[viewHazardIndex]);
        display.println(" steps");

        display.setCursor(5, 50);
        display.print("Jerk : ");
        display.print(hazardJerks[viewHazardIndex]);
      }
      display.display();
      break;

    case 4:
    display.setTextSize(2);
    display.setCursor(5, 10);
    display.println("HARD ROCK");
    display.setCursor(35, 30);
    display.println("HIT/!\\");

    display.display();
    break;


  }
}


//...............................1.Control Drill...............................
void ControlDrill(){
  vertPos = analogRead(VERT_PIN);
  horzPos = analogRead(HORZ_PIN);

  if(vertPos > 1000){
    digitalWrite(stepDir, LOW);
    
    if(micros() - lastStepTime >= (2000-(digSpeed)*10) ){
      stepPinState = !stepPinState;
      digitalWrite(stepRot, stepPinState);
      lastStepTime = micros();
      if(stepPinState == HIGH)
        steps--;
    }
    status = "Retracting";
  }else if(vertPos < 100 && !isJammed){ 
    digitalWrite(stepDir, HIGH);
    
    if(micros() - lastStepTime >= 2000-(digSpeed)*10 && steps<1400){
      stepPinState = !stepPinState;
      digitalWrite(stepRot, stepPinState);
      lastStepTime = micros();
      if(stepPinState == HIGH)
        steps++;
    }
    status = "Descending";
  }else{ 
    status = "Stationary";
  }

  //Servo
  angle = analogRead(HORZ_PIN);
  angle = map(angle, 0, 1023, 0, 180);
  servo.write(angle);

  //Jerk-LED,Buzzer
  jerk = getjerk();
  analogWrite(blue,255 - constrain(jerk,0,255));
  analogWrite(red, constrain(jerk,0,255));

  if(jerk >= 255 && !isJammed){
    tone(22,262,2);
    hazardlevel = l;

    magJerk = jerk-255;
    if(hazardCount < MAX_HAZARDS){
      hazardSteps[hazardCount] = steps;
      hazardJerks[hazardCount] = (int)magJerk;
      hazardCount++;
    }
    isJammed = true;
    subDisplay(4);
    delay(WarningPeriod*1000);
    subMenu = false;
  }else if(jerk >= 255 && isJammed){
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.drawRoundRect(0, 0, 128, 64, 5, WHITE);

    display.setCursor(2,20);
    display.println("WARNING:  DRILL STUCK");
    display.display();
  }else if(jerk < 255){
    isJammed = false;
  }

  dotMatrix();
}

float getjerk(){
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  gyroX = (Wire.read() << 8) | Wire.read();
  gyroY = (Wire.read() << 8) | Wire.read();

  float gX = (float)gyroX/131.0;
  float gY = (float)gyroY/131.0;

  float squarX = gX*gX;
  float squarY = gY*gY;
  return sqrt(squarX + squarY);
}

//....................................Dot Matix..............................

void SendData(int address, uint8_t value){ 
  if(address <= 0 || address > 8){
    return;
  }
  if(currentMenuOption == 1 && address == hazardlevel)
    value = value | B00000001;
  digitalWrite(CS, LOW);   
  shiftOut(Din, CLK, MSBFIRST, address);  
  shiftOut(Din, CLK, MSBFIRST, value);    
  digitalWrite(CS, HIGH);
}

void dotMatrix(){
  if(millis() - lastAnimUpdate >= 1000 - 5*digSpeed){
    animToggle = !animToggle;
    lastAnimUpdate = millis();
  }
  l = map(steps, -200, 1400, 1, 9);
  animateDrill(l);
}

void animateDrill(int i){
  for(int j = 1; j<i-4 && j>0; j++)
    SendData(j, B00111000);
    SendData(i-4, animToggle ? B01011100 : B01110100);
    SendData(i-3, animToggle ? B01011100 : B01110100);
    SendData(i-2, B00111000);
    SendData(i-1, B00010000);

    for(int k=i; k<=8 ; k++)
    SendData(k, B00000000);
}

//..................................2.View Readings...................................

void Readings(){
  vertPos = analogRead(VERT_PIN);

  if (millis() - lastScrollTime > 300){
    if (vertPos > 1000 && viewHazardIndex > 0){
      viewHazardIndex--;
      lastScrollTime = millis();
    }else if(vertPos < 100 && viewHazardIndex < hazardCount - 1){
      viewHazardIndex++;
      lastScrollTime = millis();
    }
  }
  subDisplay(2);
  drawHazardGraph();
}

void drawHazardGraph() {
  for(int row = 1; row <= 8; row++){
    byte rowData = B00000000;

    int displayLimit = min(hazardCount, 8); 

    for(int col = 0; col < displayLimit; col++){
      int height = map(hazardJerks[col], 0, 100, 1, 8); 
      
      int thresholdRow = 9 - height; 
      if(row >= thresholdRow){
        rowData |= (1 << (col)); 
      }
    }
    //We SendData here so the hazardRow dot doesn't overwrite our graph in the SendData function
    digitalWrite(CS, LOW);   
    shiftOut(Din, CLK, MSBFIRST, row);  
    shiftOut(Din, CLK, MSBFIRST, rowData);    
    digitalWrite(CS, HIGH);
  }
}

//..................................Interupts.................................
void Select(){
  if (millis() - lastSelectTime > 50){
    subMenu = true;
    lastSelectTime = millis();
  }
}

void Back(){
  if (millis() - lastBackTime > 50){
    subMenu = false;
    lastBackTime = millis();
  }
}

//...........................Setup and Loop.............................
void setup() {
  pinMode(VERT_PIN, INPUT);
  pinMode(HORZ_PIN, INPUT);
  pinMode(SEL_PIN, INPUT_PULLUP);
  pinMode(BACK_PIN, INPUT);
  pinMode(stepDir, OUTPUT);
  pinMode(stepRot, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(Din, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(CS, OUTPUT);
  servo.attach(13);
  analogWrite(blue, 255);

//Initialisation of the Dot matrix
  SendData(DISPLAY_TEST, 0x01);
  delay(1000);
  SendData(DISPLAY_TEST, 0x00);
  SendData(DECODE_MODE, 0x00);
  SendData(INTENSITY, 0x0e);
  SendData(SCAN_LIMIT, 0x0f);
  SendData(SHUTDOWN, 0x01);

  attachInterrupt(digitalPinToInterrupt(2),Select,FALLING);
  attachInterrupt(digitalPinToInterrupt(3),Back,FALLING);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
}

void loop() {
  if(subMenu)
    subMenuOptions();
  else
    mainMenuOptions();
  
  if(millis() - lastDisplayUpdate >= displayRefreshInterval){
    lastDisplayUpdate = millis();
    if(subMenu){
      if(!isJammed)
        subDisplay(currentMenuOption);
    }else
      displayOption(currentMenuOption);
  }
}

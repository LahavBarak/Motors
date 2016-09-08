#include <SPI.h>
#include "RF24.h"

//radio pinout
int CE = 2;
int CSN = 3;

//joystick pinout
int VERT = 1;
int HORZ = 2;

RF24 transmitter(CE,CSN);

byte addresses[][6] = {"1Node","2Node"};

/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */

void setup() {
  transmitter.begin();
  transmitter.setPALevel(RF24_PA_HIGH);
  transmitter.setDataRate(RF24_2MBPS);
  transmitter.openWritingPipe(addresses[0]);
  transmitter.openReadingPipe(1,addresses[1]);
}

/* -------------------------------------------------------------------------------------------- */
/*what takes longer? sending one 4 byte messege and decoding, or three 1 byte messeges and decoding one?
 * V2 is one messege. V3 will test the split messege concept.
 */
/* -------------------------------------------------------------------------------------------- */

void loop() {
/* ---- variable definition ---- */
  unsigned long joystickData; // unsigned long is 0-4,XXX,XXX,XXX. package is coded as follows: S;V,v1,v2;v3,H,h1;h2,h3,X
                              // S = SEL State; V = Vertical direction; v1,v2,v3 = vertical value; H=Horizontal direction
                              // h1,h2,h3 = horizontal value, X=N/A(not needed, not used for noise filtering.);
  int  vertPos;               // will contains the vertical value of the motors.
  int  horzPos;               // will contain the horizontal value of the motors.
  bool vertDir;               // true = up, false = down;
  bool horzDir;               // true = right, false = left;
  unsigned long responseData; //the return package. should be identical to the sent package

/* ---- data acquisiton ---- */
  GetJoystickData(&vertPos,&horzPos,&vertDir,&horzDir);
  
  joystickData = EncodeData(vertPos,horzPos,vertDir,horzDir);

/* ---- data transmittion ---- */
  transmitter.stopListening();
  transmitter.write(&joystickData,sizeof(unsigned long));

/* ---- confirm transmittion ---- */
  transmitter.startListening();
  unsigned long timer = micros();
  while(!transmitter.available()){
    if(micros()-timer >20000){
      timer = 0;
      break;
    }
  }

  if(timer != 0){
    transmitter.read(&responseData,sizeof(unsigned long));
  }
  transmitter.stopListening();
}

/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */

void GetJoystickData(int* vertPos, int* horzPos, bool* vertDir, bool* horzDir){
  int rawVert = analogRead(VERT);           // takes in raw input from Vert Joystick
  int rawHorz = analogRead(HORZ);           // takes in raw input from Horz Joystick
/* ---- modifiying Horz value ---- */
  if (rawHorz <= 526) {                     // 526 = Horz rest value (requires calibration for every new joystick)
    *horzDir = false;                       // < than rest value = going left
    *horzPos = map(rawHorz,0,526,255,0);    // mapping in reverse - the closer the raw value is to 0 - 
                                            // the further the joystick is from rest position = motor spins faster;
  }
  else{                           
    *horzDir = true;                        // > than rest value = going right
    *horzPos = map(rawHorz,526,1024,0,255); // mapping linearly. the closer the raw value is to 1024 -
                                            // the further the joystick is from rest position.
  }
/* ---- modifying Vert value ---- */
  if (rawVert <= 532) {
    *vertDir = false;
    *vertPos = map(rawVert,0,532,255,0);
  }
  else{
    *vertDir = true;
    *vertPos = map(rawVert,532,1024,0,255);
  }
  return;  
}

/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */

unsigned long EncodeData(int vertPos, int horzPos, bool vertDir, bool horzDir){
  unsigned long package = 5;                // package = S;V,v1,v2;v3,H,h1;h2,h3,5
  package += horzPos*10;                    // make h1,h2,h3 into h1;h2,h3,X
  package += int(horzDir)*10000;            // make H into H,X;X,X,5
  package += vertPos*100000;                // make v1,v2,v3 into v1,v2;v3,X,X;X,X,5
  package += int(vertDir)*100000000;        // make V into V,v1,v2;v3,H,h1;h2,h3,5
  return package;
}


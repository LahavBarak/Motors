#include <SPI.h>
#include "RF24.h"

//radio pinout
int CE = 2;
int CSN = 3;

//motor A pinout
int IN1 = 5;
int IN2 = 6;
int ENA = 7;

//motor B pinout
int ENB = 8;
int IN3 = 9;
int IN4 = 10;

RF24 reciever(CE,CSN);

byte addresses[][6] = {"1Node","2Node"};

/* -------------------------------------------------------------------------------------------- */
/*what takes longer? sending one 4 byte messege and decoding, or three 1 byte messeges and decoding one?
 * V2 is one messege. V3 will test the split messege concept.
 */
/* -------------------------------------------------------------------------------------------- */

void setup() {
/* ---- motor A setup ---- */
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(ENA,OUTPUT);
  digitalWrite(ENA,HIGH);
  
/* ---- motor B setup ---- */
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  pinMode(ENB,OUTPUT);
  digitalWrite(ENB,HIGH);

/* ---- reciever setup ---- */
  reciever.begin();
  reciever.setPALevel(RF24_PA_HIGH);
  reciever.setDataRate(RF24_2MBPS);
  reciever.openWritingPipe(addresses[1]);
  reciever.openReadingPipe(1,addresses[0]);
}

/* -------------------------------------------------------------------------------------------- */
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

/* ---- data acquisition ---- */
  reciever.startListening();
  while(reciever.available()){
    reciever.read(&joystickData,sizeof(unsigned long));
  }
  
 

/* ---- transmit response ---- */
  reciever.stopListening();
  reciever.write(&joystickData,sizeof(unsigned long));
  reciever.startListening();
  
/* ---- crunch data ---- */
  DecodeData(joystickData,&vertPos,&horzPos,&vertDir,&horzDir);

  OutputData(vertPos,horzPos,vertDir,horzDir);
}

/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */

void DecodeData(unsigned long package, int* vertPos, int* horzPos, bool* vertDir, bool* horzDir){
  
  *vertDir = (package/100000000);        // make V,v1,v2;v3,H,h1;h2,h3,5 into V
  package -= (package/100000000)*100000000;  // erase V from package
  
  *vertPos = (package/100000);            // make v1,v2;v3,H,h1;h2,h3,5 into v1,v2,v3
  package -= (package/100000)*100000;        // erase v1,v2,v3 from package
  
  *horzDir = (package/10000);            // make H,h1;h2,h3,5 into H
  package -= (package/10000)*10000;          // erase H from package
  
  *horzPos = (package/10);                // make h1;h2,h3,5 into h1,h2,h3

  return;
}

/* -------------------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------------------- */

void OutputData(int vertPos, int horzPos,bool vertDir, bool horzDir){

/* ---- Vert Output ---- */
  if(vertDir == true){      // if vertDir = true -> going 'up'
    analogWrite(IN1,vertPos);
    digitalWrite(IN2,LOW);
  }
  else{                     // if vertDir = false -> going 'down'
    digitalWrite(IN1,LOW);
    analogWrite(IN2,vertPos);
  }

/* ---- Horz Output ---- */
  if(horzDir == true){      // if horzDir = true -> going 'right'
    analogWrite(IN3,horzPos);
    digitalWrite(IN4,LOW);
  }
  else{                     // if horzDir = false -> going 'left'
    digitalWrite(IN3,LOW);
    analogWrite(IN4,horzPos);
  }
  return;
}

//analog read 10kHz and Serial Out
//Arduino UNO
//MaDA Lab Koji Ohashi 2019.10.23

#include "Timer_AGT_One.h"

volatile int arrayIndex = 0;  //index of last measurement
const int T1_us=100;         //timer1 interrupt interval (microseconds)---10kHz
const int SizeTx=30;          //serial out max data block size
const int SizeBuffer=128;     //analog read circular buffer size
const int margin=5;           //margin of data block
byte data[(SizeTx+2)*2];      //serial out block array
int circular[SizeBuffer];     //circular buffer array
int startIndex=0;             //next serial out position of circular buffer

void setup() {
  Serial.begin(230400);
  Serial.println("Continuous_AnalogIn10kHz_SerialOut");
  ADCSRA = ADCSRA & 0xf8;   // 分周比を決めるビット(ADPS2:0)を000へ
  //ADCSRA = ADCSRA | 0x07;   // 分周比を決めるビットに分周比128(111)をセット(default value)
  ADCSRA = ADCSRA | 0x04;   // 分周比を決めるビットに分周比16(100)をセット
  Timer1.initialize(T1_us);
  Timer1.attachInterrupt(readAnalog);
  delay((SizeTx-margin)*T1_us/1000);
}

void readAnalog(){
  circular[arrayIndex] = analogRead(A0);
  arrayIndex=(arrayIndex+1)%SizeBuffer;
}

void loop() {
  int actSizeTx, readIndex;
  noInterrupts();
  int currentIndex=(arrayIndex-1)%SizeBuffer;
  interrupts();
  if(currentIndex>=startIndex){
    actSizeTx=currentIndex-startIndex+1;
  } else{
    actSizeTx=currentIndex + SizeBuffer - startIndex+1;
  }
  data[0]=255;
  data[1]=255;
  data[2]=0;
  data[3]=actSizeTx;
  for(int i=0; i<actSizeTx; i++){
    readIndex=(startIndex+i)%SizeBuffer;
    int sensorValue = circular[readIndex];
    data[(i+2)*2]= byte(sensorValue >> 8);
    data[(i+2)*2+1]= byte(sensorValue & 0x00FF);
  }
  Serial.write(data,(actSizeTx+2)*2);
  startIndex=(readIndex+1)%SizeBuffer;
  delay(1);
}

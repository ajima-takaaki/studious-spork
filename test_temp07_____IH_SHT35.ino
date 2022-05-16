//---------------------------------------------------//
//  AJIMA test_temp07
//  Version 0.0  2022-05-07
//---------------------------------------------------//


//温湿センサSHT35使用で追加

#include <Arduino.h>
#include <Wire.h>
#include "AE_SHT35.h"

//温度のlog計算用
#include <math.h>

// aruduino uno pin assign of
// CN4
const int motorALERT =2;      // 過電流検知　＠モータドライバ東芝TB67H303HG。割り込み入力ピンなのでコプト設計回路のIN2と入れ替え。
const int motorPWM =  3;      // PWM入力ピン、全開で回す場合はHIGHにしとくのかな。＠モータドライバ東芝TB67H303HG
const int motorSTBY = 4;      // スタンバイ入力ピン　＠モータドライバ東芝TB67H303HG
const int motorIN1 =  5;      // 回転方向制御ピン　IN2とてれこでHIGHとLOWを切り替える　＠モータドライバ東芝TB67H303HG
const int motorIN2 =  6;      // 回転方向制御ピン　＠モータドライバ東芝TB67H303HG
                //pin 7 = no contact
//CN5
const int Heater =    8;      // optocoupler for heater
const int Fan =       9;      // cooling fan
const int LedHEATER = 10;      // LED01 MisI ピン、ヒータONの時に光らせるLED
const int LedFAN =    11;     // LED01 MisO ピン、ファンONの時にひかるLED
const int LedERROR =  12;       //LED02 SCK ピン、エラーの時にひかるLED

//CN6
const int TempPin0 = A0;  //ヒータプレート温度　FC50流用
const int TempPin1 = A1;  //排気温度　FC50流用
const int TempPin2 = A2;  //鍋底　
const int TempPin3 = A3;  //排気湿球温度
//const int TempPin4 = A4;
//const int TempPin[5] = {TempPin0, TempPin1, TempPin2, TempPin3, TempPin4};

//const int HeaterTempPin = A0;      // ヒータ用サーミスタ1のアナログ入力
//const int ExgasTempPin =  A1;      // 排気ガス用サーミスタ2のアナログ入力
//const int LedError =      A2;      // LED03　エラー時にひかるLED

// variables will change:
//int buttonState = 0;         // variable for reading the pushbutton status
int stopCount =0;       //過電流でモータドライバをリセットした回数。ネットで割込の関数に使う場合はvalatile付けろって書いてあったんで。
int prevStopCount = 0;
//uint8_t i;

float tempSet = 130.0;   //ヒータの目標温度
//float rDiv[] = {50000, 9980, 50500, 50300, 50200};
//int ReadVal[] = {0, 0, 0, 0, 0};
//float variableA[] = {-26.739, -27.028, -26.304, -26.312, -26.448};
//float variableB[] = {129.77, 88.691, 126.77, 127, 126.96};
//float rDivHeater = 50000;  //ヒータ温度分圧測定のための抵抗Ω
//float rDivExgas = 9980;  //排ガス温度分圧測定のための抵抗Ω
//int ReadValExgas = 0;   //アナログ入力で読み取る排ガス温度抵抗データの格納変数
float temp[] = {0, 0, 0, 0, 0};

int direction = 0;
void reverseMotor() {
  int currentDirection = digitalRead(motorIN1);
  direction = !currentDirection;
  digitalWrite(motorIN1, direction);
  digitalWrite(motorIN2, !direction);
}

//void motorReset() {
// digitalWrite(motorSTBY, LOW);　//モータドライバ仕様だと過電流検知後はSTBYがLOWに落ちるらしいので実験用にLOWに落とす
 // stopCount =  stopCount + 1;
//止まった時の回転方向を反転させる
// if (motorIN1 == HIGH) {
//    digitalWrite(motorIN1, LOW);
//    digitalWrite(motorIN2, HIGH);
//  } else if
//  (motorIN1 == LOW) {
//    digitalWrite(motorIN1, HIGH);
//    digitalWrite(motorIN2, LOW);  
//  }
//digitalWrite(motorSTBY, HIGH);
//delay(5000);
//digitalWrite(motorSTBY, LOW);
//}

// SHT35のアドレスを設定 (4pin-5pin オープン)
AE_SHT35 SHT35 = AE_SHT35(0x45);


void setup() {
  // pinの入出力設定
  pinMode(motorIN2, OUTPUT);
  pinMode(motorSTBY, OUTPUT);
  pinMode(motorIN1, OUTPUT);
  pinMode(motorALERT, INPUT);
  pinMode(Fan, OUTPUT);
  pinMode(Heater, OUTPUT);
  //pinMode(holeIC1, OUTPUT);
  pinMode(LedHEATER, OUTPUT);
  pinMode(LedFAN, OUTPUT);
  pinMode(LedERROR, OUTPUT);
//for(i = 0; i < 5; i++) { 
  pinMode(TempPin0, INPUT);
  pinMode(TempPin1, INPUT);
  pinMode(TempPin2, INPUT);
  pinMode(TempPin3, INPUT);
//  pinMode(TempPin4, INPUT);

  //pinMode(HeaterTempPin, INPUT);
  //pinMode(ExgasTempPin, INPUT);
  //pinMode(LedError, OUTPUT);  
  //attachInterrupt( 0, motorReset, FALLING); //デジタルピン2→割り込み番号は0、

  direction = digitalRead(motorIN1);

  Serial.begin(9600);

// SHT35をソフトリセット
  SHT35.SoftReset();
  // 内蔵ヒーター 0:OFF 1:ON
  SHT35.Heater(0);

}

void loop() {
  
  
  
//３回過電流検知されたらモータを止める
//  while(stopCount <= 3) {

//配列化のトライ
//for(i = 0; i < 5; i++) {
//int ReadVal[i] =analogRead(TempPin[i]);
//float vout[5];
//float rth[5];
//float vout[i] = (float)ReadVal[i] / 1024.0f * 4.97f;  
//float rth[i] =( 4.97f / vout[i] -1 ) * rDiv[i];          //サーミスタ抵抗計算
//temp[i] = variableA[i] * log( rth[i]/1000.0f ) + variableB[i] ; //測定値の回帰式より求めた温度計算（摂氏）kΩで出しちゃったから1000で割る
//}

  /* 0 ヒータ温度の計算 */
 int ReadVal0 = analogRead(TempPin0);            //アナログ入力0でデータ読み取り
 float vout0 = (float)ReadVal0 / 1024.0f * 4.95f; //分圧した出力電圧の計算、ardの5vは実測値4.95vを入れる
 float rth0 = ( 4.95f / vout0 -1 ) * 50000;          //サーミスタ抵抗計算
 float temp0 = -26.739f * log( rth0/1000.0f ) + 129.53f ; //測定値の回帰式より求めた温度計算（摂氏）kΩで出しちゃったから1000で割る

  /* 1 排気温度の計算 */
 int ReadVal1 = analogRead(TempPin1);            
 float vout1 = (float)ReadVal1 / 1024.0f * 4.95f; 
 float rth1 = ( 4.95f / vout1 -1 ) * 9980;         
 float temp1 = -27.028f * log( rth1/1000.0f ) + 88.691f ; 

  /* 2 鍋底温度の計算 */
 int ReadVal2 = analogRead(TempPin2);            
 float vout2 = (float)ReadVal2 / 1024.0f * 4.95f; 
 float rth2 = ( 4.95f / vout2 -1 ) * 50500;         
 float temp2 = -26.304f * log( rth2/1000.0f ) + 126.77f ; 

  /* 3 湿度用温度の計算 */
 int ReadVal3 = analogRead(TempPin3);            
 float vout3 = (float)ReadVal3 / 1024.0f * 4.95f; 
 float rth3 = ( 4.95f / vout3 -1 ) * 50300;         
 float temp3 = -26.312f * log( rth3/1000.0f ) + 127.0f ; 

 /* 4 蓋近傍温度の計算 */
 //int ReadVal4 = analogRead(TempPin4);            
 //float vout4 = (float)ReadVal4 / 1024.0f * 4.95f; 
 //float rth4 = ( 4.95f / vout4 -1 ) * 50200;         
 //float temp4 = -26.448f * log( rth4/1000.0f ) + 129.96f ; 

/* 排気湿度の計算 */
//float ps = 6.11*pow(10, (7.5*temp1/(237.3+temp1))); // ps:乾球温度の飽和水蒸気圧
//float pst = 6.11*pow(10, (7.5*temp3/(237.3+temp3))); // pst:湿球温度の飽和水蒸気圧
//float pt = pst - 0.0012*1013.25*(temp1-temp3)*(1+temp3/610); //pt:ペルンターの式による水蒸気圧
//float pts = pst - 0.000662*1013.25*(temp1-temp3); //pts:スプルングの式による水蒸気圧

//int HM = (pt/ps)*100; //HM:相対湿度
//int HM2 = (pts/ps)*100; //HM2:通風時の相対湿度

/* SHT35から温湿度データを取得 */
  SHT35.GetTempHum();
 
 
 /* 排気ガス温度の計算 */
 // int ReadValExgas = analogRead(ExgasTempPin);            //アナログ入力0でデータ読み取り
 // float voutExgas = (float)ReadValExgas / 1023.0f * 4.95f; //分圧した出力電圧の計算、ardの5vは実測値4.95vを入れる
 // float rthExgas = ( 4.95f / voutExgas -1 ) * rDivExgas;          //サーミスタ抵抗計算
 // float tempExgas = -27.028f * log( rthExgas/1000.0f ) + 88.691f ; //測定値の回帰式より求めた温度計算（摂氏）kΩで出しちゃったから1000で割る
  
 /* デバッグ用出力 */   
 //   Serial.print("ReadVal: ");   
 //   Serial.print( ReadVal0);
 //   Serial.print(" " );  
 //   Serial.print( ReadVal1);
 //   Serial.print(" " );  
 //   Serial.print( ReadVal2);
 //   Serial.print(" " );  
 //   Serial.print( ReadVal3);
 //   Serial.print(" " );  
 //   Serial.println(ReadVal4);
  
  
  
  /* 温度・湿度dataの記録 */   
    Serial.print("Temp *C: ");   // Temp:  と出力（print）
    //Serial.print( temp0);
    //Serial.print(" " );  
    //Serial.print( temp1);
    //Serial.print(" " );  
    Serial.print( temp2);
    Serial.print(" " );  
    //Serial.print( temp3);
    //Serial.print(" " );  
    //Serial.print(HM);
    //Serial.print(" " );  
    //Serial.print(HM2);
    //Serial.print(" " ); 
    Serial.print(SHT35.Temperature());
    Serial.print(" " ); 
    Serial.println(SHT35.Humidity());
 
 /* 温度比較してヒータOn/Off */  
 // if(temp0 < tempSet - 1.0) {
 //     digitalWrite(Heater, HIGH);
 //     //Serial.println("Heater On");       //  Heater On と出力（print）して改行
 //   } else if(temp0 > tempSet + 1.0) {
 //     digitalWrite(Heater, LOW); 
      //Serial.println("Heater Off");       //  Heater Off と出力（print）して改行
 //   }

 
    analogWrite(Fan, 255 ); //ファンのPWM出力
    digitalWrite(LedFAN, HIGH);
    //Serial.println("FAN On");
   
    //digitalWrite(LedERROR, LOW);  
    digitalWrite(motorIN1, direction);
    digitalWrite(motorIN2, !direction);
    //Serial.println("direction start");
    digitalWrite(motorSTBY, HIGH);//モータ回転開始
    digitalWrite(motorPWM, HIGH);//Hにしないと全開で回らない仕様か
    
    delay(12000);
    digitalWrite(motorPWM, LOW);
    digitalWrite(motorSTBY, LOW);
    //Serial.println("direction stop");
    delay(3000);

  /* 温度比較してヒータOn/Off */  
 // if(temp0 < tempSet - 0.5) {
 //     digitalWrite(Heater, HIGH);
 //     digitalWrite(LedHEATER, HIGH);
      //Serial.println("Heater On");       //  Heater On と出力（print）して改行
 //   } else if(temp0 > tempSet + 0.5) {
 //     digitalWrite(Heater, LOW); 
 //     digitalWrite(LedHEATER, LOW);
      //Serial.println("Heater Off");       //  Heater Off と出力（print）して改行
 //   }

    reverseMotor();
    digitalWrite(motorPWM, HIGH);
    digitalWrite(motorSTBY, HIGH);
    //Serial.println("reverse start");
    delay(12000);
    digitalWrite(motorPWM, LOW);
    digitalWrite(motorSTBY, LOW);
    //Serial.println("reverse stop");
    delay(3000);
    reverseMotor();
       
 //   if (stopCount != prevStopCount) {
    // 割り込みが入った
//    reverseMotor();
//    Serial.print("reversed!! ");
//    Serial.println(stopCount);
    
//    } else {
//    prevStopCount = stopCount;  
   // Serial.print("stopCount = ");
   // Serial.println(stopCount);
    
    }

 // }

    /*
    digitalWrite(LedDrying, LOW);
    digitalWrite(LedCooling, HIGH);
    digitalWrite(motorSTBY, LOW);
    digitalWrite(motorPWM, LOW);
    digitalWrite(Fan, LOW);  
    digitalWrite(LedFAN, LOW);  
    digitalWrite(Heater, LOW); 
    digitalWrite(LedHEATER, LOW);
    digitalWrite(LedERROR, HIGH); 
    */
//

/******************************************************************************
----------------本例程仅供学习使用，未经作者允许，不得用于其他用途。-----------
------------------------版权所有，仿冒必究！-----------------------------------
----------------1.开发环境:Arduino IDE-----------------------------------------
----------------2.测试使用开发板型号：Arduino Leonardo or Arduino UNO R3-------
----------------3.单片机使用晶振：16M------------------------------------------
----------------4.淘宝网址：Ilovemcu.taobao.com--------------------------------
----------------------------epic-mcu.taobao.com--------------------------------
----------------5.作者：神秘藏宝室---------------------------------------------
******************************************************************************/
#include <Arduino.h>
#include <MsTimer2.h>               //定时器库的 头文件
//#define DEBUG
#define PRODUCT
/******************************************************************************
 * bu jin dianji code 
 * 
 ******************************************************************************/
#define A1 8			//引脚命名
#define B1 9
#define C1 10
#define D1 11

#define delayTime (5)            // ting dun shi jian
#define upperLimit (64)           // xin hao shang xian 

const int upper = 64 / 2;
int currentPercent = 100;
int newPercent = 150;
int flag = -1;

/******************************************************************************
 * chuankou lanya code 
 * 
 ******************************************************************************/
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

/******************************************************************************
 * infrare tube and timer
 ******************************************************************************/
int ledPin = 13;
int hasDrop = 0;
unsigned long startTime = 0;
unsigned long intervalTime;
unsigned long endTime = 0;
volatile int state = LOW;
#define TIMEOUT (2000)

void setup()
{
        /* bujin dianji */
	pinMode(A1,OUTPUT);		//设置引脚为输出引脚
	pinMode(B1,OUTPUT);
	pinMode(C1,OUTPUT);
	pinMode(D1,OUTPUT);
        /* lanya chuankou */
        Serial.begin(9600);
        /* led */
        pinMode(ledPin, OUTPUT);
        // infrare tube interrupt
        attachInterrupt(0, blink, FALLING);//当int.0电平改变时,触发中断函数blink
        // timer interrupt
        MsTimer2::set(TIMEOUT, printDropSpeed);        // 中断设置函数，每 2000ms 进入一次中断
        MsTimer2::start();                //开始计时
}

void loop()
{
      digitalWrite(ledPin, state);
}

void motorRun(int new_percent)
{
      int need2Rotate = new_percent - currentPercent;
      float loopNum = (float)need2Rotate / 50.0 * (float)upper;
#ifdef DEBUG 
      Serial.print("loopNum:");
      Serial.print(loopNum);
      Serial.print("\r\n");
#endif
      if (need2Rotate>0)
      {
           for (int i=0;i<(int)loopNum;++i)
           {
           rotate_closeWise(8);
           }
      }
      else if (need2Rotate<0)
      {
        for (int i=0;i<int(abs(loopNum));++i)
         {
           rotate_counterCloseWise(8);
         }
      }
      else if (need2Rotate==0)
      {
        
      }
      currentPercent = new_percent;
}

/* uart interrupt code */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') 
    {
      stringComplete = true;
    }
    if (stringComplete) 
    {
      String Msg = inputString.substring(0, Msg.length());
      newPercent = Msg.toInt();
#ifdef DEBUG
      Serial.println(newPercent);
#endif
      /* run the motor */
      motorRun(newPercent);
      // clear the string:
      inputString = "";
      stringComplete = false;
    }
  }
}

void blink()//中断函数
{
    state = !state;
    endTime = millis();
    intervalTime = endTime - startTime;
    startTime = endTime;
    hasDrop = 1;
}

void printDropSpeed()
{
    if (hasDrop)
    {
        float dropSpeed = 1000.0 / (float)intervalTime;
#ifdef DEBUG
        Serial.print("speed:");
        Serial.print(dropSpeed);
        Serial.println("drop/s");
#else
        Serial.print(dropSpeed);
#endif
        hasDrop = 0;  
    }
    else 
    {
        // no drop, send message, alarming
#ifdef DEBUG
        Serial.println("speed:0drop/s");
#else
        Serial.print("0");
#endif
    }
}




























void Phase_A()
{
	digitalWrite(A1,HIGH);		//A1引脚高电平	
	digitalWrite(B1,LOW);
	digitalWrite(C1,LOW);
	digitalWrite(D1,LOW);
}

void Phase_B()
{
	digitalWrite(A1,LOW);	
	digitalWrite(B1,HIGH);		//B1引脚高电平	
	digitalWrite(C1,LOW);
	digitalWrite(D1,LOW);
}

void Phase_C()
{
	digitalWrite(A1,LOW);	
	digitalWrite(B1,LOW);
	digitalWrite(C1,HIGH);		//C1引脚高电平	
	digitalWrite(D1,LOW);
}

void Phase_D()
{
	digitalWrite(A1,LOW);	
	digitalWrite(B1,LOW);
	digitalWrite(C1,LOW);
	digitalWrite(D1,HIGH);		//D1引脚高电平	
}

void rotate_closeWise(int numbers)
{
        Phase_A();
        delay(delayTime);
        if (numbers==1) return;
        
        Phase_A();
        Phase_B();
        delay(delayTime); 
        if (numbers==2) return;
  
        Phase_B();
        delay(delayTime);
        if (numbers==3) return;
        
        Phase_B();
        Phase_C();
        delay(delayTime);
        if (numbers==4) return;
        
        Phase_C();
        delay(delayTime);
        if (numbers==5) return;
        
        Phase_C();
        Phase_D();
        delay(delayTime);
        if (numbers==6) return;
       
        Phase_D();
        delay(delayTime);
        if (numbers==7) return;
        
        Phase_A();
        Phase_D();      
}

void rotate_counterCloseWise(int numbers)
{
        Phase_A();
        Phase_D();   
        if (numbers==1) return;
  
        Phase_D();
        delay(delayTime);
        if (numbers==2) return;
  
        Phase_C();
        Phase_D();
        delay(delayTime);
        if (numbers==3) return;      
  
        Phase_C();
        delay(delayTime);
        if (numbers==4) return;       
  
        Phase_B();
        Phase_C();
        delay(delayTime);
        if (numbers==5) return;
        
        Phase_B();
        delay(delayTime);
        if (numbers==6) return;
      
        Phase_A();
        Phase_B();
        delay(delayTime); 
        if (numbers==7) return;
  
        Phase_A();
        delay(delayTime);
}



#define CUSTOM_SETTINGS
#define INCLUDE_GPS_SHIELD
#define INCLUDE_SMS_SHIELD
#define INCLUDE_PHONE_SHIELD
#define INCLUDE_TEXT_TO_SPEECH_SHIELD
#define trigPin 13 //Sensor Echo pin connected to Arduino pin 13
#define echoPin 12 //Sensor Trip pin connected to Arduino pin 12
#define buzzer 8
#include <OneSheeld.h>
int safetyDistance;
float lat;
float lon;
String PhoneNumber = "+919780438164";
String Message;
String Message1;
String Message2;
char latitude[10];
char longitude[10];
char charVal[4];

//boolean isMessageSent=false;
const int buttonPin1 = 6;    //this need to be changed 
const int buttonPin = 5;//this need to be changed     
const int buttonPin2 = 4;
const int ledPin = 7;

void setup() 
{
  OneSheeld.begin();      
  pinMode(buttonPin, INPUT);
  pinMode(buttonPin1, INPUT);   
  pinMode(trigPin, OUTPUT); //Initializing the pins
  pinMode(echoPin, INPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(ledPin,OUTPUT);
  digitalWrite(buttonPin, LOW);
  digitalWrite(buttonPin1, LOW);
  digitalWrite(buttonPin2, LOW);
}

void loop() {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  safetyDistance = distance;
  if(safetyDistance<=40){
    digitalWrite(buzzer,HIGH);
    digitalWrite(ledPin,HIGH);
  }
  else if(41<=safetyDistance and safetyDistance<=100){
    digitalWrite(buzzer,HIGH);
    digitalWrite(ledPin,HIGH);
    delay(200);
    digitalWrite(buzzer,LOW);
    digitalWrite(ledPin,LOW);
  }
  else{
    digitalWrite(buzzer,LOW);
    digitalWrite(ledPin,LOW);
  }


/*-------------------------------GSM + GPS+ GOOGLE_MAP + PHONE_CALL + VOICE----------------------*/

 lat = GPS.getLatitude();
lon = GPS.getLongitude();
Message1 = String(lat,8);
Message2 = String(lon,8);     


  if (digitalRead(buttonPin) == HIGH) { 
      digitalWrite(ledPin, HIGH);
         Message = "I need Help! I'm at latitude: " + Message1 + " and longitude: " + Message2;
        Message = "Google Maps - https://www.google.co.in/maps/place/" + Message1 + "," + Message2;
        SMS.send(PhoneNumber,Message);
        delay(3000);
    }
  if (digitalRead(buttonPin1) == HIGH) { 
      digitalWrite(ledPin, HIGH);
        Phone.call(PhoneNumber);
        
    }
  if (digitalRead(buttonPin2) == HIGH) { 
      dtostrf(distance,4,1,charVal);
      digitalWrite(ledPin, HIGH);
      TextToSpeech.say("Target Distance is ");
      delay(1400); 
      TextToSpeech.say(charVal);
      delay(1300);
      TextToSpeech.say("centemeter");
      delay(2000);
    } 
    else {
      digitalWrite(ledPin, LOW);      
  }  
  delay(1000); //Small delay
  //Serial.println(distance1);
 //   delay(250); //pause to let things settle
}

------------------------------------------------------------------------------------------------------------

//GPS


#define FROM_ONESHEELD_LIBRARY
#include "OneSheeld.h"
#include "GPSShield.h"

//Class Constructor 
GPSShieldClass::GPSShieldClass () : ShieldParent(GPS_ID)
{
  LatValue=0;
  LonValue=0;
  isInit=false;
  isCallBackAssigned=false;
}

//GPS Input Data Processing 
void GPSShieldClass::processData ()
{
  //Checking Function-ID
  byte functionId=getOneSheeldInstance().getFunctionId();
  if(functionId==GPS_VALUE)
  {

    LatValue=getOneSheeldInstance().convertBytesToFloat(getOneSheeldInstance().getArgumentData(0));

    LonValue=getOneSheeldInstance().convertBytesToFloat(getOneSheeldInstance().getArgumentData(1));

    isInit=true;                    //setting a flag 
  }
  //Users Function Invoked
  if (isCallBackAssigned && !isInACallback())
  {
    enteringACallback();
    (*changeCallBack)(LatValue,LonValue);
    exitingACallback();
  }
}

//Getter 
float GPSShieldClass::getLatitude()
{
  return LatValue;
}

//Getter
float GPSShieldClass::getLongitude()
{
  return LonValue;
}

//Helper 
bool GPSShieldClass::isInRange(float usersValue1 , float usersValue2,float range)
{
  if(!isInit)return false;
  float x= getDistance(usersValue1 , usersValue2);
  if( x >=0 && x < range)
  {
      return true;    
  }
  else
  {
    return false;
  }

}
//Helper
float GPSShieldClass::getDistance(float x , float y)      //x and y is the lattitude and the longitude inserted by the user 
{
  if(!isInit)return 0;
  float dLat = radian(x-LatValue);      //difference betwwen the two lattitude point  
  float dLon = radian(y-LonValue);      //difference betwwen the two longitude point  

  float chordProcess    = sin(dLat/2)*sin(dLat/2)+sin(dLon/2)*sin(dLon/2)*cos(radian(LatValue))*cos(radian(x));
  float angularDistance = 2*atan2(sqrt(chordProcess),sqrt(1-chordProcess));
  float actualDsitance  = (RADIUS_OF_EARTH*angularDistance)*1000;     //getting the actuall distance in meters

  return actualDsitance;                      
}

//Helper
float GPSShieldClass::radian(float value)
{
  float radianValue = value*(PI/180);
  return radianValue;
}

//User Function Setter
void GPSShieldClass::setOnValueChange(void (*userFunction)(float lattitude ,float longitude))
{
  changeCallBack=userFunction;
  isCallBackAssigned=true;
}

----------------------------------------------------------------------------------------------------------

//TTS


#define FROM_ONESHEELD_LIBRARY
#include "OneSheeld.h"
#include "TTSShield.h"

//Give text to be said by the phone
void TTSShield::say(const char * text)
{
  //Check length of string 
  int textLength = strlen(text);
  if(!textLength) return;
  OneSheeld.sendShieldFrame(TTS_ID,0,TTS_SAY,1,new FunctionArg(textLength,(byte*)text));
}

void TTSShield::say(String text)
{
  int textLength = text.length();

  char cTypeText[textLength+1];

  for (int i = 0; i <textLength; i++)
  {
    cTypeText[i]=text[i];
  }
  cTypeText[textLength]='\0';

  say(cTypeText);
}

-------------------------------------------------------------------------------------------------------------

//Phone_call


#define FROM_ONESHEELD_LIBRARY
#include "OneSheeld.h"
#include "PhoneShield.h"

//Class Constructor
PhoneShieldClass::PhoneShieldClass() : ShieldParent(PHONE_ID)
{
  value=0;
  number=NULL;
  isCallBackAssigned=false;
  usedSetOnString=false;  
}
//Call Setter 
void PhoneShieldClass::call(const char* phone)
{
  //Check length of string 
  int phoneLength = strlen(phone);
  if(!phoneLength) return;
  OneSheeld.sendShieldFrame(PHONE_ID,0,PHONE_CALL,1,new FunctionArg(phoneLength,(byte *)phone));
}

void PhoneShieldClass::call(String phone)
{
  int phoneLength = phone.length();

  char cTypePhone[phoneLength+1];

  for(int i=0; i<phoneLength ;i++)
  {
    cTypePhone[i]=phone[i];
  }
  cTypePhone[phoneLength]='\0';

  call(cTypePhone);
}

//Ringing Checker 
bool PhoneShieldClass::isRinging()
{
  return !!value;
}
//Number Getter
char * PhoneShieldClass::getNumber()
{
  value=!!value;
  return number;
}

String PhoneShieldClass::getNumberAsString()
{
  String phoneNumberAsString(number);
  return phoneNumberAsString;
}

//Phone Input Data Processing 
void PhoneShieldClass::processData()
{
  //Checking Function-ID
  byte functionId= getOneSheeldInstance().getFunctionId();

  if (functionId==PHONE_IS_RINGING)
  {
    value =getOneSheeldInstance().getArgumentData(0)[0];
  }
  else if (functionId==PHONE_GET_NUMBER)
  {
    if(number!=0)
    {
      free(number);
    }
    
    byte length=getOneSheeldInstance().getArgumentLength(0);
    
    number=(char*)malloc(sizeof(char)*(length+1));
    
    for (int i=0; i< length;i++)
      {
        number[i]=getOneSheeldInstance().getArgumentData(0)[i];
      }

      number[length]='\0';
      //Users Function Invoked
      if(!isInACallback())
      {
        if (isCallBackAssigned)
        {
          enteringACallback();
          (*changeCallBack)(value,number);
          exitingACallback();
        }
        if(usedSetOnString)
        {
          enteringACallback();
          String phoneNumberAsString(number);
          (*changeCallBackString)(value,phoneNumberAsString);
          exitingACallback();
        }
      }
  }
}

//Users Function Setter
void PhoneShieldClass::setOnCallStatusChange(void (*userFunction)(bool isRinging,char  phoneNumber []))
{
  changeCallBack=userFunction;
  isCallBackAssigned=true;
}


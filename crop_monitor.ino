//Included libraries
#include "dht.h"    
#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(D1, D2);  //(Rx,Tx)
char msg;

// OLED display TWI address
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(-1);

#if (SSD1306_LCDHEIGHT != 64)  //128*64 resolution
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//Wifi connection
const char* ssid     = "Priyam";
const char* password = "priyam123456";
unsigned long myChannelNumber = 520428;
const char * myWriteAPIKey = "VEHYSNWFM74B2F3S";
const char * myReadAPIKey = "1Y6NE3E4X9ZOWHHK";

//Pins for different sensors
const int AOUTpin = A0; //the AOUT pin of the CO sensor goes into analog pin A0 of the arduino
//const int DOUTpin = D1; //the DOUT pin of the CO sensor goes into digital pin D1 of the arduino
const int smokeA0 = A0;
int buzzer = D0;
#define DHT11_PIN D3
#define AirQualitypin D7
#define SoilMoistPin D8
#define trigPin D5
#define echoPin D6
#define motor D1
#define selectline D4 
//#define relay 9
//Variables for storing the data read
int COlimit;
int COValue;
int smokeThresh = 400;
long duration;
int sensorValue;
int airQuality;
int soilMoist;
int smokeAmt;
int x = 10;  // x coordinate of OLED
int y = 1;   // y coordinate of OLED
String str;
String motor_status="initial";
WiFiClient WiFiclient;    // Object of WiFiClient
dht DHT11;    //Object of dht

void setup(){
  //WiFi setup
  
  Serial.begin(115200); 
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  ThingSpeak.begin(WiFiclient);

  //pinModes 
  pinMode(AirQualitypin,INPUT);
  pinMode(SoilMoistPin,INPUT);
  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  pinMode(AOUTpin,INPUT);
//  pinMode(DOUTpin,INPUT);//sets the pin as an input to the arduino
  pinMode(buzzer,OUTPUT);
  pinMode(smokeA0,INPUT);
  pinMode(motor, OUTPUT);
  digitalWrite(motor, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);    //OLED display             
  display.clearDisplay();                            // clear display if anything is already there
  display.display();                                 //actual action takes place

  // display a line of text
  display.setTextSize(2);
  display.setTextColor(WHITE);                     // if not there then the oled will be in off mode only
  display.setCursor(x,y);                          //this specifies the coordinate frm where to start writing
  // update display with all of the above graphics
  display.display();                                 // to set the above thing

}
 
void loop(){ 

 
  
  temp();
  humid();
  air_quality();
  soil_moisture();
  water_level();
  digitalWrite(selectline,HIGH);
   carbon_monoxide();
   digitalWrite(selectline,LOW);
    smoke();
  display.clearDisplay();
  display.display();

  motor_status= ThingSpeak.readFloatField(myChannelNumber, 8, myReadAPIKey);
  Serial.println("motor_status: "+motor_status);
  if(motor_status=="0")
     {
      digitalWrite(motor,LOW);
      Serial.println("motor_status: "+motor_status);
     }
  else if(motor_status=="1")
  {
    digitalWrite(motor,HIGH);
    Serial.println("motor_status: "+motor_status);
  }
  
  if(str.length()!=0)
//    SendMessage();
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
}

//Air Temperature
void temp()
{ 
  int chk = DHT11.read11(DHT11_PIN);
  if(chk==0){
    Serial.print("Temp: ");
    Serial.print((String)DHT11.temperature);         // Printing the temperature on display.
    Serial.println("*C");     // Printing “ *C ”  on display.
  }
  else{
    Serial.println("Error");
  }
    ThingSpeak.setField(1, (int)DHT11.temperature);
  

  if(DHT11.temperature < 25 || DHT11.temperature > 28)
  {
     Serial.println("Not the OPTIMUM TEMPERATURE.");
     str=str+" temperature not favourable for plants "; 
     display.print("Not the OPTIMUM TEMPERATURE.");
     display.display();
     y = y+3;
     display.setCursor(x,y);
     display.display();
  }
  delay(2000);
}

//Humidity
void humid()
{
  int chk = DHT11.read11(DHT11_PIN);
  if(chk==0){
    
    Serial.print("Relative Humidity: ");
    Serial.print((String)DHT11.humidity);     // Printing the humidity on display
    Serial.println("%");     // Printing “%” on display
   }  
  else{
    Serial.println("Error");
  }
  //DHT11.humidity;
  ThingSpeak.setField(2,(int)DHT11.humidity);
 // ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if(DHT11.humidity < 30 || DHT11.humidity > 35)
  {
     Serial.println("Not the OPTIMUM HUMIDITY.");
     str=str+" humidity not favourable for the crops ";
     display.print("Not the OPTIMUM HUMIDITY.");
     display.display();
     y = y+3;
     display.setCursor(x,y);
     display.display(); 
  }
  delay(2000);
}

//MQ135
void air_quality()
{
  airQuality = digitalRead(AirQualitypin);//prior it was analog
  Serial.print("Air Quality = ");
  Serial.print(airQuality);
  Serial.print("PPM");
  Serial.println();
  ThingSpeak.setField(3, airQuality);
  

  if(airQuality < 100)
  {
    Serial.println("Suitable for crop growth.");
  }
  else if(airQuality>101 && airQuality < 200)
  {
    Serial.println("Unhealthy for crop growth.");
    display.print("Unhealthy for crop growth");
    display.display();
    y = y+3;
    display.setCursor(x,y);
    display.display(); 
  }
  else
  {
    Serial.println("Hazardous for crop growth.");
    str=str+" bad air quality ";
    display.print("Hazardous for crop growth");
    display.display();
    y = y+3;
    display.setCursor(x,y);
    display.display();
  }
  delay(2000);
}

//Soil moisture 
void soil_moisture()
{
  soilMoist = digitalRead(SoilMoistPin);
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoist);
  Serial.print("%");
  ThingSpeak.setField(4,soilMoist);
  //ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if(soilMoist < 15)
  {
    Serial.println("Soil moisture too low");
    str=str+"soil moisture too low ";
    digitalWrite(motor,HIGH);
    display.print("Soil moisture too low");
    display.display();
    y = y+3;
    display.setCursor(x,y);
    display.display();
  }
  else if(soilMoist>16 && soilMoist < 60)
  {
     Serial.println("Soil moisture in range");
     //str=str+"soil moisture too low ";
     digitalWrite(motor,LOW);
  }
  else
  {
    Serial.println("Soil moisture too high");
    str=str+"soil moisture too high ";
    digitalWrite(motor,LOW);
    display.print("Soil moisture too high");
    display.display();
    y = y+3;
    display.setCursor(x,y);
    display.display();
  }
  delay(2000);
}

//Ultrasonic sensor
void water_level()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  sensorValue = (duration * 0.034)/2;
  sensorValue = 10 - sensorValue;
  
  Serial.print("WATER LEVEL: ");
  Serial.print(sensorValue);
  Serial.println("cm");
  ThingSpeak.setField(5, sensorValue);
  //ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if(sensorValue < 8)
  {
    Serial.println("High Water Level");
    str=str+"high water level ";
    digitalWrite(motor,LOW); 
    display.print("High water level");
    display.display();
    y = y+3;
    display.setCursor(x,y);
    display.display();    
  }
  else
  {
    Serial.println("Water level in range");
    digitalWrite(motor,HIGH);
  }
  delay(2000);
}

//MQ7
void carbon_monoxide()
{
  COValue = analogRead(AOUTpin);//reads the analaog value from the CO sensor's AOUT pin
  //COlimit = digitalRead(DOUTpin);//reads the digital value from the CO sensor's DOUT pin
  Serial.print("CO value: ");
  Serial.println(COValue);//prints the CO value
  //Serial.print("Limit: ");
  //Serial.println(COlimit);//prints the limit reached as either LOW or HIGH (above or underneath)
  ThingSpeak.setField(6, COValue);
  //ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
   
     if(COValue>300)
     {
       digitalWrite(buzzer,LOW);
       Serial.println("CO level high");
       display.print("CO level high");
       str=str+"Carbon Monoxide high";
       display.display();
       y = y+3;
       display.setCursor(x,y);
       display.display();
   }
  else
  { digitalWrite(buzzer,HIGH);
    Serial.println("CO level OK");
  }
  delay(2000);
}

//MQ2
void smoke()
{
  smokeAmt = analogRead(smokeA0);
  Serial.print("Smoke: ");
  Serial.println(smokeAmt);
  ThingSpeak.setField(7, smokeAmt);
  //ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (smokeAmt > smokeThresh)
  {
    Serial.println("Smoke level high");
    digitalWrite(buzzer,HIGH);
    str=str+"smoke very  high";
    display.print("Smoke level high");
    display.display();
  }
  else
  {
    digitalWrite(buzzer,LOW);
  }
  delay(5000);
}

/*void SendMessage()
{
  Serial.println("send msg");
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+919312264227\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println("ALERT:  "+str);// The SMS text you want to send
  delay(100);
   mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
  Serial.println("msg sent");
}*/

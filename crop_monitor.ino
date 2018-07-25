//Included libraries
#include <dht.h>   
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include<Wire.h>

//Wifi connection
const char* ssid     = "Priyam";
const char* password = "priyam123456";
unsigned long myChannelNumber = 520428;
const char * myWriteAPIKey = "VEHYSNWFM74B2F3S";
const char * myReadAPIKey = "1Y6NE3E4X9ZOWHHK";

//Pins for different sensors
//#define AOUTpin A0 //the AOUT pin of the CO sensor goes into analog pin A0 of the arduino
//#define smokeA0  A1
int buzzer = 16;
int DHT11_PIN =5;
int AirQualitypin =4;
int SoilMoistPin= 14;
int trigPin= 12;
int echoPin= 13;
int selectline= 10 ;
int motor = 9;
//Variables for storing the data read
int COlimit;
int COValue;
int smokeThresh = 400;
long duration;
int sensorValue;
int airQuality;
int soilMoist;
int smokeAmt;
String motor_status="initial";
//int x = 10;  // x coordinate of OLED
//int y = 1;   // y coordinate of OLED
//String str = "NULL";

WiFiClient WiFiclient;    // Object of WiFiClient
dht DHT11;    //Object of dht

void temp();
void humid();
void air_quality();
void soil_moisture();
void water_level();
void carbon_monoxide();
void smoke();



void setup(){
  //WiFi setup
  Serial.begin(115200); 
  Serial.println("A");
  delay(10);
  Serial.println();
  Serial.println();
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
  delay(1000);
Serial.println("B");
  //pinModes 
  pinMode(AirQualitypin,INPUT); Serial.println("C");
  pinMode(SoilMoistPin,INPUT);Serial.println("D");
  pinMode(trigPin,OUTPUT);Serial.println("E");
  pinMode(echoPin,INPUT);Serial.println("F");
 // pinMode(AOUTpin,INPUT);Serial.println("G");
//  pinMode(DOUTpin,INPUT);//sets the pin as an input to the arduino
  pinMode(buzzer,OUTPUT);Serial.println("H");
//  pinMode(smokeA0,INPUT);Serial.println("I");
  pinMode(motor, INPUT);Serial.println("J");
  digitalWrite(motor, LOW);Serial.println("K");
}
 
void loop()
{ 
 Serial.println("L");
  temp();
  Serial.println("M");
  delay(1000);
  humid();Serial.println("N");
  delay(1000);
  air_quality();Serial.println("O");
  delay(1000);
  soil_moisture();Serial.println("P");
  delay(1000);
  water_level();Serial.println("Q");
  delay(1000);
  digitalWrite(selectline,HIGH);
   carbon_monoxide();Serial.println("R");
   digitalWrite(selectline,LOW);
   delay(1000);
    smoke();Serial.println("S");
    delay(1000);
//  display.clearDisplay();
//  display.display();

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
    Serial.println("motor_status"+motor_status);
  }
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
     ThingSpeak.setField(1, (int)DHT11.temperature);
  }
  else{
    Serial.println("Error");
  }
   
  if(DHT11.temperature < 25 || DHT11.temperature > 28)
  {
     Serial.println("Not the OPTIMUM TEMPERATURE.");
  }
  
}

//Humidity
void humid()
{
  int chk = DHT11.read11(DHT11_PIN);
  if(chk==0){
    
    Serial.print("Relative Humidity: ");
    Serial.print((String)DHT11.humidity);     // Printing the humidity on display
    Serial.println("%");     // Printing “%” on display
    ThingSpeak.setField(2,(int)DHT11.humidity);
   }  
  else{
    Serial.println("Error");
  }


  if(DHT11.humidity < 30 || DHT11.humidity > 35)
  {
     Serial.println("Not the OPTIMUM HUMIDITY.");

  }
  
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

  }
  else
  {
    Serial.println("Hazardous for crop growth.");

  }
 
}

//Soil moisture 
void soil_moisture()
{
  soilMoist = digitalRead(SoilMoistPin);
  Serial.print("Soil Moisture: ");
  Serial.print(soilMoist);
  Serial.print("%");
  ThingSpeak.setField(4,soilMoist);

  if(soilMoist < 15)
  {
    Serial.println("Soil moisture too low");
   
    digitalWrite(motor,HIGH);

  }
  else if(soilMoist>16 && soilMoist < 60)
  {
     Serial.println("Soil moisture in range");
    
     digitalWrite(motor,LOW);
  }
  else
  {
    Serial.println("Soil moisture too high");

  }
 
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


  if(sensorValue < 8)
  {
    Serial.println("High Water Level");
    
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
//  COValue = analogRead(AOUTpin);//reads the analaog value from the CO sensor's AOUT pin
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

   }
  else
  { digitalWrite(buzzer,HIGH);
    Serial.println("CO level OK");
  }
  
}

//MQ2
void smoke()
{
//  smokeAmt = analogRead(smokeA0);
  Serial.print("Smoke: ");
  Serial.println(smokeAmt);
  ThingSpeak.setField(7, smokeAmt);
  //ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (smokeAmt > smokeThresh)
  {
    Serial.println("Smoke level high");
    digitalWrite(buzzer,HIGH);

  }
  else
  {
    digitalWrite(buzzer,LOW);
  }
  
}

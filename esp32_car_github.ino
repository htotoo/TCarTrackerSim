
#include <string.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include "config.h"


#include <WebServer.h>
WebServer server(80);

#include <ArduinoHttpClient.h>

#define BAT_ADC     35
#define PWR_PIN     4

#define TINY_GSM_MODEM_SIM7000
#define SerialAT Serial1
#define SerialMon Serial
#define TINY_GSM_DEBUG SerialMon
#include <TinyGsmClient.h>

#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
TinyGsmClient client(modem);


float battery = 0.0; 
String smsMessages = "";

WiFiClient net;

byte lastSuccCode = 0;  //succ posting coords by. 2 =  gprs, 1 = wifi
int submitNum = 0; //how many submits happened
int nowYear, nowMonth, nowDay, nowHour, nowMinute, nowSecond;

float gpsLatitude = 0, gpsLongitude = 0, gpsSpeed = 0,  gpsAltitude = 0, gpsAccuracy = 0;
char gpsIsValid = 'N';  int gpsUSat = 0;

long lastMillisGpsReport = 0;
long lastMillisGpsGet = 0;
long lastMillisModemUsed = 0;

bool wakeUpByTimer = false;
bool wasSuccessGpsSignalSubmited = false;

float ReadBattery() //specific to ttgo T-SIM7000G
{
    int vref = 1100;  uint16_t volt = analogRead(BAT_ADC);   float battery_voltage = ((float)volt / 4095.0) * 2.0 * 3.3 * (vref);   return battery_voltage;
}


#include "accel.h"
#include "wific.h"
#include "oled.h"
#include "energy.h"

int incomingByte = 0; 

void modemRestart()
{
    modemPowerOff();
    delay(1000);
    modemPowerOn();
}

void modemStart()
{
  lastMillisModemUsed = millis();
  if (modem.isGprsConnected()) 
  {
    return;
  }
  modem.gprsConnect(MOBILEAPN, "", "");
}

void modemStop()
{
  modem.gprsDisconnect();
  lastMillisModemUsed = 0;
}

void InitModem()
{
  SerialAT.begin(9600, SERIAL_8N1, 26, 27);
  modemPowerOn();
  if (!modem.testAT()) {
        SerialMon.println("Failed to start modem, attempting to restart");
        modemRestart();
        return;
  }
  modemStop(); //after turn on, disconnect from prev gprs if exists
  modem.sendAT(GF("+CMNB=3"));
  String res = "";
  modem.waitResponse(10000L, res);
}

void ReadSMSes()
{
  modem.sendAT(GF("+CMGF=1"));
  String res = "";
  modem.waitResponse(10000L, res);
  res = "";
  modem.sendAT(GF("+CMGL=\"ALL\""));
  modem.waitResponse(10000L, smsMessages);
}

void UpdateGps()
{
  int vsat;
  modem.getGPS(&gpsLatitude, &gpsLongitude, &gpsSpeed, &gpsAltitude, &vsat, &gpsUSat, &gpsAccuracy, &nowYear, &nowMonth, &nowDay, &nowHour, &nowMinute, &nowSecond);
  gpsIsValid = (gpsUSat>2)?'I':'N';
}


void setup() {
  Serial.begin(115200);
  wasSuccessGpsSignalSubmited  = false;
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.println("ESP_SLEEP_WAKEUP_TIMER");
    wakeUpByTimer = true;
  }
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) Serial.println("ESP_SLEEP_WAKEUP_EXT0");
  if (wakeup_reason == ESP_SLEEP_WAKEUP_ULP) Serial.println("ESP_SLEEP_WAKEUP_ULP");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  InitDisplay();
  InitEnergySave();
  InitWifi();
  InitWebServer();
  if (!InitAccel()) Serial.println("Accel init failed");

  InitModem();
  GpsOn();
  digitalWrite(LED_BUILTIN, HIGH);
}


void submitGps()
{
  int succ = 0;
  if (gpsIsValid != 'I')
  {
    //try to get mobile position
      modemStart();
      Serial.write("Query modem loc\n");
      float lat      = 0, lon      = 0, accuracy = 0;
      int   year     = 0, month    = 0, day      = 0, hour     = 0, min      = 0, sec = 0;
      if (! modem.getGsmLocation(&lon, &lat, &accuracy, &year, &month, &day, &hour, &min, &sec) ) //needed to swap lat, lon
      {
        if (modem.getGsmLocation(&lon, &lat, &accuracy, &year, &month, &day, &hour, &min, &sec))
        {
          Serial.write("Latitude:"); Serial.write( String(lat, 8).c_str()); Serial.write( "\tLongitude:"); Serial.write(String(lon, 8).c_str());
          Serial.write("\nEND.\n");
          gpsLatitude = lat;      gpsLongitude = lon;      gpsIsValid = 'M';      gpsSpeed = 0;
          gpsAltitude = 0;      gpsAccuracy = accuracy;      gpsUSat = 0;
        }
      }
      else
      {
          Serial.write("Latitude:"); Serial.write( String(lat, 8).c_str()); Serial.write( "\tLongitude:"); Serial.write(String(lon, 8).c_str());
          Serial.write("\nEND.\n");
          gpsLatitude = lat;      gpsLongitude = lon;      gpsIsValid = 'M';      gpsSpeed = 0;
          gpsAltitude = 0;      gpsAccuracy = accuracy;      gpsUSat = 0;
      }
  }
  int vt = (wakeUpByTimer)?1:0;
  int wasAccelEventAnytime = (accelLastHitReal == 0)?0:1;
  int rion = (lastMillisModemUsed==0) ?0:1;
  String urii = String(PHPSITE) + "?rion="+String(rion)+"&anyaccel="+String(wasAccelEventAnytime)+"&vt="+String(vt)+"&alh="+String(AccelLastHitSec())+"&la="+String(gpsLatitude,6)+"&lo="+ String(gpsLongitude,6) +"&v="+String(gpsIsValid)+"&s=" + String(gpsSpeed) + "&a=" + String(gpsAltitude) + "&ac=" + String(gpsAccuracy) + "&sn=" + String(gpsUSat) + "&ba=" + String(battery, 4);
  if (SWifi::IsConnected()) //httpclient when wifi
  {
    SerialMon.write(urii.c_str());
    SerialMon.write("\n");
    HttpClient httpclient = HttpClient(net, PHPHOST, 80);
    if (httpclient.get(urii)==0){
      succ = 1;
      if (submitNum == 1 && smsMessages.length()>0)
      {
        Serial.println("Sending sms-es to php");
        //submit sms-es
        HttpClient httpclient2 = HttpClient(net, PHPHOST, 80);
        if (httpclient2.post(PHPSMSSITE, "text/plain", smsMessages) == 0)
        {
          Serial.println("Sent. Deleting sms-es..");
          //delete all sms-es          //AT+CMGD=1,4 --deletes all
          modem.sendAT(GF("+CMGD=1,4"));
          delay(100);
        }
      }
    }
  }
  if (succ == 0)
  {
    modemStart();
    HttpClient httpclient = HttpClient(client, "creativo.hu", 80);
    succ = (httpclient.get(urii) == 0)?2:0;
  }
  if (succ>0)
  {
    String ts = "SUBMIT SUCCES. "+String(succ)+"\n";
    SerialMon.write(ts.c_str());
    submitNum++;    
    if (gpsLatitude != 0 && gpsLongitude != 0)
    {
      wasSuccessGpsSignalSubmited = true;
    }
  }
  else SerialMon.write("SUBMIT FAILED\n");
  lastSuccCode = succ;
}



void loop() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();
    if (incomingByte == 'A')
    {
      if (modem.isGprsConnected()) Serial.println("GPRS CONNECTED"); else Serial.println("GPRS OFFLINE");
    }
    if (incomingByte == 'B')
    {
      modemStop();
    }
    if (incomingByte == 'C')
    {
        modem.sendAT(GF("+CPSI?"));
        String res = "";
        modem.waitResponse(10000L, res);
        Serial.println(res);
    }
  }
  SWifi::Loop();
  
  server.handleClient(); //webserver
  if (millis() - lastMillisGpsGet > 5000) {
        lastMillisGpsGet = millis();
        UpdateGps();
        if (!modem.testAT())
        {
          modemRestart();
        }
        PrintDisplay();
        AccelResetEvent();
        battery = ReadBattery();
   } 
   if (millis() - lastMillisGpsReport > (REPORTINTERVAL*1000)) {
        lastMillisGpsReport = millis();
        submitGps();
        if (submitNum == 1 && lastSuccCode >0)
        {
          ReadSMSes();
        }
   } 

   if (lastMillisModemUsed!= 0 && millis() - lastMillisModemUsed > (GPRSOFFTIME*1000))
   {
      modemStop();
   }

  if (AccelTimeOuted() || (wakeUpByTimer && wasSuccessGpsSignalSubmited))
  {
      accelLastHitReal = 0;
      //manage poweroff
      Serial.println("ACCEL ENDED!");
      detachInterrupt(digitalPinToInterrupt(ACCEL_IRQPIN)); //skip stuff
      PrepareSleep();
      SleepMe();
  }
  if (AccelIsInAlert()) AccelResetEvent();

}

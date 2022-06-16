
void InitEnergySave() //after power on, turn off stuff
{
  btStop();  
}


void SleepMe()
{
  SerialMon.write("Sleeping...\n");
  delay(300);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 1);
  esp_sleep_enable_timer_wakeup(3600000000*3);
  esp_deep_sleep_start();

}


void GpsOn()
{
  modem.sendAT("+SGPIO=0,4,1,1");
  modem.enableGPS();
  modem.sendAT(GF("+CGNSPWR=1"));
  lastMillisGpsGet = millis();
}

void GpsOff()
{
  modem.sendAT("+SGPIO=0,4,1,0");
  modem.sendAT(GF("+CGNSPWR=0"));
  modem.disableGPS();
}

void modemPowerOn()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1000);
    digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff()
{
    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, LOW);
    delay(1500);
    digitalWrite(PWR_PIN, HIGH);
}



void PrepareSleep()
{
  SerialMon.write("Prepare sleep.\n");
  DisplayOff();
  //todo put everything in prepare sleep mode
  GpsOff();
  modemPowerOff();
  modem.poweroff();
  delay(1000);  
}


#ifdef usedisplay
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  #define OLED_RESET 5
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 32 // OLED display height, in pixels
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

void InitDisplay()
{
#ifdef usedisplay
  pinMode(34, OUTPUT);
  digitalWrite(34, HIGH);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.display();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Booting...");
  display.display();
#endif   
}

void PrintDisplay()
{
#ifdef usedisplay
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("G: ");
    display.print(gpsIsValid);
    display.print(" #");
    display.print(gpsUSat);
    display.print(" #");
    display.print(battery);
    display.setCursor(0,12);
    display.print( SWifi::IsConnected() ? "Wifi: OK ": "Wifi: DC " );
    display.print( lastMillisModemUsed==0 ? "R: - ": "R: + " );
    display.setCursor(0,24);
    display.print("LS: ");
    display.print(String(lastSuccCode).c_str());
    display.print(" LA: ");
    display.print(String(AccelLastHitSec()));
    
    display.display(); 
#endif
} 

void DisplayOff()
{
#ifdef usedisplay
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  digitalWrite(34, LOW);
#endif
}

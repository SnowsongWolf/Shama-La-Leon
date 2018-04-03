#include <Adafruit_LiquidCrystal.h>
#include <OctoWS2811.h>
#include <Wire.h>

#define BTNPIN    1
#define BATPIN    A1
#define TEMPPIN   A2
#define S_LED     11
#define S_FAN     12

#define FANLOW    80
#define FANHIGH   90

Adafruit_LiquidCrystal lcd(0);

const int ledsPerStrip = 500;   // Simulate full suit, we'll only have 450 total on channel 0

DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

byte curMode = 0;       // 0 off, 1 run, 2 stop
byte curFan = false;      // Current fan state, for doing fuzzy logic

void setup() {
  // put your setup code here, to run once:
  pinMode(BTNPIN, INPUT_PULLUP);
  pinMode(BATPIN, INPUT);
  pinMode(TEMPPIN, INPUT);

  pinMode(S_LED, OUTPUT);
  pinMode(S_FAN, OUTPUT);

  digitalWrite(S_LED, LOW);
  digitalWrite(S_FAN, LOW);

  leds.begin();

  lcd.begin(16, 2);
  lcd.setBacklight(HIGH);
  
  lcd.setCursor(0, 0);
  lcd.print("LEDs ");     // Label for LED state

  lcd.setCursor(9, 0);
  lcd.print("Fan OFF");   // Defaulting fan to off for now
  
  lcd.setCursor(4, 1);
  lcd.print((char)223);   // Label for temperature display
  lcd.print('F');

  lcd.setCursor(9, 1);
  lcd.print('v');
}

void loop() {
  int curTemp = atoF(analogRead(TEMPPIN));
  float curBat = atoV(analogRead(BATPIN));
  switch (curMode) {
    case 0:     // LEDs off, just do temperature
      lcd.setCursor(5, 0);
      lcd.print("OFF");
      for (int i = 0; i < 150; i++) {
        leds.setPixel(i, 0x000000);
      }
      leds.show();
      break;

    case 1:
      lcd.setCursor(5, 0);
      lcd.print("RUN");
      digitalWrite(S_LED, HIGH);
      for (int i = 0; i < 150; i++) {
        leds.setPixel(i, 0x00FFFF);
        leds.show();
      }
      curMode = 2;
      break;

    default:
      lcd.setCursor(5, 0);
      lcd.print("ON ");
      break;
  }

  if (curFan) {
    if (curTemp <= FANLOW) {
      curFan = false;
      digitalWrite(S_FAN, LOW);
      lcd.setCursor(9, 0);
      lcd.print("Fan OFF");   // Defaulting fan to off for now
    }
  }else
    if (curTemp >= FANHIGH) {
      curFan = true;
      digitalWrite(S_FAN, HIGH);
      lcd.setCursor(9, 0);
      lcd.print("Fan ON ");   // Defaulting fan to off for now
    }
  
  // print temperature value
  lcd.setCursor(0, 1);    // character 0 line 1
  lcd.print("   ");
  lcd.setCursor(0, 1);    // character 0 line 1
  lcd.print(curTemp);
  //lcd.print(analogRead(TEMPPIN));

  // print raw analog read value
  /*lcd.setCursor(8, 1);    // character 8 line 1
  lcd.print("    ");
  lcd.setCursor(8, 1);    // character 8 line 1
  lcd.print(analogRead(TEMPPIN));*/
  lcd.setCursor(8, 1);
  lcd.print("   ");
  lcd.setCursor(8, 1);
  lcd.print(curBat);
  delay(250);

  if (!digitalRead(BTNPIN)) {
    delay(50);
    curMode ++;
    if (curMode > 2) curMode = 0;
  }
}

int atoF (int aRead) {
  //return aRead * 297 / 512 - 58;  // convert analog reading to Farenheit
  return (aRead * 58 - 4555) / 100;
}

float atoV (int aRead) {
  return map(aRead, 0, 890, 0, 360) / 100.0f; // convert analog reading to average battery cell voltage
}

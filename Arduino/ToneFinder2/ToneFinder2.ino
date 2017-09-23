#include <Audio.h>

AudioInputI2S           memsIn;       // Input for the MEMS microphone
AudioAnalyzeToneDetect  twentyK;      // Tone detect for the 20 kHz tone

AudioConnection patch01(memsIn, 0, twentyK, 0); // Tie audio input to tone detection

#define THRESHHOLD 0.01               // Volume level to trigger ON state
#define DEBOUNCE 250                  // Amount of time in ms tone must be off to register OFF state
uint16_t SONGDELAY = 425;             // ms delay between falling edge and start of song

enum sMode {
  S_ON,
  S_OFF,
  S_FALLING
};

sMode mode = S_OFF;

//uint8_t tState1 = LOW;                // Variable to store last tone state

uint32_t lastHardState = 0;               // Variable to hold milliseconds since last change
uint32_t lastSoftState = 0;
uint32_t songStart = 0;
bool song = false;

void setup() {
  // put your setup code here, to run once:
  AudioMemory(12);                    // Audio memory allocation

  while (!Serial) ;
  delay (100);

  twentyK.frequency(20000, 30);       // Set frequency monitor to 20 kHz

}

void loop() {
  static String inString = "";
  // put your main code here, to run repeatedly:
  float t1 = twentyK.read();          // Read signal strength of the tone
  uint8_t tState = LOW;              // Initialize current tone state variable

  if (t1 > THRESHHOLD) tState = HIGH;

  /*if ((tState1 != tState2) && (millis() - lastState > DEBOUNCE)) {           // Edge detection
    if (tState2 == HIGH) {            // Rising edge
      Serial.print("Tone On:  ");
    } else {                          // If not rising edge, falling edge
      Serial.print("Tone off: ");
    }
    Serial.println(millis() - lastState);
    tState1 = tState2;                // Change last 
    lastState = millis();
  }*/

  if (tState == HIGH)
    mode = S_ON;
  else if (mode == S_ON) {
    mode = S_FALLING;
    lastSoftState = millis();
  }else if ((mode == S_FALLING) && (millis() > lastSoftState + DEBOUNCE)) {
    mode = S_OFF;
    lastHardState = lastSoftState;
    lastSoftState = millis();
    //Serial.print("Falling edge at ");
    //Serial.print(lastHardState);
    //Serial.println("ms.");
    song = true;
    songStart = lastHardState + SONGDELAY;
  }

  if ((song) && (millis() >= songStart)) {
    song = false;
    Serial.println("Music!"); 
  }

  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (isDigit(inChar)) {
      inString += (char)inChar;
    }

    if (inChar == '\n') {
      Serial.print("Delay changed to ");
      Serial.println(inString);
      SONGDELAY = inString.toInt();
    }
  }

  //if (t1 >= 0.01) Serial.println(t1); else Serial.println('0.00');
  //delay();
  
}

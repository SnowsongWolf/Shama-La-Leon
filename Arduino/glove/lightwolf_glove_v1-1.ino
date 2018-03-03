// Lightwolf Glove v1.0

#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <printf.h>

AudioInputI2S           micIn;                  // I2S Microphone object
AudioAnalyzeToneDetect  td20;                   // Tone detection object
AudioConnection patchCord01(micIn, 0, td20, 0); // Link micIn to td20

#define TS 0                                    // Timescale, used to calibrate millis() function

const byte leadTime = 2;                        // lead time between chirp and music
const byte bpm = 108;                           // bpm of the song
const byte flashlen = 100;                      // millisecond length of timing flash

byte songOn = false;                            // Is a song currently playing?
uint32_t nextFlash;                              // millis location of next flash

#define RDELAY 5
#define ADDR1 "arf01"
#define ADDR2 "arf02"

RF24 radio(7, 8);
const byte address[][6] = {ADDR1, ADDR2};

void setup() {
  // put your setup code here, to run once:

  
  SPI.begin(); // Start the SPI buss
  SPI.setSCK(14);                               // Use alternate SPI clock pin 14
  //SPI.setClockDivider( SPI_CLOCK_DIV16 );       // Set SPI clock to 4.5MHz,

  
  AudioMemory(12);                              // Audio processing overhead memory
  Serial.begin(115200);                         // Talkback
  delay(250);
  td20.frequency(20000, 50);                    // Set target frequency and envelope

  pinMode(16, OUTPUT);
  pinMode(15, OUTPUT);

  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);

  digitalWrite(16, HIGH);
  digitalWrite(15, HIGH);

  Serial.println("Stage 1 complete");

  delay(1000);
  digitalWrite(16, LOW);
  delay(1000);

  radio.begin();
  radio.setChannel(108);
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.setPALevel(RF24_PA_LOW);
  Serial.println("Stage 2 complete");
  char test[] = "Hello!";
  //radio.printDetails();
  radio.startListening();
  radio.stopListening();
  radio.write(&test, strlen(test));
  radio.startListening();
  Serial.println("Setup phase complete");
  digitalWrite(16, HIGH);
  delay(250);

  if (radio.isChipConnected()) Serial.println("Radio is connected"); else Serial.println("Radio is not connected");
}

void loop() {
  // put your main code here, to run repeatedly:
  float ct;                                     // click track variable for holding the sound level of td20
  //static byte lState = HIGH;

  //if (lState == HIGH) lState = LOW; else lState = HIGH;
  //digitalWrite(16, LOW);

  if (radio.available()) {
    digitalWrite(16, LOW);
    delay(500);
    digitalWrite(16, HIGH);
    delay(500);
    char junk[32] = "";
    radio.read(&junk, radio.getDynamicPayloadSize());
    Serial.write(junk);
    Serial.write('\n');
  }

  /*char test[] = "Hello!";
  radio.stopListening();
  radio.write(&test, sizeof(test));
  radio.startListening();
  delay(2000);*/

  if (!songOn) {
    //digitalWrite(15, LOW);
    ct = td20.read();                             // check sound level
    ct *= 1000;
    if (ct > 0) {
      Serial.println(ct);                         // output sound level numbers
      songOn = true;
      nextFlash = millis() + leadTime * (1000 + TS);
      Serial.print("millis: ");
      Serial.print(millis());
      Serial.print(" | next pulse: ");
      Serial.println(nextFlash);
      digitalWrite(15, HIGH);
    }
  }else
  {
    if (millis() >= nextFlash) {
      if (millis() >= nextFlash + flashlen) {
        digitalWrite(16, HIGH);
        nextFlash += 60 * (1000 + TS) / bpm;
        Serial.print("millis: ");
        Serial.print(millis());
        Serial.print(" | next pulse: ");
        Serial.println(nextFlash);
      }else digitalWrite(16, LOW);
      
    }
  }
}

#include <FAB_LED.h>
#include <SPI.h>
#include <SD.h>
//#include <Wire.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6
#define MAX_FILES 50
#define MAX_LEDS 140
#define FRAMEDELAY 33

ws2812b<D,6> strip;
grb pixels[MAX_LEDS] = {};

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, PIN, NEO_GRB + NEO_KHZ800);
int nFiles = 0, currentFile = 0;
char *filename[MAX_FILES];
File root;
//uint32_t color = strip.Color(245,73,195);
uint8_t rgbBuf[MAX_LEDS * 3];

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

int cmp(const void *a, const void *b) {
  return strcmp(*(char **)a, *(char **)b);
}

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code

  
  char *name, header[3];
  int len;
  uint8_t i;
  File entry;

  strip.clear(2 * MAX_LEDS);
  strip.sendPixels(MAX_LEDS, pixels); // Initialize all pixels to 'off'
  Serial.begin(115200);
  Serial.println("OPC SD card streamer, NeoPixel version");
  delay(500);

  Serial.println("SD setup... ");
  if(!SD.begin(4)) {
    Serial.println("failed!");
    for(;;);
  }
  Serial.println("succeeded!");
  delay(500);

 for(root = SD.open("/"); (nFiles < MAX_FILES) &&
   (entry = root.openNextFile()); entry.close()) {
    if(entry.isDirectory()) continue;
    name = entry.name();
    if(name[0] == '.') continue; // Skip hidden files
    len = strlen(name);
    // Skip files that don't end in ".opc" or don't contain magic header
    if((len < 4) || strcasecmp(&name[len-4], ".opc") ||
       (entry.read(header, 3) < 3) || strncmp(header, "OPC", 3)) continue;
    if(name = strdup(name)) filename[nFiles++] = name; // Add to list
  }
  
  qsort(filename, nFiles, sizeof(char *), cmp);
  Serial.print(nFiles);
  Serial.println(" valid OPC files found:");
  for (i = 0; i < nFiles; i++) {
    Serial.print("  ");
    Serial.println(filename[i]);
  }
}

#define MODE_DATA    0
#define MODE_HEADER  1
#define MODE_DISCARD 2
uint8_t mode = MODE_HEADER;
uint32_t frameno = 0;

uint32_t firstframe, mil;
uint32_t s;
uint16_t m;

// bytesToRead is the number of bytes remaining in current mode, while
// bytesRead is the number read so far (used as an index into a destination
// buffer).  bytesToDiscard is the number remaining when in MODE_DISCARD.

void loop() {
  File file;

  Serial.println(filename[currentFile]);
  delay(2000);
  if (file = SD.open(filename[currentFile])) {
  //if (file = SD.open("8bittrip.OPC")) {
    int16_t a, dataSize;
    //uint8_t w;
    byte d[4];
    uint16_t curpos;
    //uint8_t dataSize;
    frameno = 0;
    file.seek(4);
    while (file.available()) {
      if (Serial.available()) {
        char in = Serial.read();
        if (in = '+') break;
      }
      
      curpos = file.position();
      a = file.read(&d, sizeof(d));
      dataSize = (d[2] << 8) | d[3];
      Serial.print("Position: ");
      Serial.print(curpos);
      Serial.print('\t');
      
      Serial.print("Channel: ");
      Serial.print((int)d[0]);
      Serial.print('\t');
      
      Serial.print("Command: ");
      Serial.print((int)d[1]);
      Serial.print('\t');
      
      Serial.print("Data length: ");
      Serial.print(dataSize);
      Serial.print('\t');

      if ((d[0] <=1) && (d[1] == 0)) {
        uint8_t dly = 0;

        for (int i = 0; i < 140; i++) {
          byte r, g, b;
          file.read(&r, sizeof(r));
          file.read(&g, sizeof(g));
          file.read(&b, sizeof(b));
          pixels[i].r = r;
          pixels[i].g = g;
          pixels[i].b = b;
        }
        
        if (frameno > 0) {
          while (millis() < firstframe + (uint32_t)(FRAMEDELAY * frameno)) {
            delay(1);
          }
        } else firstframe = millis();

        strip.sendPixels(MAX_LEDS, pixels);
        //lastframe = millis();
        Serial.print(frameno);
        Serial.print('\t');
        mil = millis() - firstframe;
        s = (uint32_t)(mil / 1000);
        m = (uint16_t)(s / 60);
        mil -= (uint32_t)(s * 1000);
        s -= m * 60;

        if (m < 10) Serial.print('0');
        Serial.print(m);
        Serial.print(':');

        if (s < 10) Serial.print('0');
        Serial.print(s);
        Serial.print('.');

        if (mil < 100) Serial.print('0');
        if (mil < 10) Serial.print('0');
        Serial.println(mil);
        
        //Serial.println("fps");
        frameno ++;
      } else {
        Serial.print("Seek to ");
        Serial.println(dataSize + curpos + 4);
        file.seek(dataSize + curpos + 4);
        //delay(2000);
      }
    }
  }
  Serial.println("End of line.");
  file.close();
  currentFile++;
  if (currentFile >= nFiles) currentFile = 0;
}


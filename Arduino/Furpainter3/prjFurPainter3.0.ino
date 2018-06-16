#include <SPI.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <OctoWS2811.h>

#define LED_CH      500   // 500 pixels per channel
#define MAX_FILES   50    // 50 files to load from SD card
#define FRAMEDELAY  40    // Millisecond delay between frames

const int ledsPerStrip = LED_CH * 8;
DMAMEM int displayMemory[ledsPerStrip * 6];
int drawingMemory[ledsPerStrip*6];
elapsedMicros elapsedSinceLastFrame = 0;
bool playing = false;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, WS2811_800kHz);
SdFatSdioEX Sd;

int nFiles = 0, currentFile = 0;
char *filename[MAX_FILES];
File root;

int cmp(const void *a, const void *b) {
  return strcmp(*(char **)a, *(char **)b);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }
  
  char *name, header[3], tmp[80];
  int len, nameSize = 13;
  uint8_t i;
  File entry;

  leds.begin();
  leds.show();

  Serial.println("OPC SD card streamer, OctoWS2811 version");
  delay(500);

  Serial.print("SD setup... ");
  if (!Sd.begin()) {
    Serial.println("failed. :-( ");
    for(;;);
  }
  Serial.println("succeeded! :-D ");
  delay(500);

  for(root = Sd.open("/"); (nFiles < MAX_FILES) && (entry = root.openNextFile()); entry.close()) {
    
    if(entry.isDirectory()) continue;
    //name = entry.name();
    //Serial.println("Getting name.");
    entry.getName(tmp, sizeof(tmp));
    //memcpy(name, tmp, 13);
    name = tmp;
    //Serial.print("Name: ");
    //Serial.println(name);
    //Serial.println(tmp);
    //Serial.println("something happened.");
    //memcpy(name, entry.name(), sizeof(entry.name()));
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

#define MODE_DATA     0
#define MODE_HEADER   1
#define MODE_DISCARD  2

uint8_t mode = MODE_HEADER;
uint32_t frameno = 0;

uint32_t firstframe, mil;
uint32_t s;
uint16_t m;

void loop() {
  File file;

  Serial.println(filename[currentFile]);
  delay(2000);
  if (file = Sd.open(filename[currentFile])) {
    int16_t a, dataSize;
    //uint8_t w;
    byte d[4];
    uint32_t curpos;
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

        /*for (int i = 0; i < 140; i++) {
          byte r, g, b;
          file.read(&r, sizeof(r));
          file.read(&g, sizeof(g));
          file.read(&b, sizeof(b));
          //pixels[i].r = r;
          //pixels[i].g = g;
          //pixels[i].b = b;
        }*/
        file.read(&drawingMemory, dataSize);
        
        if (frameno > 0) {
          while (millis() < firstframe + (uint32_t)(FRAMEDELAY * frameno)) {
            ;
          }
        } else firstframe = millis();

        //strip.sendPixels(MAX_LEDS, pixels);
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

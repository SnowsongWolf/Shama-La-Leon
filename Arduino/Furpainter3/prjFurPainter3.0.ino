#include <SPI.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <OctoWS2811.h>

#define LED_CH      500   // 500 pixels per channel
#define MAX_FILES   50    // 50 files to load from SD card
#define FRAMEDELAY  0     // Microsecond delay between frames

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
    Serial.println("Getting name.");
    entry.getName(tmp, sizeof(tmp));
    //memcpy(name, tmp, 13);
    name = tmp;
    Serial.print("Name: ");
    Serial.println(name);
    Serial.println(tmp);
    Serial.println("something happened.");
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

void loop() {
  // put your main code here, to run repeatedly:

}

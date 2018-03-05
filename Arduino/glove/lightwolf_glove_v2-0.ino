// Lightwolf Glove v2.0
// Now with 20% more cool

#include <EEPROMex.h>               // Library for EEPROM memory access
#define memoryBase 32               // Where to store config data in EEPROM
#define CONFIG_VERSION "lw3"        // EEPROM settings version code.




const char cfgVersion[4] = CONFIG_VERSION;  // EEPROM settings version code
const bool debug = true;                    // Debug on prints dPrint messages and forces serial connection before execution

String cmdIn;                               // String to hold incomming serial commands 

bool settingsUpdated = false;        // Make sure we only save settings when they have changed to avoid aging the EEPROM
int configAddress = 0;

struct StoreStruct {
  char      version[4];              // Detection for correct settings/program version
  uint16_t  timeoutShort;            // Short timeout in ms
  uint32_t  timeoutLong;             // Long timeout in ms
  uint8_t   channel;                 // Radio channel
  char      txAddr[6];               // Radio transmit pipe address
  char      rxAddr[6];               // Radio receive pipe address
  uint16_t  btnHoldTime;             // Time for button hold to trigger in ms
  bool      fbHaptic;                // Flag for haptic feedback
  bool      fbNeopixel;              // Flag for neopixel feedback
  uint16_t  clickFreq;               // Audio frequency of the click/chirp track
  uint16_t  clickDelay;              // Delay between click/chirp and start of animation
  uint16_t  settingsUpdates;         // Number of times the settings data has been updated
} storage = {
  CONFIG_VERSION,
  20000,
  60000,
  108,
  "lwlf1",
  "lwlf2",
  750,
  true,
  true,
  20000,
  2000,
  1
};



void setup() {
  Serial.begin(115200);         // Begin serial communication
  if (debug) {
    while (!Serial) delay (10); // Force system to wait for serial connection if debug enabled
  }
  dPrintln("Serial connection established.");
  
  EEPROM.setMemPool(memoryBase, EEPROMSizeTeensy31);      // Set memory base for Teensy 3.1+ board
  configAddress = EEPROM.getAddress(sizeof(StoreStruct)); // size of config object
  
  loadConfig();                 // Load config settings from EEPROM.  Will use and save default values if config version mismatch
}



void loop() {
  static elapsedMicros serTime = 0;
  if (!Serial.available()) serTime = 0;   // Reset the timer if no serial data is available yet
  
  if (updateSerial()) {                   // If a serial command was received
    dPrintln("");
    dPrintln("--------------------");
    dPrint("Command received: ");
    dPrintln(cmdIn);
    dPrint("Serial read took ");
    dPrint((uint32_t)serTime);
    dPrintln(" µs.");
    parseSerial();
    cmdIn = "";
  }
}



void loadConfig() {
  if (EEPROM.read(memoryBase) == cfgVersion[0] &&
      EEPROM.read(memoryBase + 1) == cfgVersion[1] &&
      EEPROM.read(memoryBase + 2) == cfgVersion[2]) {
    EEPROM.readBlock(configAddress, storage);
    dPrintln("Settings loaded.");
  }
  else {
    saveConfig();
    dPrintln("Settings not found.  Saving defaults.");
  }

  dPrintSettings();
}

void saveConfig() {
  storage.settingsUpdates++;
  EEPROM.writeBlock(configAddress, storage);
}

void dPrint(String msg) {
  if (debug) Serial.print(msg);       // Print debug message if debug enabled
}

void dPrintln(String msg) {
  if (debug) Serial.println(msg);     // Print debug message line if debug enabled
}

void dPrintSettings() {               // Print all settings to serial if debug enabled
  if (debug) {
    dPrint("Version ");
    dPrintln(storage.version);
    
    dPrint("Short timeout ");
    dPrint(storage.timeoutShort);
    dPrintln("ms");
    
    dPrint("Long timeout ");
    dPrint(storage.timeoutLong);
    dPrintln("ms");
    
    dPrint("Radio channel ");
    dPrintln(storage.channel);
    
    dPrint("Radio tx pipe ");
    dPrintln(storage.txAddr);
    
    dPrint("Radio rx pipe ");
    dPrintln(storage.rxAddr);

    dPrint("Button hold time ");
    dPrint(storage.btnHoldTime);
    dPrintln("ms");

    dPrint("Haptic feeback ");
    if (storage.fbHaptic) dPrintln("ON"); else dPrintln("OFF");

    dPrint("NeoPixel feedback ");
    if (storage.fbNeopixel) dPrintln("ON"); else dPrintln("OFF");

    dPrint("Click track frequency ");
    dPrint(storage.clickFreq);
    dPrintln("Hz");

    dPrint("Click track delay ");
    dPrint(storage.clickDelay);
    dPrintln("ms");

    dPrint("The settings have been saved ");
    dPrint(storage.settingsUpdates);
    dPrintln(" times.");
  }
}

bool updateSerial() {
  if (Serial.available()) {                   // If incomming serial data is waiting
    char charIn = Serial.read();              // Read a single character per loop
    switch (charIn) {                         // Decide what to do by which character was received
      case '\n':                              // Skip new line codes
        break;                                // Break from Switch block

      case '\r':
        Serial.flush();                       // Remove any leftover data from the serial buffer
        return true;                          // Return true for full command received
        break;                                // Not technically necessary but improves readability

      default:                                // For all other characters
        cmdIn += charIn;                      // Add the character to the command string
    }
  }
  return false;                               // Return false for no command received
}

void parseSerial() {
  String cmd = cmdIn.substring(0, 2).toUpperCase();         // Command will always be the first 2 characters received
  dPrint("Command is ");
  //dPrintln(cmd);
  
  if (cmd == "SS") {                                        // Check if settings are being saved first
    dPrintln("Save Settings");
    if (settingsUpdated) {
      saveConfig();
      dPrintSettings();
    } else
      dPrintln("No settings have been changed");
    return ;
  }

  settingsUpdated = true;                                   // Assume settings changed

  String val = cmdIn.substring(3, cmdIn.length()).toLowerCase();

  if (cmd == "ST") {
      dPrint("Short Timeout: ");
      storage.timeoutShort = val.toInt();
      dPrint(storage.timeoutShort);
      dPrintln("ms");

  } else if (cmd == "LT") {
      dPrint("Long Timeout: ");
      storage.timeoutLong = strTo32(val);
      dPrint(storage.timeoutLong);
      dPrintln("ms");

  } else if (cmd == "CH") {
      dPrint("Wireless Channel: ");
      storage.channel = (uint8_t)val.toInt();
      dPrintln(storage.channel);

  } else if (cmd == "TX") {
      dPrint("Transmit Address: ");
      cmdIn.substring(3, cmdIn.length()).toCharArray(storage.txAddr, 6);
      dPrintln(storage.txAddr);

  } else if (cmd == "RX") {
      dPrint("Receive Address: ");
      cmdIn.substring(3, cmdIn.length()).toCharArray(storage.rxAddr, 6);
      dPrintln(storage.rxAddr);

  } else if (cmd == "LP") {
      dPrint("Long Press Time: ");
      storage.btnHoldTime = val.toInt();
      dPrint(storage.btnHoldTime);
      dPrintln("ms");

  } else if (cmd == "HF") {
      dPrint("Haptic Feedback: ");
      if ((val == "true") || (val == "on")) {
        dPrintln("ON");
        storage.fbHaptic = true;
      } else {
        dPrintln("OFF");
        storage.fbHaptic = false;
      }
        

  } else if (cmd == "NF") {
      dPrint("NeoPixel Feedback: ");
      if ((val == "true") || (val == "on")) {
        dPrintln("ON");
        storage.fbNeopixel = true;
      } else {
        dPrintln("OFF");
        storage.fbNeopixel = false;
      }

  } else if (cmd == "CF") {
      dPrint("Click Frequency: ");
      storage.clickFreq = val.toInt();
      dPrint(storage.clickFreq);
      dPrintln("Hz");

  } else if (cmd == "CD") {
      dPrint("Click Delay: ");
      storage.clickDelay = val.toInt();
      dPrint(storage.clickDelay);
      dPrintln("ms");

  } else if (cmd == "LS") {
      dPrintln("List Settings");
      dPrintSettings();

  } else if (cmd == "XX") {
      dPrintln("Reload Settings");
      loadConfig();
      dPrintSettings();
      settingsUpdated = false;
      
  }else {
      dPrintln("Unrecognized.");
      settingsUpdated = false;              // Command not recognized, do not save settings
      return;
  }
}

uint32_t strTo32(String in) {
  if (in.trim().length() > 4) {
    uint32_t msb = in.trim().substring(0,5).toInt() * pow(10, max(in.trim().length() - 4, 0));
    uint32_t lsb = in.trim().substring(5, in.trim().length()).toInt();
    /*dPrint("MSB ");
    dPrintln(msb);
    dPrint("LSB ");
    dPrintln(lsb);*/
    return (uint32_t)(msb + lsb);
  } else
    return (uint32_t)(in.trim().toInt());
}

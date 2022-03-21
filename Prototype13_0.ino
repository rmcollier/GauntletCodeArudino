/**
 * \file MP3Shield_Library_Demo.ino
 *
 * \brief Example sketch of using the MP3Shield Arduino driver
 * \remarks comments are implemented with Doxygen Markdown format
 *
 * \author Bill Porter
 * \author Michael P. Flaga
 *
 * This sketch listens for commands from a serial terminal (like the Serial
 * Monitor in the Arduino IDE). If it sees 1-9 it will try to play an MP3 file
 * named track00x.mp3 where x is a number from 1 to 9. For eaxmple, pressing
 * 2 will play 'track002.mp3'. A lowe case 's' will stop playing the mp3.
 * 'f' will play an MP3 by calling it by it's filename as opposed to a track
 * number.
 *
 * Sketch assumes you have MP3 files with filenames like "track001.mp3",
 * "track002.mp3", etc on an SD card loaded into the shield.
 */



// NewHaven Display 
#include <NewHavenLCD.h>
#include <font5x7.h> 

// Other Libraries
#include <pgmspace.h>
#include <MemoryFree.h>
#include <SPI.h>

//Add the SdFat Libraries
#include <SdFat.h>
#include <SdFatUtil.h>

//and the MP3 Shield Library
#include <SFEMP3Shield1284p.h>

// Below is not needed if interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  #include <TimerOne.h>
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  #include <SimpleTimer.h>
#endif


// From Prototype12_2


#define PAGE0 0xB0
#define PAGE1 0xB1
#define PAGE2 0xB2
#define PAGE3 0xB3

// screenState
 #define MP3           1
 #define MP3Volume     2
 #define Pedometer     3
 #define RestTimer     4
 #define RestTimerEDIT 5
 #define ECG           6
 #define ECGAlerts     7 
 #define ECGAlertsEDIT 8
 volatile int screenState;


 //#define FLEXCONTROL_OFF      0
 #define FLEXCONTROL_REGULAR  0
 #define FLEXCONTROL_INVERSE  1
 int FlexControlMode = 1;
 
 #define ENGAGED       0
 #define DISENGAGED    1
 int VolumeGesture = 1;

 #define NUMBUTTONS 3
 #define NUMTRACKS 4
 #define REST_TIMER_TRACK 10
 #define FlexDelay 1500         //  Time in ms the flex must be held to engage Volume Increase
 //#define flexCountDelay 800   //  Time in ms the double flex must occur in to engage Track Seek Mode
 #define advTrackThresh 215     //  Accelerometer reading at palm-up orientation
 #define bckTrackThresh 420     //  Accelerometer reading at palm-down orientation
 #define seekRightThresh 200    // Enter Seekmode threshold points
 #define seekLeftThresh 400


 byte buttonState[NUMBUTTONS];
 byte TCCRtemp[5];
 
  
 volatile unsigned int trackSelect = 0;   // Number of tracks is hardcoded in TrackSeek code!!
 int accelSensor = 0;
 int outputValue = 0;
 int sensorValue;
 int flexSensor; //LOW if flex sensor is bent. HIGH if reading above flexThreshold
 unsigned int flexThreshold = 350;
 int lastflexSensor = HIGH;
 unsigned long firstFlexTime;
 unsigned long secondFlexTime;     
 unsigned int trackSeekDelay = 900;
 int flexCount = 0;
 boolean trackSeek = false;
 
 // VOLUME VARIABLES
 boolean volumeSeek = false;
 int curVolume = 20;
 int MaxVolume = 10; // Initialize MaxVolume. (Lower number == Louder)
 int MinVolume = 40;
 const unsigned int volStep = 2;   // Changed from 4
//boolean  LEDindicator = false;  // Just to record when yellowLED (volume increasing LED) is activated.
 
#define YELLOW  1
#define BLUE    2
 
 // Device modes
 boolean ECG_mode =       false;
 boolean MP3_mode =       false;
 boolean Pedometer_mode = false;
 boolean Rest_mode =      false;
 boolean ECGalert_mode =  false;
 boolean LCDscreen =      true;      
 boolean GCVModeRefreshFlag = false;
 
 // PEDOMETER VARIABLES
 volatile unsigned int stepCount = 0; // Pedometer step counter of zeroGdetect signal.
 volatile unsigned long int zeroGtime = 0;
 unsigned int zeroGdelay = 200;
 
 
 // REST TIMER VARIABLES
 volatile unsigned int restTimeSec = 10;
 volatile unsigned long int startRest;
 uint32_t pausePosition = 0;
 boolean RestTimer_isPlaying = false;
 boolean RestTimer_hasPlayed = false;
 boolean MP3_wasPlaying = false;
 boolean RestTimer_readyStart = false;
 uint16_t pauseByteRate;
 
 // DISPLAY VARIABLES
 char captionBuffer[60] = {0};   // THIS SHOULD BE VOLATILE !!!   I think. Because it's values are changing a lot.
  char buttonBuffer[43] = {0};
  byte registerState = 0;
  boolean displayLEDstate;
  //unsigned long LCDbacklightChangeTime = 0;
//unsigned int backlightTiltDebounce = 1000;
 
        // these variables are volatile because they are used during the interrupt service routine!
        volatile int BPM;                   // used to hold the pulse rate
        volatile int Signal;                // holds the incoming raw data
        volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
        volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
        volatile boolean QS = false;        // becomes true when Arduoino finds a beat.
   
PROGMEM const unsigned char *text_table[] = 
{
  caption_MP3_,    // 0
  caption_MP3volume_,
  caption_Pedometer_,
  caption_RestTimer_,
  caption_ECG_,
  caption_EDIT_,    // 5
  buttons_Menu_,
  buttons_Pause_,
  buttons_Play_,
  buttons_Volume_,    
  buttons_Return_,  // 10
  buttons_up_,
  buttons_down_,
  buttons_Reset_,
  buttons_Start_,
  buttons_Stop_,  // 15
  buttons_Edit_,
  buttons_Alerts_,
  volumeSeek_low_,
  volumeSeek_high_,
  large_numbers_,  // 20
  small_numbers_,
  icon_run_,
  icon_clock_,
  icon_mp3_,
  icon_heart_,    // 25

};


 //  PIN ASSIGNMENTS
const int zeroGdetect   =  17;       // A3
const int LCD_REGSEL    =  11;        // D4
const int LCD_CS        =  12;        // D5
const int LCD_backlight =  13;        // D10 
//const int latchPin      =  1;         // TX
//const int testSerial    =  11;        // 
const int LEDpin        =  24;        
const int pulsePin      =  28;        // A3 
const int flexPin       =  29;       // A4
const int accelPin      =  30;       // A5
const byte buttons[]    =  {25,26,27}; //Used to be A0, A1, A2 on the 328p




/**
 * \brief Object instancing the SdFat library.
 *
 * principal object for handling all SdCard functions.
 */
SdFat sd;

/**
 * \brief Object instancing the SFEMP3Shield library.
 *
 * principal object for handling all the attributes, members and functions for the library.
 */
SFEMP3Shield MP3player;


NewHavenLCD LCD;







//------------------------------------------------------------------------------
/**
 * \brief Setup the Arduino Chip's feature for our use.
 *
 * After Arduino's kernel has booted initialize basic features for this
 * application, such as Serial port and MP3player objects with .begin.
 * Along with displaying the Help Menu.
 *
 * \note returned Error codes are typically passed up from MP3player.
 * Whicn in turns creates and initializes the SdCard objects.
 *
 * \see
 * \ref Error_Codes
 */
void setup() {

  uint8_t result; //result code from some function as to be tested at later time.

  Serial.begin(115200);

  Serial.print(F("Free RAM = ")); // available in Version 1.0 F() bases the string to into Flash, to use less SRAM.
  Serial.print(FreeRam(), DEC);  // FreeRam() is provided by SdFatUtil.h
  Serial.println(F(" Should be a base line of 1040, on ATmega328 when using INTx"));


  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");

  //Initialize the MP3 Player Shield
  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to start MP3 player"));
    if( result == 6 ) {
      Serial.println(F("Warning: patch file not found, skipping.")); // can be removed for space, if needed.
      Serial.println(F("Use the \"d\" command to verify SdCard can be read")); // can be removed for space, if needed.
    }
  }

#if (0)
  // Typically not used by most shields, hence commented out.
  Serial.println(F("Applying ADMixer patch."));
  if(MP3player.ADMixerLoad("admxster.053") == 0) {
    Serial.println(F("Setting ADMixer Volume."));
    MP3player.ADMixerVol(-3);
  }
#endif

  help();
}

//------------------------------------------------------------------------------
/**
 * \brief Main Loop the Arduino Chip
 *
 * This is called at the end of Arduino kernel's main loop before recycling.
 * And is where the user's serial input of bytes are read and analyzed by
 * parsed_menu.
 *
 * Additionally, if the means of refilling is not interrupt based then the
 * MP3player object is serviced with the availaible function.
 *
 * \note Actual examples of the libraries public functions are implemented in
 * the parse_menu() function.
 */
void loop() {

// Below is only needed if not interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) \
    && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
    ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )

  MP3player.available();
#endif

  if(Serial.available()) {
    parse_menu(Serial.read()); // get command from serial input
  }

  delay(100);
}

//------------------------------------------------------------------------------
/**
 * \brief Decode the Menu.
 *
 * Parses through the characters of the users input, executing corresponding
 * MP3player library functions and features then displaying a brief menu and
 * prompting for next input command.
 */
void parse_menu(byte key_command) {

  uint8_t result; // result code from some function as to be tested at later time.

  // Note these buffer may be desired to exist globably.
  // but do take much space if only needed temporarily, hence they are here.
  char title[30]; // buffer to contain the extract the Title from the current filehandles
  char artist[30]; // buffer to contain the extract the artist name from the current filehandles
  char album[30]; // buffer to contain the extract the album name from the current filehandles

  Serial.print(F("Received command: "));
  Serial.write(key_command);
  Serial.println(F(" "));

  //if s, stop the current track
  if(key_command == 's') {
    Serial.println(F("Stopping"));
    MP3player.stopTrack();

  //if 1-9, play corresponding track
  } else if(key_command >= '1' && key_command <= '9') {
    //convert ascii numbers to real numbers
    key_command = key_command - 48;

#if USE_MULTIPLE_CARDS
    sd.chvol(); // assign desired sdcard's volume.
#endif
    //tell the MP3 Shield to play a track
    result = MP3player.playTrack(key_command);

    //check result, see readme for error codes.
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to play track"));
    } else {

      Serial.println(F("Playing:"));

      //we can get track info by using the following functions and arguments
      //the functions will extract the requested information, and put it in the array we pass in
      MP3player.trackTitle((char*)&title);
      MP3player.trackArtist((char*)&artist);
      MP3player.trackAlbum((char*)&album);

      //print out the arrays of track information
      Serial.write((byte*)&title, 30);
      Serial.println();
      Serial.print(F("by:  "));
      Serial.write((byte*)&artist, 30);
      Serial.println();
      Serial.print(F("Album:  "));
      Serial.write((byte*)&album, 30);
      Serial.println();
    }

  //if +/- to change volume
  } else if((key_command == '-') || (key_command == '+')) {
    union twobyte mp3_vol; // create key_command existing variable that can be both word and double byte of left and right.
    mp3_vol.word = MP3player.getVolume(); // returns a double uint8_t of Left and Right packed into int16_t

    if(key_command == '-') { // note dB is negative
      // assume equal balance and use byte[1] for math
      if(mp3_vol.byte[1] >= 254) { // range check
        mp3_vol.byte[1] = 254;
      } else {
        mp3_vol.byte[1] += 2; // keep it simpler with whole dB's
      }
    } else {
      if(mp3_vol.byte[1] <= 2) { // range check
        mp3_vol.byte[1] = 2;
      } else {
        mp3_vol.byte[1] -= 2;
      }
    }
    // push byte[1] into both left and right assuming equal balance.
    MP3player.setVolume(mp3_vol.byte[1], mp3_vol.byte[1]); // commit new volume
    Serial.print(F("Volume changed to -"));
    Serial.print(mp3_vol.byte[1]>>1, 1);
    Serial.println(F("[dB]"));

  //if < or > to change Play Speed
  } else if((key_command == '>') || (key_command == '<')) {
    uint16_t playspeed = MP3player.getPlaySpeed(); // create key_command existing variable
    // note playspeed of Zero is equal to ONE, normal speed.
    if(key_command == '>') { // note dB is negative
      // assume equal balance and use byte[1] for math
      if(playspeed >= 254) { // range check
        playspeed = 5;
      } else {
        playspeed += 1; // keep it simpler with whole dB's
      }
    } else {
      if(playspeed == 0) { // range check
        playspeed = 0;
      } else {
        playspeed -= 1;
      }
    }
    MP3player.setPlaySpeed(playspeed); // commit new playspeed
    Serial.print(F("playspeed to "));
    Serial.println(playspeed, DEC);

  /* Alterativly, you could call a track by it's file name by using playMP3(filename);
  But you must stick to 8.1 filenames, only 8 characters long, and 3 for the extension */
  } else if(key_command == 'f' || key_command == 'F') {
    uint32_t offset = 0;
    if (key_command == 'F') {
      offset = 2000;
    }

    //create a string with the filename
    char trackName[] = "track001.mp3";

#if USE_MULTIPLE_CARDS
    sd.chvol(); // assign desired sdcard's volume.
#endif
    //tell the MP3 Shield to play that file
    result = MP3player.playMP3(trackName, offset);
    //check result, see readme for error codes.
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to play track"));
    }

  /* Display the file on the SdCard */
  } else if(key_command == 'd') {
    if(!MP3player.isPlaying()) {
      // prevent root.ls when playing, something locks the dump. but keeps playing.
      // yes, I have tried another unique instance with same results.
      // something about SdFat and its 500byte cache.
      Serial.println(F("Files found (name date time size):"));
      sd.ls(LS_R | LS_DATE | LS_SIZE);
    } else {
      Serial.println(F("Busy Playing Files, try again later."));
    }

  /* Get and Display the Audio Information */
  } else if(key_command == 'i') {
    MP3player.getAudioInfo();

  } else if(key_command == 'p') {
    if( MP3player.getState() == playback) {
      MP3player.pauseMusic();
      Serial.println(F("Pausing"));
    } else if( MP3player.getState() == paused_playback) {
      MP3player.resumeMusic();
      Serial.println(F("Resuming"));
    } else {
      Serial.println(F("Not Playing!"));
    }

  } else if(key_command == 'r') {
    MP3player.resumeMusic(2000);

  } else if(key_command == 'R') {
    MP3player.stopTrack();
    MP3player.vs_init();
    Serial.println(F("Reseting VS10xx chip"));

  } else if(key_command == 't') {
    int8_t teststate = MP3player.enableTestSineWave(126);
    if(teststate == -1) {
      Serial.println(F("Un-Available while playing music or chip in reset."));
    } else if(teststate == 1) {
      Serial.println(F("Enabling Test Sine Wave"));
    } else if(teststate == 2) {
      MP3player.disableTestSineWave();
      Serial.println(F("Disabling Test Sine Wave"));
    }

  } else if(key_command == 'm') {
      uint16_t teststate = MP3player.memoryTest();
    if(teststate == -1) {
      Serial.println(F("Un-Available while playing music or chip in reset."));
    } else if(teststate == 2) {
      teststate = MP3player.disableTestSineWave();
      Serial.println(F("Un-Available while Sine Wave Test"));
    } else {
      Serial.print(F("Memory Test Results = "));
      Serial.println(teststate, HEX);
      Serial.println(F("Result should be 0x83FF."));
      Serial.println(F("Reset is needed to recover to normal operation"));
    }

  } else if(key_command == 'e') {
    uint8_t earspeaker = MP3player.getEarSpeaker();
    if(earspeaker >= 3){
      earspeaker = 0;
    } else {
      earspeaker++;
    }
    MP3player.setEarSpeaker(earspeaker); // commit new earspeaker
    Serial.print(F("earspeaker to "));
    Serial.println(earspeaker, DEC);

  } else if(key_command == 'M') {
    uint16_t monostate = MP3player.getMonoMode();
    Serial.print(F("Mono Mode "));
    if(monostate == 0) {
      MP3player.setMonoMode(1);
      Serial.println(F("Enabled."));
    } else {
      MP3player.setMonoMode(0);
      Serial.println(F("Disabled."));
    }

  } else if(key_command == 'g') {
    int32_t offset_ms = 20000; // Note this is just an example, try your own number.
    Serial.print(F("jumping to "));
    Serial.print(offset_ms, DEC);
    Serial.println(F("[milliseconds]"));
    result = MP3player.skipTo(offset_ms);
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to skip track"));
    }

  } else if(key_command == 'k') {
    int32_t offset_ms = -1000; // Note this is just an example, try your own number.
    Serial.print(F("moving = "));
    Serial.print(offset_ms, DEC);
    Serial.println(F("[milliseconds]"));
    result = MP3player.skip(offset_ms);
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to skip track"));
    }

  } else if(key_command == 'O') {
    MP3player.end();
    Serial.println(F("VS10xx placed into low power reset mode."));

  } else if(key_command == 'o') {
    MP3player.begin();
    Serial.println(F("VS10xx restored from low power reset mode."));

  } else if(key_command == 'D') {
    uint16_t diff_state = MP3player.getDifferentialOutput();
    Serial.print(F("Differential Mode "));
    if(diff_state == 0) {
      MP3player.setDifferentialOutput(1);
      Serial.println(F("Enabled."));
    } else {
      MP3player.setDifferentialOutput(0);
      Serial.println(F("Disabled."));
    }

  } else if(key_command == 'S') {
    Serial.println(F("Current State of VS10xx is."));
    Serial.print(F("isPlaying() = "));
    Serial.println(MP3player.isPlaying());

    Serial.print(F("getState() = "));
    switch (MP3player.getState()) {
    case uninitialized:
      Serial.print(F("uninitialized"));
      break;
    case initialized:
      Serial.print(F("initialized"));
      break;
    case deactivated:
      Serial.print(F("deactivated"));
      break;
    case loading:
      Serial.print(F("loading"));
      break;
    case ready:
      Serial.print(F("ready"));
      break;
    case playback:
      Serial.print(F("playback"));
      break;
    case paused_playback:
      Serial.print(F("paused_playback"));
      break;
    case testing_memory:
      Serial.print(F("testing_memory"));
      break;
    case testing_sinewave:
      Serial.print(F("testing_sinewave"));
      break;
    }
    Serial.println();

  } else if(key_command == 'V') {
    MP3player.setVUmeter(1);
    Serial.print(F("VU meter = "));
    Serial.println(MP3player.getVUmeter());
    Serial.println(F("Hit Any key to stop."));

    while(!Serial.available()) {
      union twobyte vu;
      vu.word = MP3player.getVUlevel();
      Serial.print(F("VU: L = "));
      Serial.print(vu.byte[1]);
      Serial.print(F(" / R = "));
      Serial.print(vu.byte[0]);
      Serial.println(" dB");
      delay(1000);
    }
    Serial.read();

    MP3player.setVUmeter(0);
    Serial.print(F("VU meter = "));
    Serial.println(MP3player.getVUmeter());

  } else if(key_command == 'h') {
    help();
  }

  // print prompt after key stroke has been processed.
  Serial.println(F("Enter 1-9,f,F,s,d,+,-,i,>,<,p,r,R,t,m,M,g,k,h,O,o,D,S,V :"));
}

//------------------------------------------------------------------------------
/**
 * \brief Print Help Menu.
 *
 * Prints a full menu of the commands available along with descriptions.
 */
void help() {
  Serial.println(F("Arduino SFEMP3Shield Library Example:"));
  Serial.println(F(" courtesy of Bill Porter & Michael P. Flaga"));
  Serial.println(F("COMMANDS:"));
  Serial.println(F(" [1-9] to play a track"));
  Serial.println(F(" [f] play track001.mp3 by filename example"));
  Serial.println(F(" [F] same as [f] but with initial skip of 2 second"));
  Serial.println(F(" [s] to stop playing"));
  Serial.println(F(" [d] display directory of SdCard"));
  Serial.println(F(" [+ or -] to change volume"));
  Serial.println(F(" [> or <] to increment or decrement play speed by 1 factor"));
  Serial.println(F(" [i] retrieve current audio information (partial list)"));
  Serial.println(F(" [e] increment Spatial EarSpeaker, default is 0, wraps after 4"));
  Serial.println(F(" [p] to pause."));
  Serial.println(F(" [r] resumes play from 2s from begin of file"));
  Serial.println(F(" [R] Resets and initializes VS10xx chip."));
  Serial.println(F(" [t] to toggle sine wave test"));
  Serial.println(F(" [m] perform memory test. reset is needed after to recover."));
  Serial.println(F(" [M] Toggle between Mono and Stereo Output."));
  Serial.println(F(" [g] Skip to a predetermined offset of ms in current track."));
  Serial.println(F(" [k] Skip a predetermined number of ms in current track."));
  Serial.println(F(" [O} turns OFF the VS10xx into low power reset."));
  Serial.println(F(" [o} turns ON the VS10xx out of low power reset."));
  Serial.println(F(" [D] to toggle SM_DIFF between inphase and differential output"));
  Serial.println(F(" [S] Show State of Device."));
  Serial.println(F(" [V] Enable VU meter Test."));
  Serial.println(F(" [h] this help"));
}



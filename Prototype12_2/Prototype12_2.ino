
//#include <SoftwareSerial.h> // May not need

#include <NewHavenLCD.h>
#include <font5x7.h>
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>  
#include <pgmspace.h>
//and the MP3 Shield Library
#include <SFEMP3Shield1284p.h>
//#include <avr/pgmspace.h>
#include <MemoryFree.h>

// Below is not needed if interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  #include <TimerOne.h>
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  #include <SimpleTimer.h>
#endif

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
 
/**
 * \brief Object instancing the library.
 *
 * principal object for handling all the attributes, members and functions for the library.
 */
SdFat sd;
SFEMP3Shield MP3player;
NewHavenLCD LCD;
//SoftwareSerial LCD(A0, 5);         // For 16x2 character LCD display

 //  PIN ASSIGNMENTS
const int zeroGdetect   =  17;       // A3
const int LCD_REGSEL    =  11;        // D4
const int LCD_CS        =  12;        // D5
const int LCD_backlight =  31;        // D10 
const int LEDpin        =  24;        
const int pulsePin      =  28;        // A3 
const int flexPin       =  29;       // A4
const int accelPin      =  30;       // A5
const byte buttons[]    =  {25,26,27}; //Used to be A0, A1, A2 on the 328p

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
  byte i;
  //byte deviceState = 0x00;
  //screenState = 0;
    
  // Make input & enable pull-up resistors on switch pins
  for (i=0; i< NUMBUTTONS; i++) {
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH); // Activate the pull-up resistors
  }
  
  //pinMode (1, INPUT);  // This input button is an active high, unlike the other three active low button configurations. No use of pullup resistor.
  
  pinMode(zeroGdetect, INPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_REGSEL, OUTPUT);
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_REGSEL,HIGH);
  pinMode(LCD_backlight, OUTPUT);
    
  
  
  
  Serial.begin(115200);

  Serial.print(F("Free RAM = ")); // available in Version 1.0 F() bases the string to into Flash, to use less SRAM.
  Serial.println(FreeRam(), DEC);  // FreeRam() is provided by SdFatUtil.h
  //Serial.println(F(" Should be a base line of 1094, on ATmega328 when using INTx"));
  

  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
  
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");
  Serial.println(F("SD Card initialized."));
 
 Serial.print(F("captionBuffer length="));
 Serial.println(sizeof(captionBuffer)/sizeof(captionBuffer[0]));

  //boot up the MP3 Player Shield
  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    //Serial.println(F(" when trying to start MP3 player"));
    if ( result == 6 ) {
      //Serial.println(F("Warning: patch file not found, skipping.")); // can be removed for space, if needed.
      //Serial.println(F("Use the \"d\" command to verify SdCard can be read")); // can be removed for space, if needed.
    }
  }
  Serial.println(F("MP3player initialized."));
  Serial.println(F("Keyboard input disabled!"));
// Typically not used by most shields, hence commented out.
//  Serial.println(F("Applying ADMixer patch."));
//  if (MP3player.ADMixerLoad("admxster.053") == 0) {
//    Serial.println(F("Setting ADMixer Volume."));
//    MP3player.ADMixerVol(-3);
//  }
 


  //help();
  


/*
  // Below is only needed if not interrupt driven. Safe to remove if not using.
    #if defined(USE_MP3_REFILL_MEANS) \
      && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
      ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )
  
    MP3player.available();
    #endif
    
    result = MP3player.playTrack(1); 
    //check result, see readme for error codes. Don't really need this.
    if(result != 0) {
        Serial.print(F("Error code: "));
        Serial.print(result);
        Serial.println(F(" when trying to play track"));
    }
    */
    

    LCD.initialize(LCD_REGSEL, LCD_CS);
    
    screenState = menuMP3();
  
    //MP3player.playMP3("Track001.mp3",60000);
    //MP3player.playMP3withTimecode(1,60000);
    //LCD.displayBPMheart(icon_heart_);
    
    //LCD.displayIcon(icon_MP3_, 1);    
    
    // End Serial communication to make use of RX/TX pins as GPIO
    //Serial.end();
    
    

    
    digitalWrite(LCD_backlight, HIGH);
    displayLEDstate = true;  // Flag to keep track of LCD-baclkight state. Only used in ToggleLED()
    
    
    FlexControlMode = FLEXCONTROL_REGULAR;
    //DecreaseVolume(1);
    
    
}   //   ******* END PROGRAM SETUP()  ******












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

  uint8_t result; // This is part of the MP3 code from Bill Porter.  
  
  /************  ECG_mode  *************/
  if (ECG_mode) { //Pulse processing mode
    
    sendDataToProcessing('S', Signal);     // send Processing the raw Pulse Sensor data
    if (QS == true){                       // Quantified Self flag is true when arduino finds a heartbeat
        //fadeRate = 255;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
        sendDataToProcessing('B',BPM);     // send heart rate with a 'B' prefix
        sendDataToProcessing('Q',IBI);     // send time between beats with a 'Q' prefix
        QS = false;                        // reset the Quantified Self flag for next time    
    }
  } //else   **** END ECG MODE CODE
  
  
  // MP3 Operational Mode
      
  
    // Below is only needed if not interrupt driven. Safe to remove if not using.
    #if defined(USE_MP3_REFILL_MEANS) \
      && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
      ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )
  
    MP3player.available();
    #endif
  
  
    /** Parse keypboard input disabled!
    if(Serial.available()) {
        parse_menu(Serial.read()); // get command from serial input
    }
    **/
  
    // POLL CONTINUOUS SENSORS
    // [flexSensor = LOW if flexed. HIGH if grip is released.]
    // [accelerometer currently only needed for when in track seek mode, or when Pedometer_mode = True]
    sensorValue = analogRead(flexPin);
    accelSensor = analogRead(accelPin);
    
 

    
    // Poll zeroGdetect
    
    if (Pedometer_mode && digitalRead(zeroGdetect) && (millis() - zeroGtime) > zeroGdelay){
      stepCount += 1;
      zeroGtime = millis();
      //Serial.print(F("step "));
      //Serial.println(stepCount);
      
      //if (screenState == Pedometer) {
        
        memset (captionBuffer,0,sizeof(captionBuffer)/captionBuffer[0]);  // Fills captionBuffer with zeros
        memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[21])), sizeof(small_numbers_)/sizeof(small_numbers_[0]));
        LCD.displaySmallDigits(stepCount, captionBuffer, PAGE0, 0x15, 0x09);
        
      //}
    }
    
    
    /*** DYNAMIC DISPLAY BACKLIGHT
    THIS FAILS FOR SOME REASON. UPON FLEXING AND THEN RELEASING THE FLEX SENSOR, AUDIO DOES NOT PLAY AND THE DEVICE FREEZES
    
    if ((accelSensor > bckTrackThresh) && !LCDscreen && (millis() - LCDbacklightChangeTime) > backlightTiltDebounce) {
 
      // Turn backlight on
      registerState = LCD.backlightON(registerState);
      LCDbacklightChangeTime = millis();
      LCDscreen = true;
      
    } else if ((accelSensor < bckTrackThresh) && LCDscreen && (millis() - LCDbacklightChangeTime) > backlightTiltDebounce) {
      
      // Turn LCD backlight off
      registerState = LCD.backlightOFF(registerState);
      LCDbacklightChangeTime = millis();
      LCDscreen = false;
    }
    */
    
    // What is  VolumeGesture state
    if (sensorValue < flexThreshold) {
        flexSensor = LOW; // FLEXED
        if (FlexControlMode == FLEXCONTROL_REGULAR){
            VolumeGesture = ENGAGED;
            
        } else {
            VolumeGesture = DISENGAGED;
        }
      
    } else {
        flexSensor = HIGH; // RELAXED
        trackSeek == false;
        if (FlexControlMode == FLEXCONTROL_REGULAR){
            VolumeGesture = DISENGAGED;
            
        } else {
            VolumeGesture = ENGAGED;
        }
    }
    
   
    
    // TRACK SEEK MODE
    
    if (trackSeek && flexSensor==LOW) {  // Should flexSensor==LOW by VolumeGesture==ENGAGED ??
    
    
          if (curVolume > MaxVolume)
          {
              curVolume = IncreaseVolume(1);
              
          }
      
          if (accelSensor < advTrackThresh) {
              //Serial.println(F("Next"));
              MP3player.stopTrack();
              //tell the MP3 Shield to play a track
              if (trackSelect == 9){                           // HARDCODE
                trackSelect = 0;
              }
              pausePosition = 0;  // Start recording from the start of song until when Pause is pressed.
                result = MP3player.playTrack(trackSelect += 1);
              //Serial.println(trackSelect);
              /*
              if(result != 0) {
                  Serial.print(F("Error code: "));
                  Serial.print(result);
                  Serial.println(F(" when trying to play track"));
              }
              */
              delay(1000);
            
          } else if (accelSensor > bckTrackThresh) {
              //Serial.println(F("Previous"));        
              MP3player.stopTrack();
              //tell the MP3 Shield to play a track
              if (trackSelect <= 1) {
                result = MP3player.playTrack(1);
                trackSelect = 1; // In case it was actuall less than zero somehow
              } else {
                result = MP3player.playTrack(trackSelect -= 1);
                pausePosition = 0;  // Start recording from the start of song until when Pause is pressed.
              }
              //Serial.print(F("Playing track #: ")); Serial.println(trackSelect);
              /*
              if(result != 0) {
                Serial.print(F("Error code: "));
                Serial.print(result);
                Serial.println(F(" when trying to play track"));
              }
              */
            delay(1000);
          }
          
          /* check result, see readme for error codes. Don't really need this.
          if(result != 0) {
              Serial.print(F("Error code: "));
              Serial.print(result);
              Serial.println(F(" when trying to play track"));
          }   */
          
          // END OF TRACKSEEK CODE
    
    } else {  
      
      
      
      
      // If Gesture has changed...
      if ((flexSensor != lastflexSensor) || GCVModeRefreshFlag) {
      
      GCVModeRefreshFlag = false;  // This flag is set in ToggleGestureControlMode. Need to get back this 'if' statement to refresh VolumeGesture value.
        
        if (VolumeGesture == DISENGAGED){
        
              // 1. Drop volume nearly instantly. 
              // 2. Exit trackSeek mode if applicable.
              // 3. If at adjusting volume screen, display appropriate captions
              // 4. If Rest Timer is On, start the Rest Timer if applicable.
              
              // 1. Drop volume nearly instantly. 
              curVolume = DecreaseVolume(MinVolume);
              
              // 2. Exit trackSeek mode.
              trackSeek = false;
              
             // 3. If at adjusting volume screen, display appropriate caption 
              if (screenState == MP3Volume)
              {
                  LCD.comm_out(PAGE2);
                  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
                  memset (captionBuffer,0,sizeof(captionBuffer)/captionBuffer[0]); 
                  memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[18])), sizeof(volumeSeek_low_)/sizeof(volumeSeek_low_[0]));
                  LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
              }
              
              // 4. If Rest Timer is On, start the Rest Timer if applicable.
              //if (Rest_mode && RestTimer_hasPlayed && RestTimer_readyStart) 
              else {  // Else'd from above because gesture changes during volume adjustment should not effect Rest Timer
                  if (Rest_mode && RestTimer_readyStart) {
                      startRest = millis();
                      RestTimer_readyStart = false;
                      RestTimer_hasPlayed = false;
                      //Serial.println(F("Rest Timer counting down..."));
                  }
              }
              
        } else if (VolumeGesture == ENGAGED) { 
          // Gesture has changed to Engage Volume State
        
            if (screenState == MP3Volume){
                  LCD.comm_out(PAGE2);
                  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
                  memset (captionBuffer,0,sizeof(captionBuffer)/captionBuffer[0]); 
                  memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[19])), sizeof(volumeSeek_low_)/sizeof(volumeSeek_low_[0]));
                  LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
            }
            
            
            // Start volume-engage delay timer.
            if (!flexCount) {
              firstFlexTime = millis();  
            }
            
            // This only for trackseek
            if ((accelSensor > seekRightThresh) && (accelSensor < seekLeftThresh)) {
                flexCount += 1;
            }
            
            if (RestTimer_isPlaying){
                // Rest Timer alert is playing, and flex was engaged and held, meaning user wants to acknowledge the alert.
                // Return MP3 player to normal playback if applicable, and reset RestTimer_isPlaying flag. But don't reset
                // RestTimer_hasPlayed, so as the user can still choose to rest at low volume without having the timer alert play
                // again. Can do this by engaging flex without letting it climb to maximum volume. (ie, flexing and releasing quickly)
                
                MP3player.stopTrack();
                if (MP3_wasPlaying){
                  RestTimer_unpauseMP3();
                  MP3_wasPlaying = false;
                }
                RestTimer_isPlaying = false;
                RestTimer_hasPlayed = true;
            }
            
            if (flexCount == 2){
                 secondFlexTime = millis(); 
            }
        }
        
    } else {       // Gesture has not changed
      
      
          // If volume gesture in ENGAGED mode, flexDelay has passed, and volume is less than MaxVolume, then increase volume
            if ((VolumeGesture == ENGAGED) && (millis() - firstFlexTime) > FlexDelay){
                // Engage volume increase
                
                
                if (curVolume > MaxVolume) {
                    curVolume = IncreaseVolume(1); // Parameter flag: Still increasing, MaxVolume not yet reached
                    
                    // If user has flexed long enough to reach max volume, safe to assume they started another set, and wish to reset timer alert
                    if (curVolume <= MaxVolume){   // Changed from: (RestTimer_hasPlayed && curVolume >= MaxVolume)
                          if (Rest_mode) {
                          RestTimer_readyStart = true;
                          }
                          
                           //digitalWrite(LEDpin,HIGH);
                    }
       
                } else if (flexCount == 2 && (accelSensor > seekRightThresh) && (accelSensor < seekLeftThresh)){
                    //Serial.println(F("Trackseek engaged"));
                    // not using in if statement:    && (millis()-secondFlexTime) < trackSeekDelay
                    
                    trackSeek = true;
                    digitalWrite(LEDpin,HIGH);
                    
                    flexCount = 0;
                    
                    // Start MP3 player for first time since power on.
                    if (!MP3_mode){
                      MP3_mode = true;
                      menuMP3();
                    }
                    
                } else {
                    //flexCount = 0;
                    trackSeek = false;
                }
                
            } else if ((millis() - firstFlexTime) > FlexDelay) {
                flexCount = 0;
            }
            
            
            // Rest Timer Alert 
            if (Rest_mode && (millis() - startRest) > restTimeSec*1000 && !RestTimer_hasPlayed && !RestTimer_readyStart && !trackSeek)
            {
                if (MP3player.isPlaying()) {
                  RestTimer_pauseMP3();
                  MP3_wasPlaying = true;
                }
              //Serial.println("Rest Timer alert");
              delay(100);
              MP3player.playTrack(REST_TIMER_TRACK); // Rest Timer .mp3 assigned to track 8 for now.
              delay(20);
              RestTimer_isPlaying = true;
              RestTimer_hasPlayed = true;
              // Return to playing trackSelect from pausePosition
              // MP3Player.playMP3(char* trackSelect_fileName, uint32_ttimecode)
            }
    
            // Determine when the RestTimer alert has stopped playing
            if (RestTimer_isPlaying && !MP3player.isPlaying() ){
              //Serial.print("Alert ended");  
              MP3player.stopTrack();
                if (MP3_wasPlaying){
                  //Serial.println("Should be unpausing..");
                  RestTimer_unpauseMP3();
                  MP3_wasPlaying = false; // Reset music was playing before Rest Timer alert track began
                }
                RestTimer_hasPlayed = true;
                RestTimer_isPlaying = false;
            }

      } // END OF NO-GESTURE-CHANGE MODE
      
      lastflexSensor = flexSensor;

  } // End of Non-track Seek MP3 mode
    
    
/*  screenState
 #define MP3           1
 #define MP3Volume     2
 #define Pedometer     3
 #define RestTimer     4
 #define RestTimerEDIT 5
 #define ECG           6
 #define ECGAlerts     7 
 #define ECGAlertsEDIT 8

*/




    // ******************   Poll buttons  ****************************
    
    
    
    for (int index = 0; index < NUMBUTTONS; index++) {
      buttonState[index] = digitalRead(buttons[index]);
      delay(10);
    }
    
   
   
   
    /*
    // Button 4 is active HIGH button instead of active LOW like the others ...
    if (buttonState[3] == LOW){
      
          if (FlexControlMode == FLEXCONTROL_REGULAR) {
              FlexControlMode = FLEXCONTROL_INVERSE;
              digitalWrite(LEDpin,HIGH);
              
          } else {
              FlexControlMode = FLEXCONTROL_REGULAR;
              digitalWrite(LEDpin,LOW);
          }
          delay(200);
    }
    */
    
    // Act on buttons
   
   
   
    // Was a button pressed...
    for (int index = 0; index < NUMBUTTONS; index++) { 
      
      if (buttonState[index] == LOW) {
        
            /*if(index==0){
              Serial.println("Button 1 was pressed.");
                digitalWrite(LEDpin, HIGH);
              
              
            } else if (index == 1){
                Serial.println("Button 2 was pressed.");
                //digitalWrite(buttons[1], HIGH);

                digitalWrite(LEDpin, LOW);
            }
    
            delay(300);
            */
        
        
        // What screen are we on...
        

          if (screenState == MP3){
            
            screenState = menuMP3(index);
            
          } else if (screenState == MP3Volume){
            
            screenState = menuMP3Volume(index);
            
          } else if (screenState == Pedometer) {
            
            screenState = menuPedometer(index);
            
          } else if (screenState == RestTimer) {
            
            screenState = menuRestTimer(index);
            
          } else if (screenState == RestTimerEDIT) {
            
            screenState = menuRestTimerEDIT(index);
            
          } else if (screenState == ECG){
            
            screenState = menuECG(index);
            
          } else if (screenState == ECGAlerts){
            
            screenState = menuECGAlerts(index);
            
          } else if (screenState == ECGAlertsEDIT){
            
            screenState = menuECGAlertsEDIT(index);
          } 
          
  
          
        }   // End of 'a button was pressed' code
        
    }  // End of check for button presses
    
    
    // Main Loop Delay
    //delay(30);
    
}  // End of main loop



    /*
        
        if (index == 0 && !ECG_mode && !MP3player.isPlaying()) {
          Serial.println("Button 0");
          
          //LCD.clearScreen();
          
          
          
          
          
          
          delay(500);
          
        } else if (index == 1 && ECG_mode == false && !MP3player.isPlaying()) {
          Serial.println("Button 1");
          //LCD.comm_out(0xA5);   //Display all points ON
          
      
      
      
      
      
          delay(500);
        } else
        
        if (index == 0 && !ECG_mode && MP3player.isPlaying()) {
          
            // Button 0 pressed (Increase Volume) 
            curVolume = IncreaseVolume(0);
            
        } else if (index == 1 && !ECG_mode && MP3player.isPlaying()) {  
          
          // Button 1 pressed (Decrease Volume) 
          curVolume = DecreaseVolume(0);

        } else if (index == 2 && !ECG_mode) {             // Hear Rate Monitor ON
              ECG_mode = true;
              volStep = 1;
              
              
              //LCD.print("HRM enabled");        
              delay(500);
              
              //sets up to read Pulse Sensor signal every 2mS 
              interruptSetup();
              
          } else if (index == 2 && ECG_mode) {              // Hear Rate Monitor OFF
            ECG_mode = false;
            volStep = 4;
            //clearScreen();
            //LCD.print("HRM disabled");
            delay(500);
            
            interruptRestore();
            
            //MP3player.resumeDataStream();
          }
      } 
    } // End of Act on Buttons
    
    */
   // End of Main Loop









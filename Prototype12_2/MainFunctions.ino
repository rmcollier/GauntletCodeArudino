
int IncreaseVolume(int engaging) { //If engaging is 1, flex sensor is engaged and we (ASSUME) we have not reached MaxVolume.
    union twobyte mp3_vol; // create key_command existing variable that can be both word and double byte of left and right.
    mp3_vol.word = MP3player.getVolume(); // returns a double uint8_t of Left and Right packed into int16_t
    
    mp3_vol.byte[1] -= volStep;
    
    if (mp3_vol.byte[1] <= 2) { // range check. Circuit protection here. Not just limit to MaxVolume.
        mp3_vol.byte[1] = 2;
    }
    
    // If IncreaseVolume was called by button push, need to adjust appropriate volume limit
    if (!engaging){
          if (VolumeGesture == DISENGAGED) {
            MinVolume -= volStep ; // Lowering this value increases the volume
          } else if (VolumeGesture == ENGAGED) {
            MaxVolume -= volStep;
          }
    }
    
    // push byte[1] into both left and right assuming equal balance.
    MP3player.setVolume(mp3_vol.byte[1], mp3_vol.byte[1]); // commit new volume
    
    
    return mp3_vol.byte[1];
}



int DecreaseVolume(int reset) { // if reset is zero, we know this function is called by button input
    union twobyte mp3_vol; // create key_command existing variable that can be both word and double byte of left and right.
    mp3_vol.word = MP3player.getVolume(); // returns a double uint8_t of Left and Right packed into int16_t
      
      // Decrease volume as called by grip release (return to MinVolume)
      if (reset) {
        mp3_vol.byte[1] = reset;
        digitalWrite(LEDpin,LOW);
      } else {  
        
        // Decrease volume as called by buttons (decrease and adjust the appropriate flexSensor volume limit.
        // assume equal balance and use byte[1] for math
        if (mp3_vol.byte[1] >= 254) { // range check
          mp3_vol.byte[1] = 254;
        } else {
          mp3_vol.byte[1] += volStep; // 2 is lowest resolution  of one full dB. 4 is two dB's.
        }
            
            
        // Adjust new volume limit, based on flexSensor
        if (VolumeGesture == DISENGAGED) {
          MinVolume = mp3_vol.byte[1];
        } else {
          MaxVolume = mp3_vol.byte[1];
        }
      }

    // push byte[1] into both left and right assuming equal balance.
    MP3player.setVolume(mp3_vol.byte[1], mp3_vol.byte[1]); // commit new volume
    
    
    //  toggleLED(BLUE);
    
    
    return mp3_vol.byte[1];
}

/*
int IncreaseVolume(int engaging) { //If engaging is 1, flex sensor is engaged and we (ASSUME) we have not reached MaxVolume.
    union twobyte mp3_vol; // create key_command existing variable that can be both word and double byte of left and right.
    mp3_vol.word = MP3player.getVolume(); // returns a double uint8_t of Left and Right packed into int16_t
    
    if (curVolume <= MaxVolume) { // range check
        curVolume = MaxVolume;
        Serial.print("Limit: mp3_vol.byte[1] <= MaxVolume "); Serial.println(mp3_vol.byte[1]);
    } else {
        Serial.print("Increasing to ");
        curVolume -= volStep;
        Serial.println(curVolume);
    }
    
    // If IncreaseVolume was called by button push, need to adjust appropriate volume limit
    if (!engaging){
          if (VolumeGesture == DISENGAGED) {
            MinVolume -= volStep ; // Lowering this value increases the volume
            Serial.print("New MinVolume = "); Serial.println(MinVolume);
          } else if (VolumeGesture == ENGAGED) {
            MaxVolume -= volStep;
            Serial.print("New MaxVolume = "); Serial.println(MaxVolume);
          }
          delay(300);
    }
    
    Serial.print("pushing new value: "); Serial.println(curVolume);
    mp3_vol.byte[1] = curVolume;
    // push byte[1] into both left and right assuming equal balance.
    MP3player.setVolume(mp3_vol.byte[1], mp3_vol.byte[1]); // commit new volume
    Serial.print("Confirming: "); Serial.println(mp3_vol.byte[1]);
    
    
    return curVolume;   
    // Does this function really need to return a value if the global variable curVolume is being changed within it?
}






int DecreaseVolume(int reset) { // if reset is zero, we know this function is called by button input
    union twobyte mp3_vol; // create key_command existing variable that can be both word and double byte of left and right.
    mp3_vol.word = MP3player.getVolume(); // returns a double uint8_t of Left and Right packed into int16_t
      
      
      // Decrease volume as called by grip release (return to MinVolume)
      if (reset) {
        mp3_vol.byte[1] = reset; // Assuming we are receiving MinVolume here.
        digitalWrite(LEDpin,LOW);
        Serial.println("Reset");
      } else {  
        
        // Decrease volume as called by buttons (decrease and adjust the appropriate flexSensor volume limit.
        // assume equal balance and use byte[1] for math
        if (mp3_vol.byte[1] >= MinVolume) { // range check
        Serial.print("Limit: mp3_vol.byte[1] >= MinVolume "); Serial.println(mp3_vol.byte[1]);
          mp3_vol.byte[1] = MinVolume;
        } else {
          mp3_vol.byte[1] += volStep; // 2 is lowest resolution  of one full dB. 4 is two dB's.
          Serial.print("Decreasing to "); Serial.print(mp3_vol.byte[1]);
        }
            
            
        // Adjust new volume limit, based on flexSensor
        if (VolumeGesture == DISENGAGED) {
          MinVolume = mp3_vol.byte[1];
        } else {
          MaxVolume = mp3_vol.byte[1];
        }
      }
    
     Serial.print("pushing new value: "); Serial.println(mp3_vol.byte[1]);
    // push byte[1] into both left and right assuming equal balance.
    MP3player.setVolume(mp3_vol.byte[1], mp3_vol.byte[1]); // commit new volume
        
    
    //  toggleLED(BLUE);
    
    
    return mp3_vol.byte[1];
}



*/




void ToggleGestureControlMode() {
  
  if (FlexControlMode == FLEXCONTROL_REGULAR) {
    FlexControlMode = FLEXCONTROL_INVERSE;
    //toggleLED(YELLOW);
  } else {
    FlexControlMode = FLEXCONTROL_REGULAR;
    //toggleLED(BLUE);
  }
  
  delay(100);
}
    
    
    /*
void toggleLED(int input){
  if (input == YELLOW)
  {
    registerState = registerState | B10000000;  // yellow On
    registerState = registerState & B10111111;   // blue off
    LEDindicator = true;
  } else if (input == BLUE){
    registerState = registerState & B01111111;  // yellow off
    registerState = registerState | B01000000;  // blue on
    LEDindicator = false;
  }
  changeShiftReg(registerState);
  delay(100);
}*/
    
/*
void yellowLED(boolean state) {
  if (state){
      registerState = registerState | B10000000;
  } else {
    registerState = registerState & B01111111;
  }
  changeShiftReg(registerState);
}

void blueLED(boolean state) {
if (state){
      registerState = registerState | B01000000;
  } else {
    registerState = registerState & B10111111;
  }
  changeShiftReg(registerState);
}*/

void toggleDisplayLED() {
if (!displayLEDstate){
      digitalWrite(LCD_backlight, HIGH);
      displayLEDstate = true;
  } else if (displayLEDstate){
    digitalWrite(LCD_backlight, LOW);
    displayLEDstate = false;
  }
  delay(150);
}


/*
void changeShiftReg(byte regStates) {

    SPI.setDataMode(SPI_MODE0);
    SPI.setClockDivider(SPI_CLOCK_DIV4);
    
    // 74HC595 output 1 turns on the screen's power
    SPI.setBitOrder(LSBFIRST);
    //registerState = registerState & B01;
    digitalWrite(latchPin, LOW);
    SPI.transfer(regStates);
    digitalWrite(latchPin, HIGH);
    delay(200);

}
*/


//------------------------------------------------------------------------------
/**
 * \brief Decode the Menu.
 *
 * Parses through the characters of the users input, executing corresponding
 * MP3player library functions and features then displaying a brief menu and
 * prompting for next input command.
 */
 
 /*
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
  if (key_command == 's') {
    MP3player.stopTrack();
    
  //if 1-9, play corresponding track
  } else if (key_command >= '1' && key_command <= '9') {
    //convert ascii numbers to real numbers
    key_command = key_command - 48;

    //tell the MP3 Shield to play a track
    result = MP3player.playTrack(key_command);

    //check result, see readme for error codes.
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to play track"));
    }

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

  //if +/- to change volume
  } else if ((key_command == '-') || (key_command == '+')) {
    union twobyte mp3_vol; // create key_command existing variable that can be both word and double byte of left and right.
    mp3_vol.word = MP3player.getVolume(); // returns a double uint8_t of Left and Right packed into int16_t

    if (key_command == '-') { // note dB is negative
      // assume equal balance and use byte[1] for math
      if (mp3_vol.byte[1] >= 254) { // range check
        mp3_vol.byte[1] = 254;
      } else {
        mp3_vol.byte[1] += 2; // keep it simpler with whole dB's
        MinVolume = mp3_vol.byte[1];
      }
    } else {
      if (mp3_vol.byte[1] <= 2) { // range check
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
  } else if ((key_command == '>') || (key_command == '<')) {
    uint16_t playspeed = MP3player.getPlaySpeed(); // create key_command existing variable
    // note playspeed of Zero is equal to ONE, normal speed.
    if (key_command == '>') { // note dB is negative
      // assume equal balance and use byte[1] for math
      if (playspeed >= 254) { // range check
        playspeed = 5;
      } else {
        playspeed += 1; // keep it simpler with whole dB's
      }
    } else {
      if (playspeed == 0) { // range check
        playspeed = 0;
      } else {
        playspeed -= 1;
      }
    }
    MP3player.setPlaySpeed(playspeed); // commit new playspeed
    Serial.print(F("playspeed to "));
    Serial.println(playspeed, DEC);
    

  /* Alterativly, you could call a track by it's file name by using playMP3(filename);
  But you must stick to 8.1 filenames, only 8 characters long, and 3 for the extension 
  } else if (key_command == 'f') {
    //create a string with the filename
    char trackName[] = "track001.mp3";

    //tell the MP3 Shield to play that file
    result = MP3player.playMP3(trackName);
    //check result, see readme for error codes.
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to play track"));
    }
/*
   //Display the file on the SdCard 
  } else if (key_command == 'd') {
    if(!MP3player.isPlaying()) {
      // prevent root.ls when playing, something locks the dump. but keeps playing.
      // yes, I have tried another unique instance with same results.
      // something about SdFat and its 500byte cache.
      Serial.println(F("Files found (name date time size):"));
      SFEMP3Shield::root.ls(LS_R | LS_DATE | LS_SIZE);
    } else {
      Serial.println(F("Busy Playing Files, try again later."));
    }

  /* Get and Display the Audio Information 
  } else if (key_command == 'i') {
    MP3player.getAudioInfo();

  } else if (key_command == 'p') {
    MP3player.pauseDataStream();
    Serial.println(F("Pausing"));

  } else if (key_command == 'r') {
    MP3player.resumeDataStream();
    Serial.println(F("Resuming"));
    //clearScreen();
    //LCD.print("Resuming");
    //delay(1500);
    //clearScreen();

  } else if (key_command == 'R') {
    MP3player.stopTrack();
    MP3player.vs_init();
    Serial.println(F("Reseting VS10xx chip"));

  } else if (key_command == 't') {
    int8_t teststate = MP3player.enableTestSineWave(126);
    if (teststate == -1) {
      Serial.println(F("Un-Available while playing music."));
    } else if (teststate == 1) {
      Serial.println(F("Enabling Test Sine Wave"));
    } else if (teststate == 2) {
      MP3player.disableTestSineWave();
      Serial.println(F("Disabling Test Sine Wave"));
    }

  } else if (key_command == 'm') {
      uint16_t teststate = MP3player.memoryTest();
    if (teststate == -1) {
      Serial.println(F("Un-Available while playing music."));
    } else if (teststate == 2) {
      teststate = MP3player.disableTestSineWave();
      Serial.println(F("Un-Available while Sine Wave Test"));
    } else {
      Serial.print(F("Memory Test Results = "));
      Serial.println(teststate, HEX);
      Serial.println(F("Result should be 0x83FF."));
      Serial.println(F("Reset is needed to recover to normal operation"));
    }

  } else if (key_command == 'e') {
    uint8_t earspeaker = MP3player.getEarSpeaker();
    if (earspeaker >= 3){
      earspeaker = 0;
    } else {
      earspeaker++;
    }
    MP3player.setEarSpeaker(earspeaker); // commit new earspeaker
    Serial.print(F("earspeaker to "));
    Serial.println(earspeaker, DEC);

  } else if (key_command == 'M') {
    uint16_t monostate = MP3player.getMonoMode();
    Serial.print(F("Mono Mode "));
    if (monostate == 0) {
      MP3player.setMonoMode(1);
      Serial.println(F("Enabled."));
    } else {
      MP3player.setMonoMode(0);
      Serial.println(F("Disabled."));
    }

  } else if (key_command == 'g') {
    int32_t offset_ms = 20000; // Note this is just an example, try your own number.
    Serial.print(F("Skipping = "));
    Serial.print(offset_ms, DEC);
    Serial.println(F("[milliseconds]"));
    result = MP3player.skipTo(offset_ms);
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to skip track"));
    }

  } else if (key_command == 'h') {
    help();
    
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

  }

  // print prompt after key stroke has been processed.
  Serial.println(F("Enter 1-9,s,d,+,-,i,>,<,p,r,R,t,m,M,g,h :"));
}


*/

//------------------------------------------------------------------------------
/**
 * \brief Print Help Menu.
 *
 * Prints a full menu of the commands available along with descriptions.
 
 
 
void help() {
  Serial.println(F("Arduino SFEMP3Shield Library Example:"));
  Serial.println(F(" courtesy of Bill Porter & Michael P. Flaga"));
  Serial.println(F("COMMANDS:"));
  Serial.println(F(" [1-9] to play a track"));
  Serial.println(F(" [s] to stop playing"));
  Serial.println(F(" [+ or -] to change volume"));
  Serial.println(F(" [> or <] to increment or decrement play speed by 1 factor"));
  Serial.println(F(" [i] retrieve current audio information (partial list)"));
  Serial.println(F(" [e] increment Spatial EarSpeaker, default is 0, wraps after 4"));
  Serial.println(F(" [p] to pause."));
  Serial.println(F(" [r] to resume."));
  Serial.println(F(" [R] Resets and initializes VS10xx chip."));
  Serial.println(F(" [t] to toggle sine wave test"));
  Serial.println(F(" [m] perform memory test. reset is needed after to recover."));
  Serial.println(F(" [M] Toggle between Mono and Stereo Output."));
  Serial.println(F(" [g] Skip to a predetermined offset of ms in current track."));
  Serial.println(F(" [h] this help"));
}


*/



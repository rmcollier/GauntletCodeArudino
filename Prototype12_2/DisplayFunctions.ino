
/* Display Menu pages, dependent on device *_mode booleans:
    ECG_mode
    MP3_mode
    Pedometer_mode
    Rest_mode
    ECGalert_mode
*/



int menuMP3() {
  // For explaination on PROGMEM:
  // http://www.arduino.cc/en/Reference/PROGMEM
  
  
  LCD.comm_out(PAGE0);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));  // Does this actually work?
  
  memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[0])), sizeof(caption_MP3_)/sizeof(caption_MP3_[0]));
  LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));

  LCD.comm_out(PAGE3);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  
  // Menu
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[6])),sizeof(buttons_Menu_)/sizeof(buttons_Volume_[0]));
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  
  if (MP3_mode){
    // Pause
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[7])),sizeof(buttons_Pause_)/sizeof(buttons_Pause_[0]));
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  } else {
    // Play
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[8])),sizeof(buttons_Play_)/sizeof(buttons_Play_[0]));
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  }
  // Volume
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[9])),sizeof(buttons_Volume_)/sizeof(buttons_Volume_[0]));
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
    
  return MP3;
}


int menuMP3(int button) {
  // Which button was pressed...
  if (button == 2){                 // Menu
    
    return menuPedometer();
              
  } else if (button == 1){        // Play / Pause
     if (!MP3_mode){
         // MP3 player is paused or stopped. Unpause playback or start first track...
         if (!trackSelect) {
           curVolume = DecreaseVolume(MinVolume);
           trackSelect = 1;
           MP3player.playTrack(trackSelect);
         } else {
           //MP3player.resumeMusic();
         }
         //MP3player.playMP3withTimecode(trackSelect, pausePosition, pauseByteRate);
         RestTimer_unpauseMP3();
         
         MP3_mode = true;
         //Serial.println("MP3 Play");

         
     } else {    //               
         
         if (MP3player.isPlaying()) {
           /// ***** pausePosition needs to be reset to zero once the song ends or track is changed!
           //pausePosition += MP3player.currentPosition();   // this is timecode in milliseconds
           //pauseByteRate = MP3player.GetMP3ByteRate();    // Should be Bytes per millisecond

           RestTimer_pauseMP3();
           // Debugging:
           /*
           Serial.println("Pausing.");
           Serial.print(F("Track No")); Serial.print(F("\ttimecode"));Serial.println(F("\tbyte rate"));
           Serial.print(trackSelect);
           Serial.print(F("\t\t0x"));
           Serial.print(pausePosition);
           Serial.print(F("\t\t\t0x"));
           Serial.println(pauseByteRate);
           MP3player.getAudioInfo();
           */
           //MP3player.stopTrack();
           //MP3player.pauseMusic();
         }
         
         // This will need to be updated to capture pausePosition
         // remove MP3 icon
         
         MP3_mode = false;
         //Serial.println("MP3 Pause");
     }  
      return menuMP3();
      
  } else if (button == 0){   // Volume
    // Initiate flexSensor polling to determine high vs low volume setting focus
    
    // Debugging:
    //MP3player.getAudioInfo();
    //MP3player.skipTo(60000);  This does work.
    
    // volumeSeek = true;
    // return menuMP3Volume();
    return menuMP3Volume();
  }
}



int menuPedometer() {

  LCD.comm_out(PAGE0);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  
  memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));  
  
  memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[2])), sizeof(caption_Pedometer_)/sizeof(caption_Pedometer_[0]));
  LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
  
  LCD.comm_out(PAGE3);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  // Menu
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[6])),sizeof(buttons_Menu_)/sizeof(buttons_Volume_[0]));    // Menu
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  
  if (Pedometer_mode){
    // Stop
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[15])),sizeof(buttons_Stop_)/sizeof(buttons_Stop_[0]));   
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  } else {
    // Start
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[14])),sizeof(buttons_Start_)/sizeof(buttons_Start_[0]));    
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  }
   // Reset
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[13])),sizeof(buttons_Reset_)/sizeof(buttons_Reset_[0]));       
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
    
    
  return Pedometer;
}


int menuPedometer (int button) {
  if (button == 2) {
    // Next Menu

    return menuRestTimer();

  } else if (button == 1){
    // On / Off
    if (!Pedometer_mode){
      Pedometer_mode = true;
      
      
      // Display stepCount
      memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0])); 
      memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[21])), sizeof(small_numbers_)/sizeof(small_numbers_[0]));
      LCD.displaySmallDigits(stepCount, captionBuffer, PAGE0, 0x15, 0x09);
      
      // Display icon_run_
      memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));  
      memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[22])),sizeof(icon_run_)/sizeof(icon_run_[0]));       
      LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));


    } else { 
      
      Pedometer_mode = false; 
      
      // Clear pedometer count from display
          LCD.comm_out(PAGE0); LCD.comm_out(0x15); LCD.comm_out(0x09);
          for (int i=0; i<35; i++){
            LCD.data_out(0x00);
          }
      
    }
    
    return menuPedometer();
    
  } else if (button == 0) {
    // Reset Pedometer?
    stepCount = 0;
    
    memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));  
    memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[21])), sizeof(small_numbers_)/sizeof(small_numbers_[0]));
    LCD.displaySmallDigits(0,captionBuffer, PAGE0, 0x15, 0x09);
    
    return Pedometer;
  }
}



int menuMP3Volume(){
  LCD.comm_out(PAGE0);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));  
  memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[1])), sizeof(caption_MP3volume_)/sizeof(caption_MP3volume_[0]));
  LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
  
  LCD.comm_out(PAGE3);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[10])), sizeof(buttons_Return_)/sizeof(buttons_Return_[0]));    // Return
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  
   memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[12])), sizeof(buttons_down_)/sizeof(buttons_down_[0]));    // Down
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[11])), sizeof(buttons_up_)/sizeof(buttons_up_[0]));    // Up
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  
 
  

  
  if (flexSensor == HIGH) {
    LCD.comm_out(PAGE2);
    LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
    memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));    
    memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[18])), sizeof(volumeSeek_low_)/sizeof(volumeSeek_low_[0]));
    LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
  } else {      
    LCD.comm_out(PAGE2);
    LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
    memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));    
    memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[19])), sizeof(volumeSeek_high_)/sizeof(volumeSeek_high_[0]));
    LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
  }
  
  return MP3Volume;
}


int menuRestTimer() {
  char NewCaptionBuffer[60] = {0};    
  // For the love of god, I don't know why the 'Display clock icon' code of this function needs a NewCaptionBuffer[] 
  // to display the icon_clock_. The global captionBuffer has been successfully reused for all LCD.displayString methods prior
  // to it. I seriously don't know.
  
  LCD.comm_out(PAGE0);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  
  // Display caption
  memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));  
  memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[3])), sizeof(caption_RestTimer_)/sizeof(caption_RestTimer_[0]));
  LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
  
  LCD.comm_out(PAGE3);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[6])), sizeof(buttons_Menu_)/sizeof(buttons_Menu_[0]));    // Menu
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  
  if (Rest_mode){
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[15])), sizeof(buttons_Stop_)/sizeof(buttons_Stop_[0]));    // Stop
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  } else {
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[14])), sizeof(buttons_Start_)/sizeof(buttons_Start_[0]));    // Start
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  }
  
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[16])), sizeof(buttons_Edit_)/sizeof(buttons_Edit_[0]));    // edit
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  
  
  // Display restTimeSec
  memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
  memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[21])), sizeof(small_numbers_)/sizeof(small_numbers_[0]));
  LCD.displaySmallDigits(restTimeSec, captionBuffer, PAGE1, 0x15, 0x09);
  //delay(40);
  
  // Dislay clock icon
  //memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
  memcpy_P(NewCaptionBuffer, (char*)pgm_read_word(&(text_table[23])), sizeof(icon_clock_)/sizeof(icon_clock_[0]));
  LCD.displayString(NewCaptionBuffer, sizeof(NewCaptionBuffer)/sizeof(NewCaptionBuffer[0]));
  
  
  return RestTimer;
}


int menuRestTimer (int button) {
  if (button == 2) {
    
    if (!Rest_mode) {
        // Erase rest timer value displayed
        LCD.comm_out(PAGE1); LCD.comm_out(0x15); LCD.comm_out(0x09);
        for (int i =0; i < 35; i++)
        {
          LCD.data_out(0x00);
        }
    }
    
    return menuECG();

  } else if (button == 1){
    if (!Rest_mode){
      Rest_mode = true;
      RestTimer_readyStart = false;
      RestTimer_hasPlayed = true;
      
      //erial.println(F("Rest Timer on."));
      
    } else { 
      Rest_mode = false;
      //Serial.println(F("Rest Timer off."));
    }
    return menuRestTimer();
    
  } else if (button == 0) {
    // Edit Rest Timer
    return menuRestTimerEDIT();
  }
}


int menuRestTimerEDIT() {
    LCD.comm_out(PAGE1);
    LCD.comm_out(0x10); LCD.comm_out(0x04);
    
    memset (captionBuffer,0,60);
    memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[5])), sizeof(caption_EDIT_)/sizeof(caption_EDIT_[0]));
    LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
    
    
    /*
    memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[20])), sizeof(large_numbers_)/sizeof(large_numbers_[0]));
    LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
    LCD.displaySmallDigits(restTimeSec/1000, captionBuffer, PAGE1, 0x15, 0x09);
    */
    
    LCD.comm_out(PAGE3);
    LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
    
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[10])), sizeof(buttons_Return_)/sizeof(buttons_Return_[0]));    // Return
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
    
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[12])), sizeof(buttons_down_)/sizeof(buttons_down_[0]));    // Down
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
    
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[11])), sizeof(buttons_up_)/sizeof(buttons_up_[0]));    // Up
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
    
    
  return RestTimerEDIT;
}

int menuRestTimerEDIT(int button) {
  if (button == 2) {
    // Menu
    
    // Remove 'EDIT' caption
    LCD.comm_out(PAGE1); LCD.comm_out(0x10); LCD.comm_out(0x04);
    for (int i=0; i < 25; i++)
    {
      LCD.data_out(0x00);
    }
    
    return menuRestTimer();
  
  } else if (button == 1 && restTimeSec != 0){
    // Down
   
    restTimeSec -= 10;
    memset (captionBuffer,0,sizeof(captionBuffer)/captionBuffer[0]);
    memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[21])), sizeof(small_numbers_)/sizeof(small_numbers_[0]));
    LCD.displaySmallDigits(restTimeSec, captionBuffer, PAGE1, 0x15, 0x09);
    
    delay(200);
    return RestTimerEDIT;
    
  } else if (button == 0) {
    // Up
    
    restTimeSec += 10;
    memset (captionBuffer,0,sizeof(captionBuffer)/captionBuffer[0]);
    memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[21])), sizeof(small_numbers_)/sizeof(small_numbers_[0]));
    LCD.displaySmallDigits(restTimeSec, captionBuffer, PAGE1, 0x15, 0x09);
    
    delay(200);
    return RestTimerEDIT;
  }
}



int menuECG(){
  //LCD.displayCaption(caption_ECG_,sizeof(caption_ECG_)/sizeof(caption_ECG_[0]));
  
  LCD.comm_out(PAGE0);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  
  memset (captionBuffer,0,sizeof(captionBuffer)/sizeof(captionBuffer[0]));  
  
  memcpy_P(captionBuffer, (char*)pgm_read_word(&(text_table[4])), sizeof(caption_Pedometer_)/sizeof(caption_Pedometer_[0]));
  LCD.displayString(captionBuffer,sizeof(captionBuffer)/sizeof(captionBuffer[0]));
  
  LCD.comm_out(PAGE3);
  LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[6])), sizeof(buttons_Menu_)/sizeof(buttons_Menu_[0]));    // Menu
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  
  if (ECG_mode){
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[15])), sizeof(buttons_Stop_)/sizeof(buttons_Stop_[0]));    // Stop
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  } else {
    memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[14])), sizeof(buttons_Start_)/sizeof(buttons_Start_[0]));    // Start
    LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));
  }
  
  memcpy_P(buttonBuffer, (char*)pgm_read_word(&(text_table[17])), sizeof(buttons_Alerts_)/sizeof(buttons_Alerts_[0]));    // alerts
  LCD.displayString(buttonBuffer,sizeof(buttonBuffer)/sizeof(buttonBuffer[0]));

  return ECG;
}


int menuECG(int button) {
    if (button == 2) {
    // Menu
    return menuMP3();  // Homescreen?

  } else if (button == 1){
    // Toggle
    /* Heart Rate sensor disabled for now
    if (!ECG_mode){
      ECG_mode = true;
    } else { ECG_mode = false; }
    */
    
    
    toggleDisplayLED(); // Toggle
    
    return ECG;
    
  } else if (button == 0) {
      // Alerts
      
    //ToggleGestureControlMode();
    //GCModeRefresh = true;
      
    return menuECGAlerts();
  }
}






int menuECGAlerts(int button){
    if (button == 2) {
    // Return
    return MP3;  // Homescreen?

  } else if (button == 1){
    // Toggle

    
    return ECG;
    
  } else if (button == 0) {
      // Edit
    return menuECGAlertsEDIT();
  }
}

int menuECGAlertsEDIT(int button) {
    if (button == 2) {
    // Set / Return
    
    return menuECGAlerts();  // Homescreen?

  } else if (button == 1){
    // Up
    
    
    // interruptSetup();   PULSE SENSOR ON
    
    // interruptRestore();     PULSE SENSOR OFF
    
    return ECG;
    
  } else if (button == 0) {
      // Down
      
    return menuECGAlertsEDIT();
  }
}

int menuECGAlerts() {
  //LCD.displayCaption(caption_ECG_,sizeof(caption_ECG_)/sizeof(caption_ECG_[0]));
  
  LCD.comm_out(PAGE3);
    LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
    
//    LCD.displayButtons(buttons_Return_,sizeof(buttons_Return_)/sizeof(buttons_Return_[0]));
//    LCD.displayButtons(buttons_toggle_,sizeof(buttons_toggle_)/sizeof(buttons_toggle_[0]));
//    LCD.displayButtons(buttons_Edit_,sizeof(buttons_Edit_)/sizeof(buttons_Edit_[0]));
//    

  return ECGAlerts;
}



int menuECGAlertsEDIT() {
  //LCD.displayCaption(caption_ECG_,sizeof(caption_ECG_)/sizeof(caption_ECG_[0]));
  //LCD.displayCaption(caption_EDIT_,sizeof(caption_EDIT_)/sizeof(caption_EDIT_[0]));
  

  return ECGAlertsEDIT;
}



void RestTimer_pauseMP3(){
    pausePosition += MP3player.currentPosition();   // this is timecode in milliseconds
    pauseByteRate = MP3player.GetMP3ByteRate();    // Should be Bytes per millisecond
    MP3player.stopTrack();
}

void RestTimer_unpauseMP3(){
  MP3player.playMP3withTimecode(trackSelect, pausePosition, pauseByteRate);
}




int menuMP3Volume(int button) {
  if (button == 2) {
    // Return
    
    // Clear PAGE2 text
    LCD.comm_out(PAGE2);
    LCD.comm_out(0x10); LCD.comm_out(0x04);	// Beginning of page
    for (int i=0;i<80;i++) {
      LCD.data_out(0x00);
    }
    
    return menuMP3();

  } else if (button == 1){
    // Volume down
    curVolume = DecreaseVolume(0);
    Serial.print("Volume: "); Serial.println(MP3player.getVolume());
    return MP3Volume;
    
  } else if (button == 0) {
    // Volume up
    curVolume = IncreaseVolume(0);
    Serial.print("Volume: "); Serial.println(MP3player.getVolume());
    return MP3Volume;
  }

}
  


  
  
  



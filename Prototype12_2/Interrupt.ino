


volatile int rate[10];                    // used to hold last ten IBI values
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find the inter beat interval
volatile int P =512;                      // used to find peak in pulse wave
volatile int T = 512;                     // used to find trough in pulse wave
volatile int thresh = 512;                // used to find instant moment of heart beat
volatile int amp = 100;                   // used to hold amplitude of pulse waveform
volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = true;       // used to seed rate array so we startup with reasonable BPM


void interruptSetup(){     
    //Initializes Timer1 to throw an interrupt every 2mS.
    cli();
    TCCRtemp[0] = TCCR1A;
    TCCRtemp[1] = TCCR1B;
    TCCRtemp[2] = TCCR1C;
    TCCRtemp[3] = TIMSK1;
    TCCRtemp[4] = ICR1;

    
    TCCR1A = 0x00; // DISABLE OUTPUTS AND PWM ON DIGITAL PINS 9 & 10
    TCCR1B = 0x11; // GO INTO 'PHASE AND FREQUENCY CORRECT' MODE, NO PRESCALER
    TCCR1C = 0x00; // DON'T FORCE COMPARE
    TIMSK1 = 0x01; // ENABLE OVERFLOW INTERRUPT (TOIE1)            ***this was set to 0x01. Set to 0??
    ICR1 = 16000; // TRIGGER TIMER INTERRUPT EVERY 2mS
    sei(); // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED
    
    
    
  /* Initializes Timer2 to throw an interrupt every 2mS.
  
  // Save whatever these default register bytes are for switching between EKG/MP3 modes:
  TCCRtemp[0] = TCCR2A;
  TCCRtemp[1] = TCCR2B;
  TCCRtemp[2] = OCR2A;
  TCCRtemp[3] = TIMSK2;
  
  TCCR2A = 0x02;     // DISABLE PWM ON DIGITAL PINS 3 AND 11, AND GO INTO CTC MODE
  TCCR2B = 0x06;     // DON'T FORCE COMPARE, 256 PRESCALER 
  OCR2A = 0X7C;      // SET THE TOP OF THE COUNT TO 124 FOR 500Hz SAMPLE RATE
  TIMSK2 = 0x02;     // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A
  sei();             // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED      
*/
    
}

/***** Timer/Counter Control Registers:

http://www.mythic-beasts.com/~markt/ATmega-timers.html

At restart the Arduino bootloader sets up the timers ready for analogWrite() PWM use. 
The timer/counters are set to phase-correct 8-bit PWM mode and with clock prescaled to system clock divided by 64. 
However timer0 is set to fast PWM mode for use with the millis()/micros() code.

For standard Arduinos the system clock is 16MHz so that the timers are clocking at 250kHz by default. 
Phase correct 8-bit PWM mode takes 510 clocks to cycle and fast 8-bit PWM mode takes 256. This means a frequency of 
490Hz for pins 5 and 6 and 977Hz for pins 3, 9, 10, 11 when used with analogWrite(). It is possible to change the 
clock prescaling to change these default PWM frequencies.

******/

void interruptRestore(){
    cli();
    TCCR1A = TCCRtemp[0];
    TCCR1B = TCCRtemp[1];
    TCCR1C = TCCRtemp[2];
    TIMSK1 = TCCRtemp[3];
    ICR1 = TCCRtemp[4];
    sei();
}

// THIS IS THE TIMER INTERRUPT SERVICE ROUTINE. 
// Timer makes sure that we take a reading every 2 miliseconds
ISR(TIMER1_OVF_vect){                           // triggered when Timer overflows
//ISR(TIMER2_COMPA_vect){
    cli();                                      // disable interrupts while we do this
    Signal = analogRead(pulsePin);              // read the Pulse Sensor
    sampleCounter += 2;                         // keep track of the time in mS with this variable
    int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

    //  find the peak and trough of the pulse wave
    if(Signal < thresh && N > (IBI/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal < T){                        // T is the trough
            T = Signal;                         // keep track of lowest point in pulse wave 
        }
    }
      
    if(Signal > thresh && Signal > P){          // thresh condition helps avoid noise
        P = Signal;                             // P is the peak
       }                                        // keep track of highest point in pulse wave
    
  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
if (N > 250){                                   // avoid high frequency noise
  if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){        
    Pulse = true;                               // set the Pulse flag when we think there is a pulse
    //digitalWrite(yellowLED,HIGH);                // turn on pin 13 LED
    IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
    lastBeatTime = sampleCounter;               // keep track of time for next pulse
         
         if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
             firstBeat = false;                 // clear firstBeat flag
             return;                            // IBI value is unreliable so discard it
            }   
         if(secondBeat){                        // if this is the second beat, if secondBeat == TRUE
            secondBeat = false;                 // clear secondBeat flag
               for(int i=0; i<=9; i++){         // seed the running total to get a realisitic BPM at startup
                    rate[i] = IBI;                      
                    }
            }
          
    // keep a running total of the last 10 IBI values
    word runningTotal = 0;                   // clear the runningTotal variable    

    for(int i=0; i<=8; i++){                // shift data in the rate array
          rate[i] = rate[i+1];              // and drop the oldest IBI value 
          runningTotal += rate[i];          // add up the 9 oldest IBI values
        }
        
    rate[9] = IBI;                          // add the latest IBI to the rate array
    runningTotal += rate[9];                // add the latest IBI to runningTotal
    runningTotal /= 10;                     // average the last 10 IBI values 
    BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
    QS = true;                              // set Quantified Self flag 
    // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }                       
}

  if (Signal < thresh && Pulse == true){     // when the values are going down, the beat is over
      //digitalWrite(yellowLED,LOW);            // turn off pin 13 LED
      Pulse = false;                         // reset the Pulse flag so we can do it again
      amp = P - T;                           // get amplitude of the pulse wave
      thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
      P = thresh;                            // reset these for next time
      T = thresh;
     }
  
  if (N > 2500){                             // if 2.5 seconds go by without a beat
      thresh = 512;                          // set thresh default
      P = 512;                               // set P default
      T = 512;                               // set T default
      lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date        
      firstBeat = true;                      // set these to avoid noise
      secondBeat = true;                     // when we get the heartbeat back
     }
  
  sei();                                     // enable interrupts when youre done!
}// end isr


void sendDataToProcessing(char symbol, int data ){
    Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
    Serial.println(data);                // the data to send culminating in a carriage return
  }



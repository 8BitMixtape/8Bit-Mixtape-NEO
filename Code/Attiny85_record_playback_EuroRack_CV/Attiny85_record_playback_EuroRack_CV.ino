// repeats what you do
// ATtiny85 at 8 MHz 
//https://bitbucket.org/mrgrok/mrgroks-arduino-sketches/src/b2d52c93640c9f582ac0cd0e867c2a67107274f4/recordAndPlaybackAtRate/recordAndPlaybackAtRate.ino?fileviewer=file-view-default
#include "neolib.h"

const byte morseBtn = 4;
const byte modeBtn = 3;

uint8_t state = 0;

const byte led = 1; //ATtiny85
const byte sampleLength = 5; // 1 ms
const byte maxSamples = 128;

boolean inRecordMode = false;
boolean wasInRecordMode = false;

// buffers to record state/duration combinations
//boolean states[maxSamples];
uint8_t states[maxSamples];
uint8_t durations[maxSamples];
int currentSampleCycles;

short idxPlayback = 0;
short idxRecord = 0;

void setup() {

    cli();

  // Set up Timer/Counter1 31250Hz
  TCCR1 &= ~(7 << CS10); // make sure bits are empty
  TCCR1 |= 1 << CS11;    // set prescaler

  // pulse width modulator A enable
  TCCR1 &= ~(1 << COM1A0);       // make sure register is empty
  TCCR1 |=   1 << COM1A1 ;       // PWM mode: clear the OC1A output line on compare match
  
  //TCCR1 |= 1 << CS12 | 1 << CS10;    // set prescaler
  //PLLCSR &=~ 1<<PCKE; // slow clock
  
  TCCR1 |= 1 << PWM1A;

  //TIMSK |= 1 << OCIE1A;          // Enable compare match interrupt for timer 1

  OCR1A = 128;                   // initialize PWM to 50% mean value

  DDRB |= 1 << PB1; // PWM pin as output

  sei();
  
  //pinMode(modeBtn, INPUT);
  //pinMode(morseBtn, INPUT);
  pinMode(led, OUTPUT);
  pinMode(0, OUTPUT);
  neobegin();
  //setColorAllPixel(colorWheel(220));
  //pixels.show();
}

void resetForRecording() {
  memset(states, 0, sizeof(states));
  memset(durations, 0, sizeof(durations));
  idxRecord = 0; // reset record idx just to make playback start point obvious 
  
  idxPlayback=0; 
  currentSampleCycles=0;
}

void loop() {
  uint8_t b=getButton();
    if( b == BUTTON_RIGHT || b == BUTTON_LEFT + BUTTON_RIGHT) {
    inRecordMode = true;
    digitalWrite(0, 1);
  }
  else {
    inRecordMode = false;
    digitalWrite(0, 0);
  }
  
  if(inRecordMode == true) {
    if(!wasInRecordMode) {
      resetForRecording();      
    }
    recordLoop();
  } else {
    // continue playing loop
    playbackLoop();
  }
  
  wasInRecordMode = inRecordMode; // record prev state for next iteration so we know whether to reset the record arr index
}

void recordLoop() {
  //boolean state = digitalRead(morseBtn);
  int16_t cv = getPoti( POTI_RIGHT ) >> 2;        // range 0..1023
  uint8_t rl=getButton();
  if( rl == BUTTON_LEFT + BUTTON_RIGHT ) {
    state = cv;
    //analogWrite(led, state); // give feedback to person recording the loop
    OCR1A = state;
  }
  else {
    state = 0;
    //analogWrite(led, state); // give feedback to person recording the loop
    OCR1A = state;
  }
  
  if(states[idxRecord] == state) {
    // state not changed, add to duration of current state
    durations[idxRecord] += sampleLength;
  } else {
    // state changed, go to next idx and set default duration
    idxRecord++;
    if(idxRecord == maxSamples) { idxRecord = 0; } // reset idx if max array size reached
    states[idxRecord] = state;
    durations[idxRecord] = sampleLength;
  }
  
  delay(sampleLength); // slow the loop to a time constant so we can reproduce the timelyness of the recording
}

void playbackLoop()
{
  int16_t lp = getPoti( POTI_LEFT );        // range 0..1023
  if(currentSampleCycles == 0 && durations[idxPlayback] != 0) {
    // state changed
    //digitalWrite(led, states[idxPlayback]); // set led
    //analogWrite(led, states[idxPlayback]); // set led
    OCR1A = states[idxPlayback];
  }
  
  if(idxPlayback == maxSamples) { 
    // EOF recorded loop - repeat
    idxPlayback=0; 
    currentSampleCycles=0;
  } else {
  //  if(durations[idxPlayback]*playbackMultiplier <= currentSampleCycles) // speed ctls deactivated
    if(durations[idxPlayback]*1 <= currentSampleCycles) 
    {
      // EOF current sample in recorder buffer 
      idxPlayback++; // move to next sample 
      currentSampleCycles = 0; // reset for next sample
    } else {
      // still in same sample, but in next cycle (==sampleLength approx)
      currentSampleCycles++;
    }
  }
  
  //delay(sampleLength); // keep time same as we had for record loop (approximately) 
  delayMicroseconds(lp);
  //digitalWrite(0,HIGH);
  //delay(sampleLength/2);
  //digitalWrite(0,LOW);
  
}

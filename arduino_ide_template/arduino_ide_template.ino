
// Minimal sketch to demonstrate programming the Komorebi Kit hardware using the Arduino IDE and Mozzi library
// 
// Dieter Vandoren & Matteo Marangoni, 2021
//
// Project info & documentation: https://instrumentinventors.org/editions/iii-sensory-kit-komorebi-instructions/ 


// Mozzi synthesis library
#include <MozziGuts.h>
#include <Oscil.h> 
#include <tables/sin2048_int8.h> // sine wavetable for oscillator
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA); // create oscillator object
#define CONTROL_RATE 64 // 64Hz control rate 

// amplifier enable output pin
const int ampPin = 17;

// light sensor input pin 
const int lightPin = 34;

// variable to store light input value
int lightValue; 

// light input pre-amplification: gain is set using a digital potentiometer via SPI protocol
#include <SPI.h>
const int slaveSelectPin = 5;
int lightGain = 127;  // gain value setting, [0-255] range is reversed:
                      //    0 = maximum gain (twilight, candle light)
                      //    255 = minimum gain (direct sunlight)
                      // use serial monitor to check whether light input values are in a useful range
                      // the input ADC has 12 bit resolution meaning values can be between 0 and 4095 
                      // if input values are stuck at 0 turn up the gain by lowering the lightGain value
                      // if input values are stuck at 4095 turn down gain by raising the lightGain value
                  
// variable for sine oscillator frequency
float sineFreq = 600.f;


void setup(){
  // start serial interface
  Serial.begin(9600);
  while(!Serial){};

  // init light input gain control
  pinMode(slaveSelectPin, OUTPUT);
  SPI.begin();
  // set gain value
  digitalPotWrite(0, lightGain); 

  // light input
  pinMode(lightPin, INPUT);
  
  // amplifier enable, required if audio output via the amplifier and speaker circuit is to be used
  pinMode(ampPin, OUTPUT);
  digitalWrite(ampPin, HIGH);

  // start Mozzi synthesis process
  startMozzi(CONTROL_RATE); 

  // set initial sine oscillator frequency
  aSin.setFreq(sineFreq); 
}


void updateControl(){
  // slow control rate calculations in here: (do the heavy lifting here)

  // read & print light input
  lightValue = analogRead(lightPin);
  Serial.println(lightValue);

  // map light input [0, 4095] to sine frequency range [600, 2500]
  sineFreq = map(lightValue, 0, 4095, 600, 2500);

  // set new frequency
  aSin.setFreq(sineFreq);
}


int updateAudio(){
  // fast audio rate calculations in here: (keep this slim and fast, no serial printing!)
  
  int sig = aSin.next(); // get next sine wave value
  
  return sig << 4; // return calculated sample value to audio output buffer
                   // fast output amplitude modification by bitshifts: 
                   //   1 bitshift left/up doubles the amplitude 
                   //   1 bitshift right/down halves the amplitude  
                   // output must not exceed 16 bit maximum, ie. 65535, or digital clipping will occur
}


void loop(){
  audioHook(); // Call Mozzi audioHook() process in loop(). 
               // Put all other operations in the slower updateControl() function 
               // unless strictly necessary to run at loop() frequency, 
               // to save processor cycles for audio processing.
}


void digitalPotWrite(int address, int value) {
  // SPI interface to digital pot for setting light input gain
  
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin, LOW);
  delay(10);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  delay(10);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin, HIGH);
}

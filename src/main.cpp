//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "LarvaDefs.hpp"
#include "PhotoSensReader.hpp"
#include "LarvaSynth2.hpp"

LarvaSynth2 mSynth;
PhotoSensReader mPhotoSensReader;

void setup()
{
  #ifdef PRINT
  Serial.begin(9600);
  while(!Serial);
  #endif

  // amp enable
  pinMode(cAmpEnablePin, OUTPUT);
  digitalWrite(cAmpEnablePin, HIGH);
  
  mPhotoSensReader.Init();
  mSynth.Init();
  mSynth.Start();
}

void updateControl(){
  mPhotoSensReader.Update();  
  int vluxraw = mPhotoSensReader.GetLuxRaw();
  int vluxscaled = mPhotoSensReader.GetLuxScaled();
  mSynth.Update( vluxraw, vluxscaled );
}

int updateAudio(){
  return mSynth.Process();
  //return 0;
}

void loop(){  
  audioHook();
}
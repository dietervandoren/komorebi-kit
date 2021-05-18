//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "LarvaSynth2.hpp"


//---------------------------------------------------------------
float mFreq2GainTable[cFreq2GainTableSize];

static void ComputeFreq2GainTable(){
    // linear distribution (not so nice but)
    for (int n=0; n<cFreq2GainTableSize; ++n ){
        float vf = n * cFreq2GainTableStep;
        float vg = 1.f - fastPow( vf / cFreq2GainTableRange, cGainSlope ) * (1.f - cGainHF );
        mFreq2GainTable[n] = vg < cGainHF ? cGainHF : vg;
    }
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LARVA SYNTH

void LarvaSynth2::Init() 
{
  
  ComputeFreq2GainTable();

  mChords[0].Init(kChord1);
  mChords[0].SetLightRange(0,250);

  mChords[1].Init(kChord2);
  mChords[1].SetLightRange(200,450);
  
  mChords[2].Init(kChord3);
  mChords[2].SetLightRange(400,650);
  
  mChords[3].Init(kChord4);
  mChords[3].SetLightRange(600,850);
  
  mChords[4].Init(kChord5);
  mChords[4].SetLightRange(800,1050);
  
  mTriggersTimestamp = millis();

  randSeed();
  startMozzi(CONTROL_RATE);
}

int16_t LarvaSynth2::Process(){
    float vMix = 0.f;
    for ( int s = 0; s < mNumActiveChords; ++s ){
      vMix += mpActiveChords[s]->Process();
    }
    
    int16_t vout = ( (int16_t)( vMix * 32000.f ) );

    return vout;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LarvaSynth2::Update( const int acLightRaw, const int acLightScaled )
{
  // rolling average of RAW input input for delta detection 
  int vLightAvg = mLightRolling.next(acLightRaw);

  // slow mov avg on SCALED input for chords managing
  int vLightSlowAvg = mLightSlowRolling.next(acLightScaled);

  // calculate delta btw current reading and average
  int light_delta = abs( acLightRaw - vLightAvg ) * mDeltaScaler; 

  // light change trigger
  mTriggered = ( light_delta > cLightTriggerTreshold );
  
  if (mTriggered){
    mTriggersCounter++;
      //Serial.print("TRIGGER: light_delta = "); Serial.println( light_delta );
  }
  
  // average delta values
  int vdeltavg = mDeltaRolling.next(light_delta);

  // update delta scaler
  if ( vdeltavg <= cLightTriggerTreshold ) 
  {
    // if avg it's too small, increase the scaler
    mDeltaScaler = map( vdeltavg, cLightTriggerTreshold, 0, cDeltaScalerDef, cDeltaScalerMax );
    mDeltaScaler /= 1000.f;
  } 
  else 
  { 
    // delta_average > trigger_treshold, then decrease from 1. to 0.1
    mDeltaScaler = constrain( map( vdeltavg, cLightTriggerTreshold * 2, 
                              cLightTriggerTreshold * 5, cDeltaScalerDef, cDeltaScalerMin ), 
                              cDeltaScalerMin, cDeltaScalerDef );
    mDeltaScaler /= 1000.f;
  }

  if (++p_count >= 20) {
    p_count = 0;
    //Serial.print("Raw: "); Serial.print(acLightRaw);
    //Serial.print("\tAvg: "); Serial.println(vLightAvg);
    //Serial.print("Delta Avg: "); Serial.println(vdeltavg);
    //Serial.print("DeltaScaler: "); Serial.println(mDeltaScaler);
    //Serial.print("----Light Slow Avg: "); Serial.println(vLightSlowAvg);
  }


  /****************************************************************************************/
  // Pulses Loudness gain vs Triggers Avg:
  // pulses loudness gain (all 3 pulses, so a master gain) varies over time, 
  // depending on amount of triggers detected (count times that delta is above threshold) 
  // in the last period (try 30 secs), the mapping is a bell curve, 
  // where few triggers = lower gain , a moderate amount of triggers == higher gain, 
  // a lot of triggers = lower gain again
  long vtime = millis();
  if ( vtime - mTriggersTimestamp > cTriggersinterval ){
    mTriggersTimestamp = vtime;
    mTriggersAvg = mTriggersRolling.next(mTriggersCounter);
    mTriggersCounter = 0;
    mPulseGain = BellCurve( mTriggersAvg, (float)cTriggersAvgMin, (float)cTriggersAvgMax, 
                                cPulseMasterGainMin, cPulseMasterGainMax );
  }
    
  /****************************************************************************************
  Pulse resonance instead can be mapped to “excursion” (as mentioned before) - 
  the range for the resonance parameter that sounds good is 10-60 (as in Riccardo’s defs), 
  a small range of randomness here is nice (for instance a random range of +-5).  
  Excursion is defined as the difference between min and max light input value in the last period (try 60 secs). 
  Excursion is then mapped to the resonance value (15-55 +- random5) again following a bell shaped curve
  */

  // compute local min/max light value over last s
  mCurLightMin = acLightScaled < mCurLightMin ? acLightScaled : mCurLightMin;
  mCurLightMax = acLightScaled > mCurLightMax ? acLightScaled : mCurLightMax;
  int vcursec = mLightExcursionTimer / CONTROL_RATE;
  mLightMin[vcursec] = mCurLightMin;
  mLightMax[vcursec] = mCurLightMax;
  
  if ( mLightExcursionTimer % CONTROL_RATE == 0 ){
    
     // compute max excursion over last n seconds
    int vmin=INT_MAX;
    int vmax=0;
    for (int i = 0; i < cLightExcursionIntervalSecs; ++i){
        if ( mLightMax[i] > vmax ){
          vmax = mLightMax[i];
        }
        if ( mLightMin[i] < vmin ){
          vmin = mLightMin[i];
        }
    }

    float vExcursion = (float)( vmax - vmin ); 
    mPulseRes = BellCurve( vExcursion, cLightExcursionMin, cLightExcursionMax, 
                                      cPulseResonanceMin, cPulseResonanceMax );
    mCurLightMax=0;
    mCurLightMin=INT_MAX;

    //Serial.print("Sec: " ); Serial.print(vcursec);
    //Serial.print("\tCurMax: " ); Serial.print(mCurLightMax);
    //Serial.print("\tCurMin: " ); Serial.println(mCurLightMin);
    //Serial.print("\tExcursion: " ); Serial.println(vExcursion,1);
    //Serial.print("PulseRes: " ); Serial.println(mPulseRes,1);
  }
     
  mLightExcursionTimer = ++mLightExcursionTimer % cLightExcursionInterval;
  
  // update chords - must be performed on all
  mNumActiveChords=0;
  for (int s=0; s < kNumChords; ++s){

    mChords[s].Update( vLightSlowAvg, acLightRaw, light_delta );
    
    // limit chords polyphony - useless for now
    // bool vtriggered = mChords[s].Update( vLightSlowAvg, acLightRaw, light_delta );
    // limit on num of active chords 
    /*if ( vtriggered ){
      if (mNumActiveChords < cNumActiveChordsMax ){
         mChords[s].Unmute();
         Serial.print("Trigger Chord: "); Serial.println(s);
      }
      else{
         mChords[s].Reset();
         Serial.println("discarding chord "); 
      }
    }*/

    mChords[s].SetPulseMasterGain(mPulseGain);
    mChords[s].SetPulseResonanceAvg(mPulseRes);
    //Serial.print("Pulse Res: ");  Serial.println(mPulseRes); 

    if ( mChords[s].Active() ){
      mpActiveChords[mNumActiveChords++] = &mChords[s];
    }
  }

}





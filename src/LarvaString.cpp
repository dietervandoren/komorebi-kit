//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "LarvaString.hpp"


void LarvaString::Init(const int acID,  const float acFundFreq ){

    mID = acID;
    
    #ifdef PRINT
    //Serial.print("String: Init with id: "); Serial.print(mID); 
    //Serial.print("\tfund:  "); Serial.println(acFundFreq);
    #endif

    for (int i=0; i<cNumPartials; ++i){
        mSin[i] = Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> (SIN2048_DATA);
        mSmooth[i] = Smooth <unsigned int>(gcSmoothness);   
    }
}

void LarvaString::Update(){

  mTriggered = false;
    
    // update gains
    for (int i = 0; i < cNumPartials; i++) {

        if ( mDroneLevels[i] < cDroneStartThres ) {
            mGains[i] = 0;
        } 
        else {
            mGains[i] = map( mDroneLevels[i], cDroneStartThres, mDroneRange, 0, gain_max );
            long exp_scale = ( mGains[i] * mGains[i] * mGains[i]) >> 16;         

            // Weight partial gain vs freq
            mGains[i] = Freq2GainScaler( mFreq[i], (byte)exp_scale );
        }
    }

    // update smooth gains
    for (int i = 0; i < cNumPartials; ++i) {
        mSmoothGains[i] = mSmooth[i].next(mGains[i]);
    }

    // compute an overall gain sum
    long smooth_gains_sum = 0;
    for (int i = 0; i < cNumPartials; ++i) {
        smooth_gains_sum += mSmoothGains[i];
    }

    // drone at minimum? then mute string
    // else if we are muted and gain level goes above thres, retune the string and unmute
    if ( smooth_gains_sum <= gain_min ) { 
        Mute();
    }
    else {
        Unmute();
    }

    // section performed only if playing
    if (mActive){
        
        // Decrease Drone Levels
        unsigned long time_now = millis();
        if ( time_now - mDecreaseTime > cDroneDecreaseRate ) {
            for (int i = 0; i < cNumPartials; i++) {
            mDroneLevels[i] = max( mDroneLevels[i] - mDroneDecreaseStep[i], 0 ); 
            }
            mDecreaseTime = time_now;
        }

        // Pulses
        for (int i = 0; i < cNumPartials; i++) 
        {
            if ( mPulseL_levels[i] >= mPulseL_treshold ) { 
                mPulseL_levels[i] = 0;
                TriggerRandomPulse( i, mFreq[i], cPulseGainL );
            }
            else if ( mPulseM_levels[i] >= mPulseM_treshold ) { 
                mPulseM_levels[i] = 0;
                TriggerRandomPulse( i, mFreq[i], cPulseGainM );
            }
            else if ( mPulseS_levels[i] >= mPulseS_treshold ) {
                mPulseS_levels[i] = 0;
                TriggerRandomPulse( i, mFreq[i], cPulseGainS );
            }
        }

        mPlokSynth.Update();

        // modulate partial freqs with LFO
        if ( ++mLFOUpdateTimer >= cLFOUpdateInterval ){
            float vlfo = mLFO.Get();
            for (int i = 0; i < cNumPartials; ++i) {
                    mFreq[i] = mBaseFreq[i] + mDetune[i] * vlfo;
                    mSin[i].setFreq( mFreq[i] );
            }
            mLFOUpdateTimer=0;
        }

    } // if active
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// called @kr on all strings 
void LarvaString::UpdateLevels( const int acLightInput, const int acLightDelta ){

    // light change trigger
    if ( acLightDelta >= cLightTriggerTreshold ) 
    {
        // bump up levels
        for (int i = 0; i < cNumPartials; i++) { 
        
            // within harmonic range?
            if ( acLightInput >= mLightRangeMin[i] && acLightInput < mLightRangeMax[i] ) { 

                // increase and clip levels
                mDroneLevels[i] = min( mDroneLevels[i] + acLightDelta, mDroneRange ); 
                mPulseL_levels[i] += acLightDelta;
                mPulseM_levels[i] += acLightDelta;
                mPulseS_levels[i] += acLightDelta;

                //Serial.print("String "); Serial.print(mID);
                //Serial.print(" drone level: ");
                //Serial.println(mDroneLevels[i]);
            }
        }
    }
}

void LarvaString::Retune(const float acFundamental) {
  
  // the cutoff frequency determines from which partial above the fundamental 
  // we start on one string (1-3) partial 1 is the fundamental etc
  // when the fundamental of the string is set, this value is picked at random, 
  // with a higher waiting of the lower values compared to the higher values
    mCutoffPartial = rand(MaxTuningOffset) + 1;

    #ifdef PRINT
        //Serial.print("-- String "); Serial.print(mID);
        //Serial.print(" sets cutoff at: ");  Serial.print(mCutoffPartial);
        //Serial.print(" fund: ");  Serial.println(acFundamental);
        #endif

    mFundFreq = acFundamental;

    for (int i = 0; i < cNumPartials; i++) {

        mBaseFreq[i] = mFundFreq * ( mCutoffPartial + i );
        
        // add random detune
        float vdet = cDetuneFactor * mBaseFreq[i];
        mDetune[i] = ( random(2000)*0.001f - 1.f ) * vdet;
        mFreq[i] = mBaseFreq[i] + mDetune[i];
        mSin[i].setFreq( mFreq[i] ); 

        // set decrease step according to master value & freq curve
        float vscaler = FreqScaler(mFreq[i],cDecrCutoff,cDecrFreqMax,cDecrHF,cDecrSlope);
        vscaler = vscaler > 1e-5f ? vscaler : 1e-5f;
        mDroneDecreaseStep[i] = (int)( mDroneDecreaseStepMaster / vscaler );
        
        #ifdef PRINT
        //Serial.print("\tFreq: ");  Serial.print(mFreq[i]);
        //Serial.print("\tdecrstep: ");  Serial.println(mDroneDecreaseStep[i]);
        //Serial.print("-- String "); Serial.print(mID);
        //Serial.print(" tune partial: ");  Serial.print(i);
        //Serial.print(" at freq: ");  Serial.println(mFreq[i]);
        #endif
  }
}

/*
the cutoff is also reset when the following condition occurs:  
check how many events were triggered (how many times the light input delta 
was above the threshold) in the period since the cutoff was last set, 
if this value is above a certain threshold (which means the string has been played a lot), 
then choose another random (weighted) value
*/
void LarvaString::UpdateCutoffLevel(){
    
    mCutoffPartial = rand(3) + 1;

    //Serial.print("-- String "); Serial.print(mID);
    //Serial.print(" UpdateCutoff: ");  Serial.println(mCutoffPartial);
    
    for (int i = 0; i < cNumPartials; i++) {

        mBaseFreq[i] = mFundFreq * ( mCutoffPartial + i );
        
        // add random detune
        //float vdet = cDetuneFactor * mBaseFreq[i];
        //mDetune[i] = map( random(1001), 0.f, 1000.f, -vdet, vdet );
        mFreq[i] = mBaseFreq[i] + mDetune[i];
        mSin[i].setFreq( mFreq[i] ); 
  }
}

void LarvaString::TriggerRandomPulse( const int acVoice, 
                                      const float acFreq, 
                                      const float acGain )
{
    // dither plok params
    float vrand = ( (float)rand(2000) * 0.001f - 1.f ) * cPulseResonanceRandRange;
    float q = mPulseResonanceAvg + vrand;
    int d = cPlokImpulseDurMin + rand(cPlokImpulseDurRange);
    float vg = acGain * mPulseMasterGain;
    
    mPlokSynth.Trigger( acFreq, q, vg, d );
    
    //Serial.print("Trig Plok on "); Serial.print(mID); Serial.println(acFreq);
    //Serial.print(" voice "); Serial.println(acVoice);
    //Serial.print(" freq "); Serial.println(acFreq);
    //Serial.print("Qrand: "); Serial.println(vrand);
}
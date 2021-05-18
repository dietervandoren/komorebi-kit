//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LarvaSynth
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma once

#include <MozziGuts.h>
#include <RollingAverage.h>
#include <Smooth.h>
#include <mozzi_rand.h>
#include <ADSR.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h>
#include "Plok.hpp"
#include "LarvaDefs.hpp"        

extern float mFreq2GainTable[cFreq2GainTableSize];

// simple triangular lfo
class LFO{

public:
    
    LFO(){}
    ~LFO(){}
    
    inline void SetRate(const float acValue){ 
        rate = acValue; 
        hperiod = (int)(cLFOsr/rate) / 2;
        hperiod = hperiod > 1 ? hperiod : 1;
        timer=0;
    }

    inline float Get(){
        float vval = (float)timer/(float)hperiod;        
        if (timer==0) step=1;
        else if (timer>=hperiod) step=-1;
        timer += step;

        return vval;
    }

private:
    float rate{0.1f};
    long int timer{0};
    int step{1};
    long int hperiod{1};
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class LarvaString
{
public:
    LarvaString(){};
    ~LarvaString(){};
    void Init( const int acID, const float acFundFreq );
    
    // callme @sr
    inline float Process(){

        float vsum = 0.f;
        for ( int i=0; i < cNumPartials; ++i ){
            float vosc = (float)( mSin[i].next() * mSmoothGains[i] );
            vsum += vosc;
        }

        vsum *= cStringDroneGainScaler;
        vsum += mPlokSynth.Process();
        
        return vsum;
    }

    // callme @kr
    void Update();

    // callme only if you have to update accumulators
    void UpdateLevels( const int acLightInput, const int acLightDelta );
        
    inline void Mute(){ 
        if (mActive==true){
            //Serial.print("-- Muting String "); Serial.println(mID);
            mActive=false; 
        }
    }
    
    inline void Unmute(){ 
        if (mActive==false){
            //Serial.print("-- Unmuting String "); Serial.println(mID);
            Retune(mFundFreq); 
            mActive=true; 
            mTriggered=true;
        } 
    }
    
    inline bool Active(){ return mActive; }
    inline bool Triggered(){ return mTriggered; }
    
    void Retune(const float acFundamental);

    void SetLightRange(const int acMin, const int acMax ){ 

        // compute light ranges for each partial
        int vrange = acMax - acMin;
        int vrange12 = vrange / cNumPartials; 
        for (int i = 0; i < cNumPartials; i++) {         
            mLightRangeMin[i] = acMin + ( i * vrange12 );
            mLightRangeMax[i] = acMin + ( (i+1) * vrange12 );  
        }
    }

    inline void SetID(const int acValue){ mID = acValue; }
    inline int ID(){ return mID; }

    inline void SetPulseLThres(const int acValue){ mPulseL_treshold = acValue; }
    inline void SetPulseMThres(const int acValue){ mPulseM_treshold = acValue; }
    inline void SetPulseSThres(const int acValue){ mPulseS_treshold = acValue; }
    inline void SetDroneRange(const int acValue){ mDroneRange = acValue; }
    
    inline void SetDroneDecreaseStep(const int acValue){ 
        mDroneDecreaseStepMaster = acValue; 
    }

    inline void SetPulseMasterGain( const float acValue ){
        mPulseMasterGain = acValue;
    }

    inline void SetPulseResonanceAvg( const float acValue ){
        mPulseResonanceAvg = acValue;
    }

    inline void SetLFORate(const float acValue){ mLFO.SetRate(acValue); }

private:    
    void UpdateCutoffLevel();
    void TriggerRandomPulse( const int acVoice, const float acFreq, const float acGain );
    
    // :TODO: tabulate
    float FreqScaler( const float acFreq, const float acCutoff, 
                    const float acFreqMax, const float acGainMin, 
                    const float acSlope ){

        if ( acFreq <= acCutoff ){
            return 1.f;
        }
        else{   
            float vg = 1.f - fastPow( ( acFreq - acCutoff ) / ( acFreqMax - acCutoff ), acSlope ) * (1.f - acGainMin );
            vg = vg < acGainMin ? acGainMin : vg;
            //Serial.print("FreqScaler: f: ");Serial.print(acFreq);
            //Serial.print("\tg: ");Serial.println(vg);
            return vg;
        }
    }

private:
    int mID{0};
    bool mActive{false};
    bool mTriggered{false};
    bool retune_ready;
    float mFundFreq{333.f};
    float mBaseFreq[cNumPartials];
    float mDetune[cNumPartials];
    float mFreq[cNumPartials];

    static const int mNumPartials{cNumPartials};
    Oscil<SIN2048_NUM_CELLS, AUDIO_RATE> mSin[cNumPartials];
    Smooth <unsigned int> mSmooth[cNumPartials];
        
    byte mGains[cNumPartials]; // 0-255 8bit for speed
    byte mSmoothGains[cNumPartials];
    
    int mNumTriggeredEvents{0};
    static const int cNumTriggeredEventsThresh = 1000;

    // the cutoff frequency determines from which partial above the fundamental 
    // we start on one string (1-4) partial 1 is the fundamental etc 
    int mCutoffPartial{1};

    PlokSynth mPlokSynth;
    
    // master gain controlled by triggers activity 
    float mPulseMasterGain{1.f};

    // Avg pulse resonance
    // Fix 29/4 - chance of negative q setting on startup
    float mPulseResonanceAvg{cPulseResonanceMin};

    int mPulseL_levels[cNumPartials];
    int mPulseM_levels[cNumPartials];
    int mPulseS_levels[cNumPartials];

    int mPulseL_treshold{0};
    int mPulseM_treshold{0};
    int mPulseS_treshold{0};

    int mDroneLevels[cNumPartials]; // 0-1000
    int mDroneRange{0};
    int mDroneDecreaseStepMaster{66};
    int mDroneDecreaseStep[cNumPartials];
    unsigned long mDecreaseTime;
    
    // light ranges for each partial
    int mLightRangeMin[cNumPartials];
    int mLightRangeMax[cNumPartials];

    LFO mLFO;
    
    int mLFOUpdateTimer{0};

    // scale by 1 / ( npartials * 32640 ) * master
    static constexpr float cStringDroneGainScaler = 1.f / ( cNumPartials * 32640.f ) * cDroneMasterGain;

    byte Freq2GainScaler(const float acFreq, byte acGain){

        float vdf = acFreq - cFreqCutoff;
        
        if (vdf <= 0.f ){
            return acGain;
        }

        // no interp for optim
        int vn = floorf( vdf / cFreq2GainTableStep );
        byte vgain = (byte)( acGain * mFreq2GainTable[vn] );
        return vgain;
    }
};

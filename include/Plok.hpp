//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Plok dsp module
// plok sound generator
// Simple Source-Filter Plok sound synth
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma once

#include "LarvaDefs.hpp"
#include <MozziGuts.h>
#include <math.h>

class Plok{

public:
    
    Plok(){
        UpdateBiquad();
        mLifeEstimation = (unsigned long)(AUDIO_RATE * cPlokDurEstimMs * 0.001f); 
        mOnCounter = mLifeEstimation;
    }

    void Trigger( const float acFreq, const float acQ, 
                    const float acGain, const int acImpulseDur )
    {
        mEnvDur = acImpulseDur > 0 ? acImpulseDur : 1;
        mGain = acGain > 0.f ? acGain : 0.f;
        mFc = acFreq > 20.f ? acFreq : 20.f;
        mQ = acQ > 1.f ? acQ : 1.f;
        UpdateBiquad();
        mCounter=0;
        mOnCounter=0;
    }

    ~Plok(){}
    
    inline float Process(){

        // update life counter
        if ( mOnCounter < mLifeEstimation ){
            mOnCounter++;
        }
        
        // white noise source
        float vsig = WNoise() * 1.5f;

        // envelope (simple rect impulse)
        float venv = ++mCounter < mEnvDur ? 1.f : 0.f;
        vsig *= venv;

        // res biquad
        float y0 = vsig * b0 + x2 * b2 + y1 * a1 + y2 * a2;
        x2 = x1;
        x1 = vsig;
        y2 = y1;
        y1 = y0;

        return y0;
    }

    inline void SetImpulseDur(const int acValue){
        mEnvDur = acValue;
    }

    inline void SetFrequency(const float acValue){
        mFc = acValue;
        UpdateBiquad();
    }

    inline void SetQ(const float acValue){ 
        mQ = acValue;
        UpdateBiquad();
    }

    inline void SetGain(const float acValue){
        mGain = acValue;
        b0 = R * mGain * ( 1.f -  R );
        b2 = -b0;
    }
    
    // one for all
    void Set( const float acFreq, const float acQ, const float acGain, const int acImpulseDur )
    {
        mEnvDur = acImpulseDur;
        mFc = acFreq;
        mQ = acQ;
        mGain = acGain;
        UpdateBiquad();
    }

    inline bool On(){ return ( mOnCounter < mLifeEstimation ); }

private:

    void UpdateBiquad()
    {     
        float bw = mFc / mQ;
        R =  expf( -cOmegaFactor * bw  );
        
        // note signs - these are actually -a1, -a2
        a2 = -R * R;
        a1 = 2.f * R * cosf( cOmegaFactor * mFc );
        b0 = R * mGain * ( 1.f -  R );
        b2 = -b0;
    }
    
    static constexpr __uint32_t rrandmax = 0x3fffffffu;
    uint32_t rrand = 1u;
    inline float WNoise(){
        return (float)( (rrand = rrand * 1103515245u + 12345u) & rrandmax ) / 536870912.f - 1.f;
    }

private:

    static constexpr float cOmegaFactor = 2.f * PI / AUDIO_RATE;
    
    // life dur estim for 1 plok event
    unsigned long mLifeEstimation{0};
    unsigned long mOnCounter{0};

    // Biquad Resonant Filter
    float mFc{1000.f};
    float mQ{1.f};
    float mGain{1.f};
    float R{0.f};
    float b0{0.f};
    float b2{0.f};
    float a1{0.f};
    float a2{0.f};
    
    float y2{0.f};
    float y1{0.f};
    float x1{0.f};
    float x2{0.f};

    // Envelope Gen
    int mCounter{0};
    int mEnvDur{25};
};

//---------------------------------------------------------
class PlokSynth
{

public:
    PlokSynth(){}
    ~PlokSynth(){}

    void TriggerVoice(  const int acVoice, 
                        const float acFreq, 
                        const float acQ,
                        const float acGain,
                        const int acImpulseDur );

    // voice stealing
    void Trigger(  const float acFreq, 
                   const float acQ,
                   const float acGain,
                   const int acImpulseDur );

    // control rate callback
    void Update();

    // sample rate callback
    inline float Process(){
        float vmix = 0.f;
        for ( int i = 0; i < mNActiveVoices; ++i ){
            vmix += mpActiveVoices[i]->Process();    
        }
        return vmix;
    }

    inline bool Available(){ return (mNActiveVoices<cNVoices); }

private:
    static const int cNVoices = 12;
    int mNActiveVoices{0};
    Plok mVoices[cNVoices];
    Plok* mpActiveVoices[cNVoices];
    int mCurVoice{0};
};
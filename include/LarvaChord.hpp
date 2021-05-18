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

#include "LarvaDefs.hpp"
#include "LarvaString.hpp"

class LarvaChord
{
public:
    
    LarvaChord(){};
    ~LarvaChord(){};

    void Init( const ChordID acChord );
    
    // callme @sr
    float Process() 
    {
        float vMix = 0.f;
        for ( int s = 0; s < mNumActiveStrings; ++s ){
            vMix += mpActiveStrings[s]->Process();
        }

        // scale by number of strings
        vMix *= 0.33f;

        return vMix; 
    }

    // callme @kr
    void Update( const int acLightAvg, const int acLight, const int acLightDelta );
    
    inline void Mute(){ 
        if (mActive==true){
            //Serial.print("- Muting Chord "); Serial.println(mID);
            mActive=false; 
        }
    }
    
    inline void Unmute(){ 
        if (mActive==false){
            
            // each time the audio output is zero (completely silent), the 3 strings are retuned.
            Retune();

            mActive=true; 
            mTriggered=true;
            //Serial.print("- Unmuting Chord "); Serial.println(mID);
        } 
    }

    inline bool Active(){ return mActive; }
    inline bool Triggered(){ return mTriggered; }
    
    void Retune();

    void SetLightRange(const int acMin, const int acMax);
    
    inline void SetPulseMasterGain(const float acValue){
        for (int s = 0; s < mNumActiveStrings; ++s){
            mpActiveStrings[s]->SetPulseMasterGain(acValue);
        }
    }

    inline void SetPulseResonanceAvg(const float acValue){
        for (int s = 0; s < mNumActiveStrings; ++s){
            mpActiveStrings[s]->SetPulseResonanceAvg(acValue);
        }
    }
    inline ChordID ID(){ return mID; }

private:
    bool mActive{false};
    bool mTriggered{false};

    int mLightRangeMin{0};
    int mLightRangeMax{0};

    ChordID mID{kChord1};
    
    int mNumActiveStrings{0};
    LarvaString* mpActiveStrings[cNumStrings];
    LarvaString mString[cNumStrings];
};



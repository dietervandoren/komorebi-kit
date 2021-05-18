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
#include "LarvaChord.hpp"

class LarvaSynth2
{

public:
    LarvaSynth2(){}
    ~LarvaSynth2(){}

    // callme @setup
    void Init();
    
    // callme @sr
    int16_t Process();

    // callme @kr
    void Update( const int acLightRaw, const int acLightScaled );

    inline void Start(){ startMozzi(CONTROL_RATE); }
    inline void Stop(){ stopMozzi(); }

private:

    inline float BellCurve( const float acIn, const float acInMin, const float acInMax,
                    const float acOutMin, const float acOutMax )
    {
        float vin = acIn < acInMin ? acInMin : vin;
        vin = vin > acInMax ? acInMax : vin;
        float vinorm = (vin - acInMin) / ( acInMax - acInMin );
        float vout = acOutMin + ( 1.f - cosf( 2.f*PI*vinorm) ) * 0.5f * ( acOutMax - acOutMin );
        vout = vout > acOutMax ? acOutMax : vout;
        vout = vout < acOutMin ? acOutMin : vout;

        return vout;
    }

private:

    bool mTriggered{false};

    // Chords
    LarvaChord mChords[kNumChords];
    LarvaChord* mpActiveChords[kNumChords];
    int mNumActiveChords{0};

    // Light input process
    float mDeltaScaler{1.f};
    RollingAverage<int, cChordChangeLightAvgSize> mLightSlowRolling; 
    RollingAverage<int, cLightRollingSize> mLightRolling;
    RollingAverage<int, cDeltaRollingSize> mDeltaRolling; 

    // Triggers avg over last n seconds
    int mTriggersAvg{0};
    int mTriggersCounter{0};
    int mTriggersTimestamp{0};
    RollingAverage<int, cTriggersRollingSize> mTriggersRolling; 

    // master controls for Pulse gain and resonance
    float mPulseGain{1.f};
    float mPulseRes{15.f};

    // Light Excursion - min/max history over n seconds
    static const int cLightExcursionInterval = cLightExcursionIntervalSecs * CONTROL_RATE;
    int mCurLightMin{INT_MAX};
    int mCurLightMax{0};
    int mLightMin[cLightExcursionIntervalSecs];
    int mLightMax[cLightExcursionIntervalSecs];
    int mLightExcursionTimer{0};
    int mLightExcursion{0};

    // serial printing counter
    int p_count; 
};





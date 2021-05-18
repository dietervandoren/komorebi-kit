//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main parameter definitions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma once

#include "MozziGuts.h"

//#define PRINT

enum ChordID { 
    kChord1=0,
    kChord2,
    kChord3,
    kChord4,
    kChord5,

    kNumChords
};


// Chords defs

// fundameltals (circle of 5ths)
const float Fund1 = 110.;
const float Fund2 = Fund1/2*3;
const float Fund3 = (Fund2/2*3)/2;
const float Fund4 = Fund3/2*3;
const float Fund5 = (Fund4/2*3)/2;

static const float cChord1[5] = {
Fund1,
Fund1/4*5, //(5:4)
Fund1/2*3, //(3:2)
Fund1/3*5, // (5:3)
Fund1*2
};

static const float cChord2[5] = {
Fund2,
Fund2/4*5, 
Fund2/2*3, 
Fund2/3*5,
Fund2*2
};

static const float cChord3[5] = {
Fund3,
Fund3/4*5, 
Fund3/2*3, 
Fund3/3*5,
Fund3*2
};

static const float cChord4[5] = {
Fund4,
Fund4/4*5, 
Fund4/2*3, 
Fund4/3*5,
Fund4*2
};

static const float cChord5[5] = {
Fund5,
Fund5/4*5, 
Fund5/2*3, 
Fund5/3*5,
Fund5*2
};

static const float* cChords[kNumChords] = {
    cChord1,
    cChord2,
    cChord3,
    cChord4,
    cChord5,
};

// num strings for each Chord
static const int cNumStrings = 3;

// num partials for each string
static const int cNumPartials = 12;

// max random offset from which to start sounding partials above the fundamental
static const int MaxTuningOffset = 3; // tested 0-3

// when tuning the 3 strings we randomly pick between one of these voicing options:

static const int cNumChordVoicings = 5;
static const int cChordVoicings[cNumChordVoicings][cNumStrings] = {
    { 0, 1, 2 },
    { 0, 1, 3 },
    { 0, 2, 3 },
    { 0, 2, 4 },  
    { 1, 3, 4 },
};


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// DRONES SETTINGS

static const int TMul = 15;
static constexpr float cDroneMasterGain = 2.5f;

// mod gain vs freq
const float cFreqCutoff = 300.f; // gain of freqs above this cutoff is scaled
const float cFreqMax = 6000.f;  // freq at which gain will be min
const float cGainHF = 0.001f;    // min gain

// slope of the gain curve (<1 valle, >1 monte) 
// range (0.25--3.0)
const float cGainSlope = 0.8f;  


// max playing time duration ms before resetting cutoff partial
static const unsigned long cDroneMaxDurMs = 4000;

// detune factor, normalized
static const float cDetuneFactor = 0.01f;

// LFO on detune
static const float cLFOrate[cNumStrings] = { 0.10f, 0.045f, 0.06f }; // 0.5f, 0.17f, 0.23f 
static const int cLFOsr = 6; // lfo samplerate
static const int cLFOUpdateInterval = CONTROL_RATE / cLFOsr;

// drone gain smoothing factor
static const float gcSmoothness = 0.975f; 

static const int cDroneStartThres = 10*TMul;
static const int cDroneRange[cNumStrings] = { 1100*TMul, 1100*TMul, 1100*TMul};

static const int cDroneDecreaseStep[cNumStrings] = { 5*TMul, 6*TMul, 7*TMul };
static const int cDroneDecreaseRate = 20; // ms

// Decrease step mult factor vs freq curve
const float cDecrCutoff = 800.f;    // decrease step of freqs above this cutoff is scaled
const float cDecrFreqMax = 6000.f;  // freq at which gain will be min
const float cDecrHF = 0.1f;         // decrease step factor @ max freq 1= no effect, < 1 damps higher freqs
const float cDecrSlope = 0.8f;      // slope (<1 valle, >1 monte)  


// partial gain
static const int gain_max = 255;
static const int gain_min = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// PLOKS SETTINGS

static const int cPlokImpulseDurMin = 10;
static const int cPlokImpulseDurRange = 20;

static const float cPlokDurEstimMs = 50.f;

static constexpr float cPulseGainL = 14.f;
static constexpr float cPulseGainM = 6.f;
static constexpr float cPulseGainS = 2.f;

/*
Pulse resonance instead can be mapped to “excursion” (as mentioned before) - 
the range for the resonance parameter that sounds good is 10-60 (as in Riccardo’s defs), 
a small range of randomness here is nice (for instance a random range of +-5).  
Excursion is defined as the difference between min and max light input value in the last period (try 60 secs). 
Excursion is then mapped to the resonance value (15-55 +- random5) again following a bell shaped curve 
(same as described above with pulse loudness relative to trigger count, 
also here we need to tune the bell curve to a good range while testing). 
*/
static const int cLightExcursionIntervalSecs = 60;  // in seconds
static const float cLightExcursionMin = 0.f;        // on absolute light range (scaled)
static const float cLightExcursionMax = 300.f;     // on absolute light range (scaled)
static const float cPulseResonanceMin = 20.f;
static const float cPulseResonanceMax = 50.f;
static const float cPulseResonanceRandRange = 5.f;


// Pulse triggering Thresholds -------------------- A      B    C

static const int cPulseL_treshold[cNumStrings] = { 2000*TMul, 3000*TMul, 4000*TMul }; // pulse L
static const int cPulseM_treshold[cNumStrings] = { 1000*TMul, 1500*TMul, 2000*TMul }; // pulse M
static const int cPulseS_treshold[cNumStrings] = { 500*TMul, 800*TMul, 1000*TMul }; // pulse S

// Pulses loudness vs activity bell

// time interval for computing triggers average (ms)
static const long cTriggersinterval = 1000;
static const int cTriggersRollingSize = 30;

// "low activity" thres (ntriggers/interval)
static const int cTriggersAvgMin = 1;

// "high activity" thres (ntriggers/interval)
static const int cTriggersAvgMax = CONTROL_RATE;

// pulses master gain @ low/high activity
static const float cPulseMasterGainMin = 0.2f;

// pulses master gain @ mid
static const float cPulseMasterGainMax = 1.3f;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LIGHT SENSOR

// valid reading range 
// 0 and 4095 flag saturation
// A saturation routine flags when out of range 
static const int cLuxRawThresMin = 1;
static const int cLuxRawThresMax = 4094;

// slow light avg for changing chords
static const int cChordChangeLightAvgSize = 1000; // 1000/64 = 15s

// fast light avg for detecting triggers
static const int cLightRollingSize = 32; // 32 / 64 Hz = 0.5s

// delta light avg size for adjusting deltaScaler
static const int cDeltaRollingSize = 2048 ; // 2048 / 64 Hz = 32s

// delta scaler settings
static const int cDeltaScalerMin = 500;
static const int cDeltaScalerDef = 1000;
static const int cDeltaScalerMax = 20000;

// sets sensitivity to light changes
static const int cLightTriggerTreshold = 80; // 25

// photosens out range
static const float cLightRange = 1050.f; 

// Photo Sensor Pin
static const int cPhotoSensPin = 34; 

// Light sensor gain calibration

// mov avg size on raw input for calibrating the gain
static const int cGainCalibAvgSize = 128; // @64 = 2s

// desired range for avg light input, that is, 
// an hysteresis centered around mid value (2000)
// if the input value is out of range, gain calibration occurs
static const int mLuxRawAvgThresMin = 1000;
static const int mLuxRawAvgThresMax = 3000;

// Amp enable pin
static const int cAmpEnablePin = 17;

// gain range
static const int cGainMin = 0;
static const int cGainMax = 255;

// gain update step
static const int cGainStep = 1;

// gain update period (ms)
static const unsigned long cGainModPeriodMs = 100;

static const int cFreq2GainTableSize = 128;
static const float cFreq2GainTableRange = cFreqMax - cFreqCutoff + 1.f;
static const float cFreq2GainTableStep = cFreq2GainTableRange / cFreq2GainTableSize;

// Optimized pow fuction by Martin Ankerl
// https://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
static double fastPow(double a, double b) {
    union {
        double d;
        int x[2];
    } u = { a };
    u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}



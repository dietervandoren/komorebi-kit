//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// PlokSynth
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "../include/Plok.hpp"

// Triggers a specific voice
void PlokSynth::TriggerVoice( const int acVoice, const float acFreq, 
                                const float acQ, const float acGain, const int acImpulseDur ){
    
    if ( acVoice >= cNVoices ){
        //Serial.print("WARNING: PlokSynth: SetVoice: invalid voice id:  ");
        //Serial.print(acVoice);
        return;
    }

    // no stealing! ignore trigger if already playing    
    if ( mVoices[acVoice].On()==true ){
       // Serial.println("WARNING: PlokSynth: SetVoice: already on, return");
        return;
    } 
    
    mVoices[acVoice].Trigger( acFreq, acQ, acGain, acImpulseDur );   
}

// Trigger - no stealing
void PlokSynth::Trigger( const float acFreq, 
                         const float acQ, 
                        const float acGain, 
                        const int acImpulseDur ){

    if ( mNActiveVoices < cNVoices ){
        mVoices[mNActiveVoices++].Trigger( acFreq, acQ, acGain, acImpulseDur );   
    }
}

// @controlRate
void PlokSynth::Update(){

    // flush ptrs to dead voices
    mNActiveVoices=0;
    for ( int i = 0; i < cNVoices; ++i ){
        if ( mVoices[i].On() ){
            mpActiveVoices[mNActiveVoices++] = &mVoices[i];
        }
    }
}



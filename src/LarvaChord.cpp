//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "LarvaChord.hpp"


void LarvaChord::Init(const ChordID acChord ){

    mID = acChord;
    //Serial.print("- Chord: Init with id: "); Serial.println(mID);
  
    for (int i = 0; i < cNumStrings; i++){

        int vid = ((int)(mID)+1)*100 + (i+1);
        mString[i].Init( vid, cChords[mID][i] );
        mString[i].SetPulseLThres( cPulseL_treshold[i] );
        mString[i].SetPulseMThres( cPulseM_treshold[i] );
        mString[i].SetPulseSThres( cPulseS_treshold[i] );
        mString[i].SetDroneRange( cDroneRange[i] );
        mString[i].SetDroneDecreaseStep( cDroneDecreaseStep[i] );
        mString[i].SetLFORate(cLFOrate[i]);
    }

    Retune();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*int32_t LarvaChord::ProcessFix32() 
{
    int32_t vMix = 0;

    for ( int s = 0; s < mpActiveStrings.size(); ++s ){
        vMix += mpActiveStrings[s]->ProcessFix16();
    }

    return vMix; 
}*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void LarvaChord::Update( const int acLightAvg, const int acLightInput, const int acLightDelta ){
  
    mTriggered=false;

    // if slow avg is within the range of this chord
    if ( acLightAvg >= mLightRangeMin && acLightAvg < mLightRangeMax ){
        for (int s=0; s < cNumStrings; ++s){
            mString[s].UpdateLevels( acLightAvg, acLightDelta );
            
        }
    }

    // Note: kr update must be performed on ALL strings (both active/inactive)
    mNumActiveStrings=0;
    for (int s=0; s < cNumStrings; ++s){
        
        mString[s].Update();

         if ( mString[s].Active() ){
            mpActiveStrings[mNumActiveStrings++] = &mString[s];
            Unmute(); // chord is unmuted when at least one of its strings is unmuted

            /*if ( mString[s].Triggered() ){
                Serial.print("Chord: "); Serial.print(mID); 
                Serial.print("\tnstrings: "); Serial.println(mNumActiveStrings);
            }*/
        }
    }

    if (mNumActiveStrings==0){
        Mute();
    }
    

}

/* Retune
the 3 strings are tuned to one of the 5 chords, depending on the average light input 
over a long period the whole range of the light input (0-1050) is divided in 5 equal parts 
within those ranges of 250 values the 3 strings function just as in the old Larva code
in the range 0-250 we tune to chord 1, in the range 251-500 we tune to chord 2 etc
each chord has 4 possible pitches, but we only ever use 3 at a time
*/
void LarvaChord::Retune(){

    int voicing = random(cNumChordVoicings);

    #ifdef PRINT
    //Serial.print("- Chord: ");  Serial.print(mID);
    //Serial.print(" Retune to voicing: ");  Serial.println(voicing);
    #endif

     for (int i = 0; i < cNumStrings; i++)
     {
        int voice = cChordVoicings[voicing][i];
        float vfund = cChords[mID][voice];
        mString[i].Retune(vfund);
    }
}

void LarvaChord::SetLightRange(const int acMin, const int acMax){ 
    
    mLightRangeMin=acMin; 
    mLightRangeMax=acMax;

    for (int s=0; s < cNumStrings; ++s){
        mString[s].SetLightRange(acMin,acMax);
    }
}

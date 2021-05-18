//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// photosensor reader module
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma once

#include "MCP4151Controller.hpp"
#include "LarvaDefs.hpp"
#include <RollingAverage.h>

class PhotoSensReader
{

public:
    PhotoSensReader(){}
    ~PhotoSensReader(){}

    // callme @ setup
    void Init(){
        pinMode( cPhotoSensPin, INPUT );
        mDigiPot.Init();
        mDigiPot.SetGain( mGain );
    }

    // callback @ controlrate
    void Update()
    {
        // Get analog value
        mLuxRaw = analogRead( cPhotoSensPin );
        
        // note: avg keeps also out of range vals in order to check if calibration needed
        mLuxRawAvg = LuxMovAvg(mLuxRaw);

        // Calibrate the gain in order to keep the moving average in range
        if ( mLuxRawAvg < mLuxRawAvgThresMin || mLuxRawAvg > mLuxRawAvgThresMax ){
            Calibrate();
        }
        else if (mCalibrating==true){
            mCalibrating=false;
            //Serial.print("gain: "); Serial.println( mGain );
        }
        
        // extreme values are saturation flags
        if ( mLuxRaw < cLuxRawThresMin || mLuxRaw > cLuxRawThresMax ){
            if ( mSatCounter >= mSatCounterMax ){
                mSaturated=true;
                //Serial.print("SAT"); 
            }
            else{
                mSatCounter++;
            }
        }
        else{
            // resets saturation timer
            mSatCounter=0;
            mSaturated=false;
        }
        
        // correct raw lux reading w/ respect to gain
        // from [0,4095] to [ 0.0039, 4095 ]
        float vlux = (float)mLuxRaw / (float)(mGain + 1.f);
    
        // scaling & normalize according to Matteo Max Patch
        // DTR recalculated to: 
        // [0.0039, 4095] --> db [] (0.0039 = 1. / 256.)
        // [-48.18dB,+72.25dB] --> [0,1050]
        float vluxdb = 20.f * log10( vlux );
        float vdbmin = -48.18f;
        float vdbmax = 72.25f;
        vluxdb = limitf( vluxdb, vdbmin, vdbmax );
        float vnorm = ( vluxdb - vdbmin ) / ( vdbmax - vdbmin );
        mLuxScaled = (int)roundf( vnorm * cLightRange );

        if (++mPrintCounter >= 20 ){
            mPrintCounter=0;
         //   Serial.print("Lux Raw: "); Serial.print(mLuxRaw);
           // Serial.print("\tLux Raw Avg:"); Serial.print(mLuxRawAvg);
           // Serial.print("\tGain:"); Serial.print( mGain );
            //Serial.print("\tLux Scaled:"); Serial.print(mLuxScaled);
            //Serial.println(" ");
        }
    }

    // Gain Calibration
    void Calibrate()
    {
        if ( mCalibrating==false ){
            mCalibrating = true;
        }

        unsigned long now = millis();
        if( now - mTimestamp > cGainModPeriodMs )
        {
            mTimestamp = now;
            int vdelta = mLuxRawAvg < mLuxRawAvgThresMin ? cGainStep : -cGainStep;
            mGain += vdelta;
            
            if ( mGain >= cGainMax  || mGain <= cGainMin )
            {
                mGain = mGain > cGainMax ? cGainMax : mGain;
                mGain = mGain < cGainMin ? cGainMin : mGain;
                
                // reached limits, that's the best we can do, thus exit calibration
                mCalibrating = false;
            }

            mDigiPot.SetGain( mGain );   
        }
    }

    inline float GetLuxScaled(){  return mLuxScaled; }
    inline float GetLuxRaw(){  return mLuxRaw; }
    inline bool Saturated(){ return mSaturated; }

private:

    inline float limitf( const float aIn, const float aMin, const float aMax ){
        float vOut = aIn < aMin ? aMin : aIn > aMax? aMax : aIn;
        return vOut;
    }

    int LuxMovAvg(const int acval ){
        mAvgBuf[mAvgInd] = acval;
        mAvgInd = ++mAvgInd % cGainCalibAvgSize;
        int val=0;
        for (int i=0; i<cGainCalibAvgSize; ++i){
            val+=mAvgBuf[i];
        }
        return val/cGainCalibAvgSize;
    }
    
private:
    
    // raw lux value [0,4096]
    int mLuxRaw{0}; 

    int mSatCounter{0};
    int mSatCounterMax{10};
    bool mSaturated{false};

    // abs luma value in the range [0,1050]
    int mLuxScaled{0}; 
    
    //---------------------------------------------------------
    // Gain Calibration
    
    bool mCalibrating{false};

    // slow movavg on raw input for gain calibration 
    int mLuxRawAvg{0}; 
    int mAvgBuf[cGainCalibAvgSize];
    int mAvgInd{0};

    // current gain
    int mGain{0};
    
    unsigned long mTimestamp{0};

    // Photodiode gain controller
    MCP4151Controller mDigiPot;

    // debug
    static const int cPrintCount = 1;
    int mPrintCounter{0};
};


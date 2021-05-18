//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// KOMOREBI KIT, 2021 
//
// Created by Matteo Marangoni & Dieter Vandoren 
// Programming by Riccardo Marogna
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MCP4151 digital pot Controller
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma once

#include <SPI.h>
#include "Arduino.h"
#include "LarvaDefs.hpp"

class MCP4151Controller
{

public:
    MCP4151Controller(){}
    ~MCP4151Controller(){}

    // init @ setup
    void Init(){
        pinMode(slaveSelectPin, OUTPUT); //??
        SPI.begin();
        digitalPotWrite(0, 0); // initial value
    }

    void SetGain( const int acGain ){
        int val = acGain;
        val = val>255 ? 255 : val;
        val = val<0 ? 0 : val;
        mGain = val;

        // note: pot/gain is inverse
        int pot = 255 - mGain;
        digitalPotWrite(0, pot);
    }

    int GetGain(){ return mGain; }

    // Write to digital pot to control photosensor amp
    void digitalPotWrite(int address, int value) 
    {
        // take the SS pin low to select the chip:
        digitalWrite(slaveSelectPin, LOW);
        
        //  send in the address and value via SPI:
        SPI.transfer(address);
        SPI.transfer(value);
        
        // take the SS pin high to de-select the chip:
        digitalWrite(slaveSelectPin, HIGH);
    }

private:
    const int slaveSelectPin = 5;

    // 0...255
    int mGain{0};

    unsigned long mPrevTimeStampMs{0};
};

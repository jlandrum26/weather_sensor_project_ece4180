/* 
  *@section LICENSE
  * This program is free software; you can redistribute it and/or modify 
  * it under the terms of the GNU General Public License as published by 
  * the Free Software Foundation; either version 2 of the License, or 
  * (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful, but 
  * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
  * for more details.
  * 
  * You should have received a copy of the GNU General Public License along 
  * with this program; if not, write to the Free Software Foundation, Inc., 
  * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  * 
  *@section NOTES 
  *MPL115A2 Sensor Library for mbed 
  *
  *created for arduino by R.N <zan73722@gmail.com>
  *ported to mbed by joe holdsworth <joeholdsworth@gmail.com>
  *
  *2012-05-07 Ported mbed beta version - tested on LPC1768
*/

#ifndef MPL115A2_H
#define MPL115A2_H

#include "mbed.h"

class MPL115A2
{
    private:
        DigitalOut _shdnPin;
        bool _bShutdown;
        short _i2c_address;
        short _sia0, _sib1, _sib2, _sic12, _sic11, _sic22;
        unsigned short _uiPadc, _uiTadc;
        I2C *_i2c;
        Serial *_pc;
        bool _debug;
       
        
    public:
        MPL115A2(I2C *i2c, const PinName shdnPin = p21);
        MPL115A2(I2C *i2c, Serial *pc, const PinName shdnPin = p21); //pass serial for debug
        void begin();
        void shutdown();
        void ReadSensor();
        float GetPressure();
        float GetTemperature();
};
#endif
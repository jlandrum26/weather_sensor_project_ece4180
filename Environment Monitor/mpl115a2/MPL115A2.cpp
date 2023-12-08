/******************************************************************************
 * Includes
 ******************************************************************************/
#include "MPL115A2.h"
#include "mbed.h"

/******************************************************************************
 * Constructors
 ******************************************************************************/
MPL115A2::MPL115A2(I2C *i2c, const PinName shdnPin):
_i2c_address(0xc0), _i2c(i2c), _shdnPin(shdnPin),_debug(false),_pc(NULL)
{
   
}

MPL115A2::MPL115A2(I2C *i2c, Serial *pc, const PinName shdnPin):
_i2c_address(0xc0), _i2c(i2c), _shdnPin(shdnPin), _pc(pc), _debug(true)
{
   
}

/******************************************************************************
 * Global Functions
 ******************************************************************************/
void MPL115A2::ReadSensor()
{
    int res;

    //wakeup
    _shdnPin=1;
    _bShutdown = false;
    wait(0.001); // wait the device to be ready
    
    // start AD conversions
    char cmd[2];
    cmd[0]=0x12;
    cmd[1]=0x01;
    res = _i2c->write(_i2c_address,cmd,2);
    
    if(_debug)
    {
        if(res==0)
        {
            _pc->printf("Success: Requested temp and pressure \n");
        }
        else
        {
            _pc->printf("Failed : Requested temp and pressure %d\n", res);
        }
    }
    
    wait(0.01);
    cmd[0]=0x00;
    res = _i2c->write(_i2c_address,cmd,1);
    
    if(_debug)
    {
        if(res==0)
        {
            _pc->printf("Success: Select temp and pressure register \n");
        }
        else
        {
            _pc->printf("Failed : Select temp and pressure register %d\n", res);
        }
    }    
    
    // compensation
    char data[4];   
    res = _i2c->read(_i2c_address, data, 4);
    
    if(_debug)
    {
        if(res==0)
        {
            _pc->printf("Success: Read temp and pressure register \n");
        }
        else
        {
            _pc->printf("Failed : Read temp and pressure register %d\n", res);
        }
    }    
    
    if(res == 0)
    {   
        _uiPadc = (unsigned short) data[0] << 8;
        _uiPadc += (unsigned short) data[1] & 0x00FF;
        _uiTadc = (unsigned short) data[2] << 8;
        _uiTadc += (unsigned short) data[3] & 0x00FF;
        
        // Coefficient 9 equation compensation
        _uiPadc = _uiPadc >> 6;
        _uiTadc = _uiTadc >> 6;
    }    
}
 
 
/**********************************************************
 * GetPressure
 *  Gets the current pressure from the sensor.
 *
 * @return float - The local pressure in kPa
 **********************************************************/
float MPL115A2::GetPressure()
{
    short siPcomp;
    int lt1, lt2, lt3, si_c11x1, si_a11, si_c12x2;
    int si_a1, si_c22x2, si_a2, si_a1x1, si_y1, si_a2x2;
    
    // Step 1 c11x1 = c11 * Padc
    lt1 = (int) _sic11;
    lt2 = (int) _uiPadc;
    lt3 = lt1*lt2;
    si_c11x1 = (int) lt3;
    
    // Step 2 a11 = b1 + c11x1
    lt1 = ((int)_sib1)<<14;
    lt2 = (int) si_c11x1;
    lt3 = lt1 + lt2;
    si_a11 = (int)(lt3>>14);
    
    // Step 3 c12x2 = c12 * Tadc
    lt1 = (int) _sic12;
    lt2 = (int) _uiTadc;
    lt3 = lt1*lt2;
    si_c12x2 = (int)lt3;
    
    // Step 4 a1 = a11 + c12x2
    lt1 = ((int)si_a11<<11);
    lt2 = (int)si_c12x2;
    lt3 = lt1 + lt2;
    si_a1 = (int) lt3>>11;
    
    // Step 5 c22x2 = c22*Tadc
    lt1 = (int)_sic22;
    lt2 = (int)_uiTadc;
    lt3 = lt1 * lt2;
    si_c22x2 = (int)(lt3);
    
    // Step 6 a2 = b2 + c22x2
    lt1 = ((int)_sib2<<15);
    lt2 = ((int)si_c22x2>1);
    lt3 = lt1+lt2;
    si_a2 = ((int)lt3>>16);
    
    // Step 7 a1x1 = a1 * Padc
    lt1 = (int)si_a1;
    lt2 = (int)_uiPadc;
    lt3 = lt1*lt2;
    si_a1x1 = (int)(lt3);
    
    // Step 8 y1 = a0 + a1x1
    lt1 = ((int)_sia0<<10);
    lt2 = (int)si_a1x1;
    lt3 = lt1+lt2;
    si_y1 = ((int)lt3>>10);
    
    // Step 9 a2x2 = a2 * Tadc
    lt1 = (int)si_a2;
    lt2 = (int)_uiTadc;
    lt3 = lt1*lt2;
    si_a2x2 = (int)(lt3);
    
    // Step 10 pComp = y1 + a2x2
    lt1 = ((int)si_y1<<10);
    lt2 = (int)si_a2x2;
    lt3 = lt1+lt2;
    
    // Fixed point result with rounding
    siPcomp = ((int)lt3>>13);
    
    if(_debug)
    {       
        _pc->printf("Success: Calculated Pressure Compesation Value %d\n", siPcomp);
    }        
    
    // decPcomp is defined as a floating point number
    // Conversion to decimal value from 1023 ADC count value
    // ADC counts are 0 to 1023, pressure is 50 to 115kPa respectively
    return (((650.0/1023.0)*siPcomp)+500);
}


/**********************************************************
 * GetTemperature
 *  Gets the current temperature from the sensor.
 *
 * @return float - The local pressure in °C 
 **********************************************************/
float MPL115A2::GetTemperature()
{
    return (25 + ((_uiTadc - 498.0) / -5.35));
}


/**********************************************************
 * Begin
 *  Gets the coefficients from the sensor.
 *
 **********************************************************/
void MPL115A2::begin()
{
    short sia0MSB, sia0LSB;
    short sib1MSB, sib1LSB;
    short sib2MSB, sib2LSB;
    short sic12MSB, sic12LSB;
    short sic11MSB, sic11LSB;
    short sic22MSB, sic22LSB;
    
    //wakeup
    _shdnPin=1;
    _bShutdown = false;
    wait(0.001); // wait the device to be ready

    //send request for coefficients
    char cmd[1];
    cmd[0] = 0x04;
    _i2c->write(_i2c_address, cmd, 1);

    // read out coefficients
    char coeff[12];
    int res = _i2c->read(_i2c_address, coeff, 12);
    if (res == 0)
    {
        sia0MSB = coeff[0];
        sia0LSB = coeff[1];
        sib1MSB = coeff[2];
        sib1LSB = coeff[3];
        sib2MSB = coeff[4];
        sib2LSB = coeff[5];
        sic12MSB = coeff[6];
        sic12LSB = coeff[7];
        sic11MSB = coeff[8];
        sic11LSB = coeff[9];
        sic22MSB = coeff[10];
        sic22LSB = coeff[11];
    
    
        // Placing coefficients into 16-bit Variables
        // a0
        _sia0 = (signed int) sia0MSB << 8;
        _sia0 += (signed int) sia0LSB & 0x00FF;
        // b1
        _sib1 = (signed int) sib1MSB << 8;
        _sib1 += (signed int) sib1LSB & 0x00FF;
        // b2
        _sib2 = (signed int) sib2MSB << 8;
        _sib2 += (signed int) sib2LSB & 0x00FF;
        // c12
        _sic12 = (signed int) sic12MSB << 8;
        _sic12 += (signed int) sic12LSB & 0x00FF;
        // c11
        _sic11 = (signed int) sic11MSB << 8;
        _sic11 += (signed int) sic11LSB & 0x00FF;
        // c22
        _sic22 = (signed int) sic22MSB << 8;
        _sic22 += (signed int) sic22LSB & 0x00FF;
    }
}


void MPL115A2::shutdown()
{
    _shdnPin = 0;
    _bShutdown = true;
}
/** @file charToStringWaveform.c
 *  @author Freddie Akeroyd, STFC (freddie.akeroyd@stfc.ac.uk)
 *  @ingroup asub_functions
 *
 *  Copy a CHAR waveform record into a STRING waveform record. If this is done by
 *  a normal CAPUT the character byte codes are not preserved
 *
 *  It expect the A input to be the waveform data and B to be "NORD" (number of elements)
 *  it write its output to VALA
 */
#include <string.h>
#include <registryFunction.h>
#include <aSubRecord.h>
#include <menuFtype.h>
#include <errlog.h>

#include <epicsExport.h>
/**
 *  Convert a character waveform into a string waveform
 *  @ingroup asub_functions
 *  @param[in] prec Pointer to aSub record
 */

typedef struct 
{
    unsigned spec_select : 4;
    unsigned overflow_select : 4;
    unsigned dsp_gain : 2;
    unsigned accept_all : 1;
    unsigned lld : 22;
    unsigned start_slope_cut : 22;
    unsigned uld : 22;   
    unsigned second_deriv_cut : 22;
    unsigned max_slope_cut : 22;
    unsigned prescale_seclection : 22;
    unsigned start_slope_bits : 3;
    unsigned max_slope_bits : 3;
    unsigned second_deriv_bits : 3;
    unsigned unused : 8;    
} ADCControlBits;

static long ADCReadControlReg(aSubRecord *prec) 
{
    ADCControlBits adc;
    if (prec->fta != menuFtypeULONG || prec->ftva != menuFtypeLONG)
    {
         errlogPrintf("%s incorrect input type. A (ULONG), VALA (LONG)", prec->name);
		 return -1;
    }
    if (prec->noa != 8)
    {
         errlogPrintf("%s incorrect input array length. ", prec->name);
		 return -1;        
    }
    if ( (sizeof(ADCControlBits) != 5 * sizeof(epicsUInt32)) || (sizeof(ADCControlBits) > prec->noa * sizeof(epicsUInt32)) )
    {
         errlogPrintf("%s incorrect ADCControlBits size", prec->name);
		 return -1;        
    }        
    memcpy(&adc, prec->a, sizeof(ADCControlBits));
    *(epicsInt32*)(prec->vala) = adc.spec_select;
    *(epicsInt32*)(prec->valb) = adc.overflow_select;
    *(epicsInt32*)(prec->valc) = adc.dsp_gain;
    *(epicsInt32*)(prec->vald) = adc.accept_all;
    *(epicsInt32*)(prec->vale) = adc.lld;
    return 0;
}
    
static long ADCWriteControlReg(aSubRecord *prec) 
{
    ADCControlBits adc;
    if (prec->fta != menuFtypeLONG || prec->ftva != menuFtypeULONG)
    {
         errlogPrintf("%s incorrect input type. A (LONG), VALA (ULONG)", prec->name);
		 return -1;
    }
    if (prec->nova != 8)
    {
         errlogPrintf("%s incorrect output] array length. ", prec->name);
		 return -1;        
    }
    if ( (sizeof(ADCControlBits) != 5 * sizeof(epicsUInt32)) || (sizeof(ADCControlBits) > prec->nova * sizeof(epicsUInt32)) )
    {
         errlogPrintf("%s incorrect ADCControlBits size", prec->name);
		 return -1;        
    }        
    memset(&adc, 0, sizeof(adc));
    memset(prec->vala, 0, prec->nova * sizeof(epicsUInt32));
    adc.spec_select = *(epicsInt32*)(prec->a);
    adc.overflow_select = *(epicsInt32*)(prec->b);
    adc.dsp_gain = *(epicsInt32*)(prec->c);
    adc.accept_all =*(epicsInt32*)(prec->d);
    adc.lld = *(epicsInt32*)(prec->e);
    memcpy(prec->vala, &adc, sizeof(ADCControlBits));
    return 0;
}

epicsRegisterFunction(ADCReadControlReg); /* must also be mentioned in daedataSupport.dbd */
epicsRegisterFunction(ADCWriteControlReg); /* must also be mentioned in daedataSupport.dbd */

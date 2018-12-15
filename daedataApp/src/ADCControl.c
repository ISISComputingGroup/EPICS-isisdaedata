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
#include <stdint.h>

#include <epicsExport.h>
/**
 *  Convert a character waveform into a string waveform
 *  @ingroup asub_functions
 *  @param[in] prec Pointer to aSub record
 */

//struct ADCControlBits
//{
//    unsigned spec_select : 4;
//    unsigned overflow_select : 4;
//    unsigned dsp_gain : 2;
//    unsigned accept_all : 1;
//    unsigned lld : 22;
//    unsigned start_slope_cut : 22;
//    unsigned uld : 22;   
//    unsigned second_deriv_cut : 22;
//    unsigned max_slope_cut : 22;
//    unsigned prescale_seclection : 22;
//    unsigned start_slope_bits : 3;
//    unsigned max_slope_bits : 3;
//    unsigned second_deriv_bits : 3;
//    unsigned misplace_bits : 2;    
//    unsigned unused : 6;    
//};

/*
 * bit fields must all fit into a complete word. We use int64 as it 
 * requires less splitting of fields, but still have to split "uld" and "prescale"
 * and use macros to set them
 */
typedef struct
{
    uint64_t spec_select : 4;
    uint64_t overflow_select : 4;
    uint64_t dsp_gain : 2;
    uint64_t accept_all : 1;
	uint64_t lld : 22;
    uint64_t start_slope_cut : 22;
    uint64_t uld0 : 9;
    
    uint64_t uld1 : 13;   
    uint64_t second_deriv_cut : 22;
    uint64_t max_slope_cut : 22;
    uint64_t prescale_selection0 : 7;

    uint64_t prescale_selection1 : 15;
    uint64_t start_slope_bits : 3;
    uint64_t max_slope_bits : 3;
    uint64_t second_deriv_bits : 3;
    uint64_t misplace_bits : 2;    
    uint64_t unused : 38;    
} ADCControlBits;

#define ADC_STRUCT_WORDS 6 /* how many 32 bit integers in struct adc */
#define ADC_STRUCT_SIZE (ADC_STRUCT_WORDS * sizeof(epicsUInt32)) /* how big struct adc should be - used as a test of byte alignment */

/* 
 * macros to handle values like uld that are split over boundaries as uld0 and uld1
 * uld0 is the lower bits, uld1 the higher so set field argument to "uld" and bits argument 
 * to the number of bits in lower uld0 part
 */

#define get_field(adc,field,bits)      ( adc.field##0 | (adc.field##1 << bits) )
#define set_field(adc,field,bits,val)    adc.field##0 = val & ((1 << bits) - 1); adc.field##1 = (val >> bits)

#define get_uld(adc)       get_field(adc,uld,9)
#define set_uld(adc,val)   set_field(adc,uld,9,val)

#define get_prescale_selection(adc)       get_field(adc,prescale_selection,7)
#define set_prescale_selection(adc,val)   set_field(adc,prescale_selection,7,val)

static long ADCReadControlReg(aSubRecord *prec)
{
    ADCControlBits adc;
    // static_assert(sizeof(adc) == ADC_STRUCT_SIZE, "struct adc size wrong");
    if (prec->fta != menuFtypeULONG || prec->ftva != menuFtypeLONG)
    {
         errlogSevPrintf(errlogMajor, "%s incorrect input type. A (ULONG), VALA (LONG)\n", prec->name);
		 return -1;
    }
    if (prec->noa != 8)
    {
         errlogSevPrintf(errlogMajor, "%s incorrect input array length %d != 8.\n", prec->name, prec->noa);
		 return -1;        
    }
    if (sizeof(adc) != ADC_STRUCT_SIZE)
    {
         errlogSevPrintf(errlogMajor, "%s incorrect ADCControlBits size %lu bytes\n", prec->name, sizeof(adc));
		 return -1;        
    }        
    if (sizeof(adc) > prec->noa * sizeof(epicsUInt32))
    {
         errlogSevPrintf(errlogMajor, "%s NOA not big enough %d\n", prec->name, prec->nova);
		 return -1;        
    }        
    memcpy(&adc, prec->a, sizeof(adc));
    *(epicsInt32*)(prec->vala) = adc.spec_select;
    *(epicsInt32*)(prec->valb) = adc.overflow_select;
    *(epicsInt32*)(prec->valc) = adc.dsp_gain;
    *(epicsInt32*)(prec->vald) = adc.accept_all;
    *(epicsInt32*)(prec->vale) = adc.lld;
    *(epicsInt32*)(prec->valf) = adc.start_slope_cut;
    *(epicsInt32*)(prec->valg) = get_uld(adc);
    *(epicsInt32*)(prec->valh) = adc.second_deriv_cut;
    *(epicsInt32*)(prec->vali) = adc.max_slope_cut;
    *(epicsInt32*)(prec->valj) = get_prescale_selection(adc);
    *(epicsInt32*)(prec->valk) = adc.start_slope_bits;
    *(epicsInt32*)(prec->vall) = adc.max_slope_bits;
    *(epicsInt32*)(prec->valm) = adc.second_deriv_bits;
    *(epicsInt32*)(prec->valn) = adc.misplace_bits;
	return 0;
}
    
static long ADCWriteControlReg(aSubRecord *prec) 
{
    ADCControlBits adc;
    // static_assert(sizeof(adc) == ADC_STRUCT_SIZE, "struct adc size wrong");
    if (prec->fta != menuFtypeLONG || prec->ftva != menuFtypeULONG)
    {
         errlogSevPrintf(errlogMajor, "%s incorrect input type. A (LONG), VALA (ULONG)\n", prec->name);
		 return -1;
    }
    if (prec->nova != 8)
    {
         errlogSevPrintf(errlogMajor, "%s incorrect output array length %d != 8.\n", prec->name, prec->nova);
		 return -1;        
    }
    if (sizeof(adc) != ADC_STRUCT_SIZE)
    {
         errlogSevPrintf(errlogMajor, "%s incorrect ADCControlBits size %lu bytes\n", prec->name, sizeof(adc));
		 return -1;        
    }        
    if (sizeof(adc) > prec->nova * sizeof(epicsUInt32))
    {
         errlogSevPrintf(errlogMajor, "%s nova not big enough %d\n", prec->name, prec->nova);
		 return -1;        
    }        
    memset(&adc, 0, sizeof(adc));
    memset(prec->vala, 0, prec->nova * sizeof(epicsUInt32));
    adc.spec_select = *(epicsInt32*)(prec->a);
    adc.overflow_select = *(epicsInt32*)(prec->b);
    adc.dsp_gain = *(epicsInt32*)(prec->c);
    adc.accept_all =*(epicsInt32*)(prec->d);
    adc.lld = *(epicsInt32*)(prec->e);
    adc.start_slope_cut = *(epicsInt32*)(prec->f);
    set_uld(adc, *(epicsInt32*)(prec->g));
    adc.second_deriv_cut = *(epicsInt32*)(prec->h);
    adc.max_slope_cut = *(epicsInt32*)(prec->i);
    set_prescale_selection(adc, *(epicsInt32*)(prec->j));
    adc.start_slope_bits = *(epicsInt32*)(prec->k);
    adc.max_slope_bits = *(epicsInt32*)(prec->l);
    adc.second_deriv_bits = *(epicsInt32*)(prec->m);
    adc.misplace_bits = *(epicsInt32*)(prec->n);
    memcpy(prec->vala, &adc, sizeof(adc));
    prec->neva = ADC_STRUCT_WORDS;
    return 0;
}

epicsRegisterFunction(ADCReadControlReg); /* must also be mentioned in daedataSupport.dbd */
epicsRegisterFunction(ADCWriteControlReg); /* must also be mentioned in daedataSupport.dbd */

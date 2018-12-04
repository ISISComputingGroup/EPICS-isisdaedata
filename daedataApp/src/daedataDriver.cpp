#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <stdint.h>

#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <iocsh.h>

#include "daedataDriver.h"
#include "convertToString.h"
#include "daedataUDP.h"

#include <macLib.h>
#include <epicsGuard.h>

#include <epicsExport.h>

static epicsThreadOnceId onceId = EPICS_THREAD_ONCE_INIT;

static const char *driverName="daedataDriver";

template<typename T>
asynStatus daedataDriver::writeValue(asynUser *pasynUser, const char* functionName, T value)
{
    int function = pasynUser->reason;
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
	unsigned address;
	getParamName(function, &paramName);
	try
	{
		if (function == P_Address)
		{
			address = atoi((const char*)pasynUser->userData);
			m_udp->writeData(address, &value, 1, true, pasynUser);
			setIntegerParam(P_AddressW, address);
		}
		else
		{
			throw std::runtime_error("invalid parameter");
		}
		callParamCallbacks();
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%s\n", 
              driverName, functionName, function, paramName, convertToString(value).c_str());
		return asynSuccess;
	}
	catch(const std::exception& ex)
	{
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%s, error=%s", 
                  driverName, functionName, status, function, paramName, convertToString(value).c_str(), ex.what());
		return asynError;
	}
}

template<typename T>
asynStatus daedataDriver::readValue(asynUser *pasynUser, const char* functionName, T* value)
{
	int function = pasynUser->reason;
	unsigned address;
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
	getParamName(function, &paramName);
	try
	{
		if (function == P_Address)
		{
			address = atoi((const char*)pasynUser->userData);
			m_udp->readData(address, value, 1, pasynUser);
			setIntegerParam(P_AddressR, address);
		}
		else
		{
			throw std::runtime_error("invalid parameter");
		}
		callParamCallbacks();
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s, value=%s\n", 
              driverName, functionName, function, paramName, convertToString(*value).c_str());
		return asynSuccess;
	}
	catch(const std::exception& ex)
	{
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, value=%s, error=%s", 
                  driverName, functionName, status, function, paramName, convertToString(*value).c_str(), ex.what());
		return asynError;
	}
}

template<typename T>
asynStatus daedataDriver::writeArray(asynUser *pasynUser, const char* functionName, T *value, size_t nElements)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  const char *paramName = NULL;
	unsigned address;
	getParamName(function, &paramName);

	try
	{
		if (function == P_Address)
		{
			address = atoi((const char*)pasynUser->userData);
			m_udp->writeData(address, value, nElements, true, pasynUser);
			setIntegerParam(P_AddressW, address);
		}
		else
		{
			throw std::runtime_error("invalid parameter");
		}
		callParamCallbacks();
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s\n", 
              driverName, functionName, function, paramName);
		return asynSuccess;
	}
	catch(const std::exception& ex)
	{
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, error=%s", 
                  driverName, functionName, status, function, paramName, ex.what());
		return asynError;
	}
}

template<typename T>
asynStatus daedataDriver::readArray(asynUser *pasynUser, const char* functionName, T *value, size_t nElements, size_t *nIn)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
   unsigned address;
  const char *paramName = NULL;
	getParamName(function, &paramName);

	try
	{
		if (function == P_Address)
		{
			address = atoi((const char*)pasynUser->userData);
			m_udp->readData(address, value, nElements, pasynUser);
			setIntegerParam(P_AddressR, address);
		}
		else
		{
			throw std::runtime_error("invalid parameter");
		}
		callParamCallbacks();
		*nIn = nElements;
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
              "%s:%s: function=%d, name=%s\n", 
              driverName, functionName, function, paramName);
		return asynSuccess;
	}
	catch(const std::exception& ex)
	{
		*nIn = 0;
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: status=%d, function=%d, name=%s, error=%s", 
                  driverName, functionName, status, function, paramName, ex.what());
		return asynError;
	}
}

asynStatus daedataDriver::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
	return writeValue(pasynUser, "writeInt32", (epicsUInt32)value);
}

asynStatus daedataDriver::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
	return readValue(pasynUser, "readInt32", (epicsUInt32*)value);
}

asynStatus daedataDriver::readInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements, size_t *nIn)
{
    return readArray(pasynUser, "readInt32Array", (epicsUInt32*)value, nElements, nIn);
}

asynStatus daedataDriver::writeInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements)
{
    return writeArray(pasynUser, "writeInt32Array", (epicsUInt32*)value, nElements);
}

asynStatus daedataDriver::readInt16Array(asynUser *pasynUser, epicsInt16 *value, size_t nElements, size_t *nIn)
{
  static const char* functionName = "readInt16Array";
  int function = pasynUser->reason;
  const char *paramName = NULL;
  asynStatus status;
	getParamName(function, &paramName);
	*nIn = 0;
	if (nElements % 2 == 0)
	{
		size_t nIn32 = 0;
		epicsInt32* tmpval = new epicsInt32[nElements / 2];
		if ( (status = readArray(pasynUser, functionName, (epicsUInt32*)tmpval, nElements / 2, &nIn32)) == asynSuccess )
		{
//			for(int i=0, j =0; i<nIn32; ++i, j += 2)
//			{
//		        value[j] = (tmpval[i] >> 16) & 0xffff;
//		        value[j+1] = tmpval[i] & 0xffff;
//			}
			memcpy(value, tmpval, nIn32 * sizeof(epicsInt32));
		    *nIn = nIn32 * 2;
		}
		delete[] tmpval;
		return status;
	}
	else
	{
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: function=%d, name=%s, error=%s", 
                  driverName, functionName, function, paramName, "nElements must be even");
		return asynError;		
	}
}

asynStatus daedataDriver::writeInt16Array(asynUser *pasynUser, epicsInt16 *value, size_t nElements)
{
  static const char* functionName = "writeInt16Array";
  int function = pasynUser->reason;
  const char *paramName = NULL;
  asynStatus status;
	getParamName(function, &paramName);
	if (nElements % 2 == 0)
	{
		size_t nOut32 = nElements / 2;
		epicsInt32* tmpval = new epicsInt32[nOut32];
		memcpy(tmpval, value, nOut32 * sizeof(epicsInt32));
		status = writeArray(pasynUser, functionName, (epicsUInt32*)tmpval, nOut32);
		delete[] tmpval;
		return status;
	}
	else
	{
        epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                  "%s:%s: function=%d, name=%s, error=%s", 
                  driverName, functionName, function, paramName, "nElements must be even");
		return asynError;		
	}
}


/// Constructor for the isisdaeDriver class.
/// Calls constructor for the asynPortDriver base class.
/// \param[in] dcomint DCOM interface pointer created by lvDCOMConfigure()
/// \param[in] portName @copydoc initArg0
daedataDriver::daedataDriver(const char *portName, const char* host) 
   : asynPortDriver(portName, 
                    1, /* maxAddr */ 
                    NUM_ISISDAE_PARAMS,
                    asynInt32Mask | asynInt32ArrayMask | asynInt16ArrayMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynInt32ArrayMask | asynInt16ArrayMask,  /* Interrupt mask */
                    ASYN_CANBLOCK , /* asynFlags.  This driver can block but it is not multi-device */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0)	/* Default stack size*/					
{
    const char *functionName = "daedataDriver";
//	epicsThreadOnce(&onceId, initCOM, NULL);

	m_udp = new DAEDataUDP(host);

	createParam(P_AddressString, asynParamInt32, &P_Address);
	createParam(P_AddressWString, asynParamInt32, &P_AddressW);
	createParam(P_AddressRString, asynParamInt32, &P_AddressR);

    // Create the thread for background tasks (not used at present, could be used for I/O intr scanning) 
    if (epicsThreadCreate("isisdaePoller",
                          epicsThreadPriorityMedium,
                          epicsThreadGetStackSize(epicsThreadStackMedium),
                          (EPICSTHREADFUNC)pollerThreadC, this) == 0)
    {
        printf("%s:%s: epicsThreadCreate failure\n", driverName, functionName);
        return;
    }
}

void daedataDriver::pollerThreadC(void* arg)
{ 
    daedataDriver* driver = (daedataDriver*)arg; 
	driver->pollerThread();
}

void daedataDriver::pollerThread()
{
    static const char* functionName = "isisdaePoller";
	while(false)
	{
		lock();
//		setIntegerParam(P_GoodFrames, m_iface->getGoodFrames());
        
		callParamCallbacks();
		unlock();
		epicsThreadSleep(1.0);
	}
}	

asynStatus daedataDriver::drvUserCreate(asynUser *pasynUser, const char* drvInfo, const char** pptypeName, size_t* psize)
{
   const char *functionName = "drvUserCreate";
   if (strncmp(drvInfo, "0x", 2) == 0)
   {
       pasynUser->reason = P_Address;
       pasynUser->userData = epicsStrDup(drvInfo);
       asynPrint(pasynUser, ASYN_TRACE_FLOW,
          "%s:%s: index=%d address=%s\n", 
          driverName, functionName, pasynUser->reason, (const char*)pasynUser->userData);
       return asynSuccess;
   }
   else
   {
       return asynPortDriver::drvUserCreate(pasynUser, drvInfo, pptypeName, psize);
   }
}

asynStatus daedataDriver::drvUserDestroy(asynUser *pasynUser)
{
   const char *functionName = "drvUserDestroy";
   if ( pasynUser->reason == P_Address  )
   {
      asynPrint(pasynUser, ASYN_TRACE_FLOW,
          "%s:%s: index=%d address=%s\n", 
          driverName, functionName, pasynUser->reason, (const char*)pasynUser->userData);
      free(pasynUser->userData);
      pasynUser->userData = NULL;
      return asynSuccess;
  }
  else
  {
      return asynPortDriver::drvUserDestroy(pasynUser);
  }
}

extern "C" {

/// EPICS iocsh callable function to call constructor of lvDCOMInterface().
/// \param[in] portName @copydoc initArg0
/// \param[in] configSection @copydoc initArg1
/// \param[in] configFile @copydoc initArg2
/// \param[in] host @copydoc initArg3
/// \param[in] options @copydoc initArg4
/// \param[in] progid @copydoc initArg5
/// \param[in] username @copydoc initArg6
/// \param[in] password @copydoc initArg7
int daedataConfigure(const char *portName, const char *host)
{
	try
	{
			new daedataDriver(portName, host);
			return(asynSuccess);
	}
	catch(const std::exception& ex)
	{
		std::cerr << "daedataConfigure failed: " << ex.what() << std::endl;
		return(asynError);
	}
}

// EPICS iocsh shell commands 

static const iocshArg initArg0 = { "portName", iocshArgString};			///< The name of the asyn driver port we will create
static const iocshArg initArg1 = { "host", iocshArgString};				///< host name where LabVIEW is running ("" for localhost) 

static const iocshArg * const initArgs[] = { &initArg0,
											 &initArg1 };

static const iocshFuncDef initFuncDef = {"daedataConfigure", sizeof(initArgs) / sizeof(iocshArg*), initArgs};

static void initCallFunc(const iocshArgBuf *args)
{
    daedataConfigure(args[0].sval, args[1].sval);
}

static void daedataRegister(void)
{
    iocshRegister(&initFuncDef, initCallFunc);
}

epicsExportRegistrar(daedataRegister);

}


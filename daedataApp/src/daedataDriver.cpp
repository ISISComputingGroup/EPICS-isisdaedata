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
	getParamName(function, &paramName);
	try
	{
		if (function == P_SVN_VERSION)
		{
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
    asynStatus status = asynSuccess;
    const char *paramName = NULL;
	getParamName(function, &paramName);
	try
	{
		if (function == P_SVN_VERSION)
		{
			m_udp->readData(0x0008FFE4, value, 1, pasynUser);
			setIntegerParam(P_SVN_VERSION, *value);
		}
		else if (function == P_FIRMWARE_VERSION)
		{
			m_udp->readData(0x0008FFE0, value, 1, pasynUser);
			setIntegerParam(P_FIRMWARE_VERSION, *value);
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
	getParamName(function, &paramName);

	try
	{
		if (function == P_CHANNEL_POSITION)
		{
			m_udp->writeData(0x400, value, nElements, true, pasynUser);
			doCallbacksInt32Array(value, nElements, P_CHANNEL_POSITION, 0);
		}
		else
		{
			throw std::runtime_error("invalid parameter");
		}
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
  const char *paramName = NULL;
	getParamName(function, &paramName);

	try
	{
		if (function == P_CHANNEL_POSITION)
		{
			m_udp->readData(0x400, value, nElements, pasynUser);
//			doCallbacksInt32Array(value, nElements, P_CHANNEL_POSITION, 0);   nedessary?
		}
		else
		{
			throw std::runtime_error("invalid parameter");
		}
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
	return writeValue(pasynUser, "writeInt32", value);
}

asynStatus daedataDriver::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
	return readValue(pasynUser, "readInt32", value);
}

asynStatus daedataDriver::readInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements, size_t *nIn)
{
    return readArray(pasynUser, "readInt32Array", value, nElements, nIn);
}

asynStatus daedataDriver::writeInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements)
{
    return writeArray(pasynUser, "writeInt32Array", value, nElements);
}


/// Constructor for the isisdaeDriver class.
/// Calls constructor for the asynPortDriver base class.
/// \param[in] dcomint DCOM interface pointer created by lvDCOMConfigure()
/// \param[in] portName @copydoc initArg0
daedataDriver::daedataDriver(const char *portName, const char* host) 
   : asynPortDriver(portName, 
                    0, /* maxAddr */ 
                    NUM_ISISDAE_PARAMS,
                    asynInt32Mask | asynInt32ArrayMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynInt32ArrayMask,  /* Interrupt mask */
                    ASYN_CANBLOCK , /* asynFlags.  This driver can block but it is not multi-device */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0)	/* Default stack size*/					
{
    const char *functionName = "daedataDriver";
//	epicsThreadOnce(&onceId, initCOM, NULL);

	m_udp = new DAEDataUDP(host);

	createParam(P_SVN_VERSIONString, asynParamInt32, &P_SVN_VERSION);
	createParam(P_FIRMWARE_VERSIONString, asynParamInt32, &P_FIRMWARE_VERSION);
	createParam(P_CHANNEL_POSITIONString, asynParamInt32Array, &P_CHANNEL_POSITION);

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


#ifndef DAEDATADRIVER_H
#define DAEDATADRIVER_H
 
#include "asynPortDriver.h"

class DAEDataUDP;

class daedataDriver : public asynPortDriver 
{
public:
    daedataDriver(const char *portName, const char* host);
 	static void pollerThreadC(void* arg);
                
    // These are the methods that we override from asynPortDriver
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
	virtual asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
    virtual asynStatus readInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements, size_t *nIn);
    virtual asynStatus writeInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements);

private:

	DAEDataUDP* m_udp;

    int P_SVN_VERSION; // int
	int P_CHANNEL_POSITION; // int array
	int P_FIRMWARE_VERSION; // int
	#define FIRST_ISISDAE_PARAM P_SVN_VERSION
	#define LAST_ISISDAE_PARAM P_FIRMWARE_VERSION
	
	void pollerThread();
	
	template<typename T> asynStatus writeValue(asynUser *pasynUser, const char* functionName, T value);
    template<typename T> asynStatus readValue(asynUser *pasynUser, const char* functionName, T* value);
    template<typename T> asynStatus writeArray(asynUser *pasynUser, const char* functionName, T *value, size_t nElements);
    template<typename T> asynStatus readArray(asynUser *pasynUser, const char* functionName, T *value, size_t nElements, size_t *nIn);

};

#define NUM_ISISDAE_PARAMS (&LAST_ISISDAE_PARAM - &FIRST_ISISDAE_PARAM + 1)

#define P_SVN_VERSIONString				"SVN_VERSION"
#define P_CHANNEL_POSITIONString		"CHANNEL_POSITION"
#define P_FIRMWARE_VERSIONString		"FIRMWARE_VERSION"

#endif /* DAEDATADRIVER_H */

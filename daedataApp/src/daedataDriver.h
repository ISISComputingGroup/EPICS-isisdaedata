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
    virtual asynStatus readInt16Array(asynUser *pasynUser, epicsInt16 *value, size_t nElements, size_t *nIn);
    virtual asynStatus writeInt16Array(asynUser *pasynUser, epicsInt16 *value, size_t nElements);

    virtual asynStatus drvUserCreate(asynUser *pasynUser, const char* drvInfo, const char** pptypeName, size_t* psize);
    virtual asynStatus drvUserDestroy(asynUser *pasynUser);

private:

	DAEDataUDP* m_udp;
	
	int P_Address; // int
	int P_AddressW; // int
	int P_AddressR; // int

	#define FIRST_ISISDAE_PARAM P_Address
	#define LAST_ISISDAE_PARAM P_AddressR
	
	void pollerThread();
	
	template<typename T> asynStatus writeValue(asynUser *pasynUser, const char* functionName, T value);
    template<typename T> asynStatus readValue(asynUser *pasynUser, const char* functionName, T* value);
    template<typename T> asynStatus writeArray(asynUser *pasynUser, const char* functionName, T *value, size_t nElements);
    template<typename T> asynStatus readArray(asynUser *pasynUser, const char* functionName, T *value, size_t nElements, size_t *nIn);

};

#define NUM_ISISDAE_PARAMS (&LAST_ISISDAE_PARAM - &FIRST_ISISDAE_PARAM + 1)

#define P_AddressString					"ADDRESS"
#define P_AddressWString				"ADDRESS_W"
#define P_AddressRString				"ADDRESS_R"

#endif /* DAEDATADRIVER_H */

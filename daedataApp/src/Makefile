TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================
# Build the IOC support library

LIBRARY_IOC += daedataSupport

daedataSupport_SRCS += daedataDriver.cpp convertToString.cpp daedataUDP.cpp ADCControl.c
daedataSupport_LIBS += asyn
daedataSupport_LIBS += $(EPICS_BASE_IOC_LIBS)
daedataSupport_SYS_LIBS_WIN32 += ws2_32

#=============================
# Build the IOC application

PROD_IOC = daedata
# daedata.dbd will be created and installed
DBD += daedata.dbd

# daedata.dbd will be made up from these files:
daedata_DBD += base.dbd
daedata_DBD += asyn.dbd
daedata_DBD += daedataSupport.dbd

# Include dbd files from all support applications:
#daedata_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#daedata_LIBS += devIocStats

# daedata_registerRecordDeviceDriver.cpp derives from daedata.dbd
daedata_SRCS += daedata_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
daedata_SRCS_DEFAULT += daedataMain.cpp
daedata_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#daedata_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

daedata_LIBS += daedataSupport asyn
daedata_LIBS_WIN32 += oncrpc
 
# Finally link to the EPICS Base libraries
daedata_LIBS += $(EPICS_BASE_IOC_LIBS)

daedata_SYS_LIBS_WIN32 += ws2_32
#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE


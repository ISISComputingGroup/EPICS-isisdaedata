#!../../bin/windows-x64/daedata

## You may have to change daedata to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase "dbd/daedata.dbd"
daedata_registerRecordDeviceDriver pdbbase

daedataConfigure("dae","192.168.1.220")

## Load record instances
dbLoadRecords("db/daedata.db","P=$(MYPVPREFIX)")

cd ${TOP}/iocBoot/${IOC}


iocInit

## Start any sequence programs
#seq sncxxx,"user=faa59Host"

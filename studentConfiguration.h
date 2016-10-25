#ifndef STUDENTCONFIGURATION_H_
#define STUDENTCONFIGURATION_H_
/****************************************************************************
    StudentConfiguration.h
    Of all the files given to you at the start of this project, this
    is the ONLY one you should ever modify.

      4.30 Jan. 2016           StudentConfiguration.h created
****************************************************************************/
/****************************************************************************
    Choose one of the operating systems below
****************************************************************************/
//#define         NT
//#define         LINUX
#define         MAC

/*****************************************************************
    The next five defines have special meaning.  They allow the
    Z502 processor to report information about its state.  From
    this, you can find what the hardware thinks is going on.
    The information produced when this debugging is on is NOT
    something that should be handed in with the project.
    Change FALSE to TRUE to enable a feature.
******************************************************************/
#define         DO_DEVICE_DEBUG                 FALSE
#define         DO_MEMORY_DEBUG                 FALSE
#define         DEBUG_LOCKS                     FALSE
#define         DEBUG_CONDITION                 FALSE
#define         DEBUG_USER_THREADS              FALSE
#define         MAX_NUMBER_OF_PROCESS              25
#define         ROOT_PCB_PRIO                     888
#define         ROOT_PCB_ID                       888
#define         RUNNING                           801
#define         SLEEPING                          802
#define         TERMINATED                        803
#define         WAITING                           804
#define         DISK_READING                      901
#define         DISK_WRITING                      902
#define         DISK_READY                        903
#include "stdio.h"
#include "stdlib.h"
#include "global.h"


typedef struct  {
    int          Mode;
    long         Field1;
    long         Field2;
    long         Field3;
    long         Field4;
} MEMORY_MAPPED_IO2;

//typedef struct{
//    int diskid;
//    int sectorid;
//    char* data;
//    int status;
//
//}myDISK ;
typedef struct {
    char *name;
    INT32 pid;
    INT32 state;
    MEMORY_MAPPED_IO2 context;
    long delay;
    
    
} PCB;

#endif /* STUDENTCONFIGURATION_H_ */


//      These are Portability enhancements






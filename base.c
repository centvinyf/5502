/************************************************************************

 This code forms the base of the operating system you will
 build.  It has only the barest rudiments of what you will
 eventually construct; yet it contains the interfaces that
 allow test.c and z502.c to be successfully built together.

 Revision History:
 1.0 August 1990
 1.1 December 1990: Portability attempted.
 1.3 July     1992: More Portability enhancements.
 Add call to SampleCode.
 1.4 December 1992: Limit (temporarily) printout in
 interrupt handler.  More portability.
 2.0 January  2000: A number of small changes.
 2.1 May      2001: Bug fixes and clear STAT_VECTOR
 2.2 July     2002: Make code appropriate for undergrads.
 Default program start is in test0.
 3.0 August   2004: Modified to support memory mapped IO
 3.1 August   2004: hardware interrupt runs on separate thread
 3.11 August  2004: Support for OS level locking
 4.0  July    2013: Major portions rewritten to support multiple threads
 4.20 Jan     2015: Thread safe code - prepare for multiprocessors
 ************************************************************************/

#include             "global.h"
#include             "syscalls.h"
#include             "protos.h"
#include             "string.h"
#include             <stdlib.h>
#include             <ctype.h>
#include             "LinkedList.h"//This is a class about linked list of PCBs

//  Allows the OS and the hardware to agree on where faults occur
extern void *TO_VECTOR[];

char *call_names[]={ "mem_read ", "mem_write", "read_mod ", "get_time ",
		"sleep    ", "get_pid  ", "create   ", "term_proc", "suspend  ",
		"resume   ", "ch_prior ", "send     ", "receive  ", "PhyDskRd ",
		"PhyDskWrt", "def_sh_ar", "Format   ", "CheckDisk", "Open_Dir ",
        "OpenFile ", "Crea_Dir ", "Crea_File", "ReadFile ", "WriteFile",
		"CloseFile", "DirContnt", "Del_Dir  ", "Del_File "};

//typedef struct{
//    myDISK* data;
//    struct disknode* next;
//    
//} disknode,*Disklist;
PCB *current_PCB = NULL;
int current_process_id = 1;
PCB *root_PCB = NULL;
Linkedlist  waiting_queue;
Linkedlist  ready_queue;
Linkedlist  all_process;
int disk_state[MAX_NUMBER_OF_DISKS]={DISK_READY};
//Disklist disk_list;
//linkedlist protocols
void *create_new_list();
//void *create_disk_list();
//int remove_from_disk_list(Disklist DL,int diskid,int sectorid);
//int get_disk_list_length(Disklist DL);
int add_to_list(Linkedlist L,PCB* new_PCB);
int remove_from_head(Linkedlist L);
PCB* get_front_PCB(Linkedlist L);
int get_length_of_list(Linkedlist L);
PCB* get_pcb_with_pid(Linkedlist L,INT32 pid);
PCB* get_pcb_with_name(Linkedlist L,char*name);

//add a pcb to ready queue
void addToReadyQueue(PCB* current_PCB){
    add_to_list(ready_queue, current_PCB);
}

//add a pcb to waiting queue
void addToWaitingQueue(PCB* current_PCB){
    add_to_list(waiting_queue, current_PCB);
}
//add a pcb to all_process list
void addToAllProcess(PCB* current_PCB)
{
    add_to_list(all_process, current_PCB);
}
int check_for_existence_by_name(char*name){
    PCB* tem=get_pcb_with_name(all_process, name);
    if(tem==NULL) return 0;
    else if(tem->state!=TERMINATED)
        return 1;
    else
        return 0;
}

void startContextofPCB(PCB* p){
    MEMORY_MAPPED_IO mmio;
    p->state=RUNNING;
    memcpy(&mmio,&p->context,sizeof(MEMORY_MAPPED_IO));
    mmio.Mode =Z502StartContext;
    mmio.Field2 = START_NEW_CONTEXT_AND_SUSPEND;
    MEM_WRITE(Z502Context, &mmio);

}
//dispatcher simply checks when ready queue is not empty, pick the first PCB and run the context
void dispatcher(){
    while(1){
    //if there is something in the ready queue, get the first pcb and run context
        if(get_length_of_list(ready_queue)>0){
            PCB *f = get_front_PCB(ready_queue);
            if(f->state!=TERMINATED)
                current_PCB = get_front_PCB(ready_queue);
            remove_from_head(ready_queue);
            
            if(current_PCB->state!=TERMINATED){
                startContextofPCB(current_PCB);
                break;
            }else{
                if(get_length_of_list(ready_queue)>0)
                    continue;
                else{
                    break;
//                    }
                }
            }
            
            
        }else{
//
            CALL(Z502Idle);
        }
    }
    
}






/************************************************************************
 INTERRUPT_HANDLER
 When the Z502 gets a hardware interrupt, it transfers control to
 this routine in the OS.
 ************************************************************************/


void InterruptHandler(void) {
	INT32 DeviceID;
	INT32 Status;

	MEMORY_MAPPED_IO mmio;       // Enables communication with hardware
//
//	static BOOL remove_this_in_your_code = TRUE; /** TEMP **/
	static INT32 how_many_interrupt_entries = 0; /** TEMP **/

	// Get cause of interrupt
	mmio.Mode = Z502GetInterruptInfo;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502InterruptDevice, &mmio);
	DeviceID = mmio.Field1;
	Status = mmio.Field2;
    
    switch(DeviceID){
        case TIMER_INTERRUPT:{
            PCB* front = get_front_PCB(waiting_queue);
            remove_from_head(waiting_queue);
            front->delay = 0;
            front->state = WAITING;
            
            //check timerqueue and reset timer
            PCB* now_front = get_front_PCB(waiting_queue);
            if(now_front!=NULL){
                mmio.Mode = Z502ReturnValue;
                mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
                MEM_READ(Z502Clock, &mmio);
                long cur_time = mmio.Field1;
                mmio.Mode = Z502Start;
                mmio.Field1 = now_front->delay-cur_time;
                mmio.Field2 = mmio.Field3 = 0;
                MEM_WRITE(Z502Timer, &mmio);
            }
            addToReadyQueue(front);
        }
        case(DISK_INTERRUPT):
        case(DISK_INTERRUPT+1):
        case(DISK_INTERRUPT+2):
        case(DISK_INTERRUPT+3):
        case(DISK_INTERRUPT+4):
        case(DISK_INTERRUPT+5):
        case(DISK_INTERRUPT+6):
        case(DISK_INTERRUPT+7):
        {
            int disk_id = DeviceID-DISK_INTERRUPT;
            if (disk_id < 0 || disk_id> MAX_NUMBER_OF_DISKS-1){//if disk id is invalid
                break;
            }else{
                mmio.Mode = Z502Status;
                mmio.Field1 = disk_id;
                mmio.Field2 = mmio.Field3 = 0;
                MEM_READ(Z502Disk, &mmio);
                if(mmio.Field2==DISK_READY)
                    disk_state[disk_id]=DISK_READY;
                else
                    break;
            }
            
            break;

        }
            
    }


	// Clear out this device - we're done with it
	mmio.Mode = Z502ClearInterruptStatus;
	mmio.Field1 = DeviceID;
	mmio.Field2 = mmio.Field3 = 0;
	MEM_WRITE(Z502InterruptDevice, &mmio);
}           // End of InterruptHandler

/************************************************************************
 FAULT_HANDLER
 The beginning of the OS502.  Used to receive hardware faults.
 ************************************************************************/

void FaultHandler(void) {
	INT32 DeviceID;
	INT32 Status;

	MEMORY_MAPPED_IO mmio;       // Enables communication with hardware

	// Get cause of interrupt
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	mmio.Mode = Z502GetInterruptInfo;
	MEM_READ(Z502InterruptDevice, &mmio);
	DeviceID = mmio.Field1;
	Status = mmio.Field2;

	printf("Fault_handler: Found vector type %d with value %d\n", DeviceID,
			Status);
	// Clear out this device - we're done with it
	mmio.Mode = Z502ClearInterruptStatus;
	mmio.Field1 = DeviceID;
	MEM_WRITE(Z502InterruptDevice, &mmio);
} // End of FaultHandler

/************************************************************************
 SVC
 The beginning of the OS502.  Used to receive software interrupts.
 All system calls come to this point in the code and are to be
 handled by the student written code here.
 The variable do_print is designed to print out the data for the
 incoming calls, but does so only for the first ten calls.  This
 allows the user to see what's happening, but doesn't overwhelm
 with the amount of data.
 ************************************************************************/






//create a process for tests according to the argument passed to it
void osCreateProcess(SYSTEM_CALL_DATA *SystemCallData){
    //illegal priority
        int prio = (int)SystemCallData->Argument[2];
    
    char* proname = (char*)malloc(sizeof(char)*32);
    strcpy(proname, (char*)SystemCallData->Argument[0]);
    
    if(prio<0){//check valid prio
        *(SystemCallData->Argument[4])= ERR_BAD_PARAM;
        return;
    }
    else if(get_length_of_list(all_process)>=MAX_NUMBER_OF_PROCESS){//check number of processes
        *(SystemCallData->Argument[4])=ERR_BAD_PARAM;
        return;
    }
    else if(check_for_existence_by_name(proname)==1){//check valid process name
        *(SystemCallData->Argument[4])=ERR_BAD_PARAM;
        return;
    }
    else{
        if(prio!=ROOT_PCB_PRIO)
            *(SystemCallData->Argument[4])=ERR_SUCCESS;
    }
    // have a new z502 context
    MEMORY_MAPPED_IO mmio;
    mmio.Mode = Z502InitializeContext;
    mmio.Field1 = 0;
    mmio.Field2 = (long*)SystemCallData->Argument[1];
//    mmio.Field2 = (long) test1;
    mmio.Field3 = (long) (void *) calloc(2, NUMBER_VIRTUAL_PAGES);
    
    MEM_WRITE(Z502Context, &mmio);
    
    // initialize a new PCB
    PCB * new_PCB = (PCB*)malloc(sizeof(PCB));
    new_PCB->name = (char*)malloc(sizeof(32));
    new_PCB->name = proname;
    if(prio==ROOT_PCB_PRIO){
        new_PCB->pid = ROOT_PCB_ID;
    }else{
    new_PCB->pid = current_process_id;
    current_process_id++;
    }
    memcpy(&new_PCB->context, &mmio, sizeof(MEMORY_MAPPED_IO));
//    new_PCB->context=&mmio;
    new_PCB->delay = 0;
//    if(current_PCB!=NULL){
//        addToReadyQueue(current_PCB);
//    }
    //if it's root process,run it
    if(prio==ROOT_PCB_PRIO){
        root_PCB = new_PCB;
        current_PCB = new_PCB;
        addToAllProcess(current_PCB);
        new_PCB->state=RUNNING;
        startContextofPCB(root_PCB);
        
    }
    //if ready queue is empty or it's the root process, then run the process
    else if(ready_queue->data==NULL&&root_PCB==NULL){
        new_PCB->state = RUNNING;
        current_PCB=new_PCB;
        addToAllProcess(current_PCB);
        startContextofPCB(new_PCB);
    
    }else//put PCB into readyqueue and all_process list
    {
        new_PCB->state = WAITING;
        addToAllProcess(new_PCB);
        addToReadyQueue(new_PCB);
    }
    
    int numofallprocess= get_length_of_list(all_process);
    if(numofallprocess>=MAX_NUMBER_OF_PROCESS)
        *(SystemCallData->Argument[4])=ERR_BAD_PARAM;
    
    
}

//Get time of a disk by reading value of shared memory
void osGetTimeOfDay(SYSTEM_CALL_DATA *SystemCallData){
    MEMORY_MAPPED_IO mmio;
    mmio.Mode = Z502ReturnValue;
    mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
    MEM_READ(Z502Clock, &mmio);
    *(SystemCallData->Argument[0]) = mmio.Field1;
}

//Sleep function
void osSleep(SYSTEM_CALL_DATA *SystemCallData){
    MEMORY_MAPPED_IO mmio;
    //get current time and set delay value
    mmio.Mode = Z502ReturnValue;
    mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
    MEM_READ(Z502Clock, &mmio);
    long cur_time = mmio.Field1;
    long delay = SystemCallData->Argument[0];
    current_PCB->delay=delay+cur_time;
    current_PCB->state=SLEEPING;
    //put currentpcb into waiting queue
    addToWaitingQueue(current_PCB);
    
    //if it's at the front, we should reset the timer
    if(get_front_PCB(waiting_queue)->pid == current_PCB->pid){
        //start the timer
        
        mmio.Mode = Z502Start;
        mmio.Field1 = delay;
        mmio.Field2 = mmio.Field3 = 0;
        MEM_WRITE(Z502Timer, &mmio);
    }
//    mmio.Mode = Z502Status;
//    mmio.Field1=mmio.Field2=mmio.Field3=mmio.Field4=0;
//    MEM_READ(Z502Timer, &mmio);
//    int status=mmio.Field1;
    
    dispatcher();
}

//get processid
void osGetProcessId(SYSTEM_CALL_DATA * SystemCallData){
    char *name = (char*)SystemCallData->Argument[0];
    PCB* searched=NULL;
    if(strcmp(name, "") != 0){
        searched = get_pcb_with_name(all_process, (char*)SystemCallData->Argument[0]);}
    else{
        searched = current_PCB;
    }
    if(searched!=NULL&&searched->state!=TERMINATED){
    *(SystemCallData->Argument[1]) = searched->pid;
    *(SystemCallData->Argument[2]) = ERR_SUCCESS;
    }
    else{
        *(SystemCallData->Argument[1]) = -1;
        *(SystemCallData->Argument[2]) = ERR_ILLEGAL_ADDRESS;
    }
}

//read disk
void osPhysicalDiskRead(SYSTEM_CALL_DATA* SystemCallData){
    MEMORY_MAPPED_IO mmio;
    mmio.Mode = Z502Status;
    mmio.Field1 = SystemCallData->Argument[0];
    mmio.Field2 = mmio.Field3 = 0;
    MEM_READ(Z502Disk, &mmio);
    
//    mmio.Mode = Z502Action;
//    mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
//    MEM_WRITE(Z502Idle, &mmio);
    
    // The disk should now be done.  Make sure it's idle
    mmio.Mode = Z502Status;
    mmio.Field1 = SystemCallData->Argument[0];
    mmio.Field2 = mmio.Field3 = 0;
    MEM_READ(Z502Disk, &mmio);

    /* Now we read the data back from the disk.  If we're lucky,
     we'll read the same thing we wrote!                     */
    // Start the disk by reading a block of data
    mmio.Mode = Z502DiskRead;
    mmio.Field1 = SystemCallData->Argument[0]; // Pick same disk location
    mmio.Field2 = SystemCallData->Argument[1];
    mmio.Field3 = SystemCallData->Argument[2];
    
    // Do the hardware call to read data from disk
    MEM_WRITE(Z502Disk, &mmio);
    // Wait for the disk action to complete.
    // (Note - students should NEVER write OS code this way, but
    //     instead should use the Z502Idle hardware request
    mmio.Field2 = DEVICE_IN_USE;
    while (mmio.Field2 != DEVICE_FREE) {
        mmio.Mode = Z502Status;
        mmio.Field1 = SystemCallData->Argument[0];
        mmio.Field2 = mmio.Field3 = 0;
        MEM_READ(Z502Disk, &mmio);
    }


}
//write disk
void osPhysicalDiskWrite(SYSTEM_CALL_DATA* SystemCallData){
    MEMORY_MAPPED_IO mmio;
    mmio.Mode = Z502Status;
    mmio.Field1 = SystemCallData->Argument[0];
    mmio.Field2 = mmio.Field3 = 0;
    MEM_READ(Z502Disk, &mmio);
    // Start the disk by writing a block of data
    mmio.Mode = Z502DiskWrite;
    mmio.Field1 = SystemCallData->Argument[0];
    mmio.Field2 = SystemCallData->Argument[1];
    mmio.Field3 = SystemCallData->Argument[2];
    
   
    MEM_WRITE(Z502Disk, &mmio);

    
    mmio.Field2 = DEVICE_IN_USE;
    while (mmio.Field2 != DEVICE_FREE) {
        mmio.Mode = Z502Status;
        mmio.Field1 = SystemCallData->Argument[0];
        mmio.Field2 = mmio.Field3 = 0;
        MEM_READ(Z502Disk, &mmio);
    }
}

void osTerminateProcess(SYSTEM_CALL_DATA* SystemCallData){
    MEMORY_MAPPED_IO mmio;
    if((long)SystemCallData->Argument[0]==-1){
        current_PCB->state=TERMINATED;
        if(get_length_of_list(ready_queue)>0)
            dispatcher();
        else if (get_length_of_list(waiting_queue)>0)
            dispatcher();
        else{
            *(SystemCallData->Argument[1])=ERR_SUCCESS;
            mmio.Mode = Z502Action;
            mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
            MEM_WRITE(Z502Halt, &mmio);
        }
            
            
    }
    else if((long)SystemCallData->Argument[0]==-2){
        *(SystemCallData->Argument[1])=ERR_SUCCESS;
        mmio.Mode = Z502Action;
        mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
        MEM_WRITE(Z502Halt, &mmio);
    }
    
    else{
        if(ready_queue->data==NULL&&waiting_queue->data==NULL&&root_PCB->state==TERMINATED){
            
            mmio.Mode = Z502Action;
            mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
            MEM_WRITE(Z502Halt, &mmio);
        }
        int term_id = (int)SystemCallData->Argument[0];
        PCB* term_pcb = (PCB*)get_pcb_with_pid(all_process,term_id);
            term_pcb->state=TERMINATED;
    }
    
    *(SystemCallData->Argument[1])=ERR_SUCCESS;
    if(get_length_of_list(ready_queue)>0)
        dispatcher();

}
void svc(SYSTEM_CALL_DATA *SystemCallData) {
	short call_type;
	static short do_print = 10;
	short i;
    MEMORY_MAPPED_IO mmio;
    
	call_type = (short) SystemCallData->SystemCallNumber;
	if (do_print > 0) {
		printf("SVC handler: %s\n", call_names[call_type]);
		for (i = 0; i < SystemCallData->NumberOfArguments - 1; i++) {
			//Value = (long)*SystemCallData->Argument[i];
			printf("Arg %d: Contents = (Decimal) %8ld,  (Hex) %8lX\n", i,
					(unsigned long) SystemCallData->Argument[i],
					(unsigned long) SystemCallData->Argument[i]);
		}
		do_print--;
        
	}
    switch (call_type) {
            // Get time service call
        case SYSNUM_GET_TIME_OF_DAY: // This value is found in syscalls.h
            osGetTimeOfDay(SystemCallData);
            break;
            // terminate system call
        case SYSNUM_TERMINATE_PROCESS:
            osTerminateProcess(SystemCallData);
            break;
        case SYSNUM_SLEEP:
            osSleep(SystemCallData);
            break;
        case SYSNUM_GET_PROCESS_ID:
            osGetProcessId(SystemCallData);
            break;
            
        case SYSNUM_CREATE_PROCESS:
            osCreateProcess(SystemCallData);
            break;
        case SYSNUM_PHYSICAL_DISK_READ:
            osPhysicalDiskRead(SystemCallData);
            break;
        case SYSNUM_PHYSICAL_DISK_WRITE:
            osPhysicalDiskWrite(SystemCallData);
            break;
        default:
            printf( "ERROR! call_type not recognized!\n" );
            printf( "Call_type is - %i\n", call_type);
    } // End of switch
}                                               // End of svc

/************************************************************************
 osInit
 This is the first routine called after the simulation begins.  This
 is equivalent to boot code.  All the initial OS components can be
 defined and initialized here.
 ************************************************************************/



void osInit(int argc, char *argv[]) {
	void *PageTable = (void *) calloc(2, NUMBER_VIRTUAL_PAGES);
	INT32 i;
	MEMORY_MAPPED_IO mmio;
    waiting_queue = create_new_list();
    ready_queue = create_new_list();
    all_process = create_new_list();
//    disk_list = create_disk_list();
    //init systemcalldata2
    //systemcalldata2 is the paras to create a process
    SYSTEM_CALL_DATA *SystemCallData2;
    SystemCallData2= (SYSTEM_CALL_DATA*)malloc(sizeof(SYSTEM_CALL_DATA));
    
    // Demonstrates how calling arguments are passed thru to here

	printf("Program called with %d arguments:", argc);
	for (i = 0; i < argc; i++)
		printf(" %s", argv[i]);
	printf("\n");
	printf("Calling with argument 'sample' executes the sample program.\n");

	// Here we check if a second argument is present on the command line.
	// If so, run in multiprocessor mode
	if (argc > 2) {
		if ( strcmp( argv[2], "M") || strcmp( argv[2], "m")) {
		printf("Simulation is running as a MultProcessor\n\n");
		mmio.Mode = Z502SetProcessorNumber;
		mmio.Field1 = MAX_NUMBER_OF_PROCESSORS;
		mmio.Field2 = (long) 0;
		mmio.Field3 = (long) 0;
		mmio.Field4 = (long) 0;
		MEM_WRITE(Z502Processor, &mmio);   // Set the number of processors
		}
	} else {
		printf("Simulation is running as a UniProcessor\n");
		printf(
				"Add an 'M' to the command line to invoke multiprocessor operation.\n\n");
	}

	//          Setup so handlers will come to code in base.c

	TO_VECTOR[TO_VECTOR_INT_HANDLER_ADDR ] = (void *) InterruptHandler;
	TO_VECTOR[TO_VECTOR_FAULT_HANDLER_ADDR ] = (void *) FaultHandler;
	TO_VECTOR[TO_VECTOR_TRAP_HANDLER_ADDR ] = (void *) svc;

	//  Determine if the switch was set, and if so go to demo routine.

	PageTable = (void *) calloc(2, NUMBER_VIRTUAL_PAGES);
	if ((argc > 1) && (strcmp(argv[1], "sample") == 0)) {
		mmio.Mode = Z502InitializeContext;
		mmio.Field1 = 0;
		mmio.Field2 = (long) SampleCode;
		mmio.Field3 = (long) PageTable;

		MEM_WRITE(Z502Context, &mmio);
        
        
        
//        osCreateProcess();
        // Start of Make Context Sequence
		mmio.Mode = Z502StartContext;
		// Field1 contains the value of the context returned in the last call
		mmio.Field2 = START_NEW_CONTEXT_AND_SUSPEND;
		MEM_WRITE(Z502Context, &mmio);     // Start up the context

	} // End of handler for sample code - This routine should never return here
    else if(argc>1 &&(strcmp(argv[1], "test1") == 0)){
        SystemCallData2->NumberOfArguments=6;
        SystemCallData2->Argument[0] = "test1";
        SystemCallData2->Argument[1] = (long)test1;
        SystemCallData2->Argument[2] = (long)ROOT_PCB_PRIO;
        osCreateProcess(SystemCallData2);
    }
    else if(argc>1 &&(strcmp(argv[1], "test2") == 0)){
        SystemCallData2->NumberOfArguments=6;
        SystemCallData2->Argument[0] = "test2";
        SystemCallData2->Argument[1] = (long)test2;
        SystemCallData2->Argument[2] = (long)ROOT_PCB_PRIO;
        osCreateProcess(SystemCallData2);
    
    }
    else if(argc>1 &&(strcmp(argv[1], "test3") == 0)){
        SystemCallData2->NumberOfArguments=6;
        SystemCallData2->Argument[0] = "test3";
        SystemCallData2->Argument[1] = (long)test3;
        SystemCallData2->Argument[2] = (long)ROOT_PCB_PRIO;
        osCreateProcess(SystemCallData2);
    
    }
    else if(argc>1 &&(strcmp(argv[1], "test4") == 0)){
        SystemCallData2->NumberOfArguments=6;
        SystemCallData2->Argument[0] = "test4";
        SystemCallData2->Argument[1] = (long)test4;
        SystemCallData2->Argument[2] = (long)ROOT_PCB_PRIO;
        osCreateProcess(SystemCallData2);
    
    }
    else if(argc>1 &&(strcmp(argv[1], "test5") == 0)){
        SystemCallData2->NumberOfArguments=6;
        SystemCallData2->Argument[0] = "test5";
        SystemCallData2->Argument[1] = (long)test5;
        SystemCallData2->Argument[2] = (long)ROOT_PCB_PRIO;
        osCreateProcess(SystemCallData2);
        
    }
    else if(argc>1 &&(strcmp(argv[1], "test6") == 0)){
        SystemCallData2->NumberOfArguments=6;
        SystemCallData2->Argument[0] = "test6";
        SystemCallData2->Argument[1] = (long)test6;
        SystemCallData2->Argument[2] = (long)ROOT_PCB_PRIO;
        osCreateProcess(SystemCallData2);
        
    }
    else if(argc>1 &&(strcmp(argv[1], "test7") == 0)){
        SystemCallData2->NumberOfArguments=6;
        SystemCallData2->Argument[0] = "test7";
        SystemCallData2->Argument[1] = (long)test7;
        SystemCallData2->Argument[2] = (long)ROOT_PCB_PRIO;
        osCreateProcess(SystemCallData2);
        
    }
    else if(argc>1 &&(strcmp(argv[1], "test8") == 0)){
        SystemCallData2->NumberOfArguments=6;
        SystemCallData2->Argument[0] = "test8";
        SystemCallData2->Argument[1] = (long)test8;
        SystemCallData2->Argument[2] = (long)ROOT_PCB_PRIO;
        osCreateProcess(SystemCallData2);
        
    }
    else if(argc>1 &&(strcmp(argv[1], "test9") == 0)){
        SystemCallData2->NumberOfArguments=6;
        SystemCallData2->Argument[0] = "test9";
        SystemCallData2->Argument[1] = (long)test9;
        SystemCallData2->Argument[2] = (long)ROOT_PCB_PRIO;
        osCreateProcess(SystemCallData2);
        
    }

	//  By default test0 runs if no arguments are given on the command line
	//  Creation and Switching of contexts should be done in a separate routine.
	//  This should be done by a "OsMakeProcess" routine, so that
	//  test0 runs on a process recognized by the operating system.

	
	mmio.Mode = Z502StartContext;
	// Field1 contains the value of the context returned in the last call
	// Suspends this current thread
	mmio.Field2 = START_NEW_CONTEXT_AND_SUSPEND;
	MEM_WRITE(Z502Context, &mmio);     // Start up the context

}                                               // End of osInit
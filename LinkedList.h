//
//  LinkedList.h
//  OSproject
//
//  Created by fan yang on 10/18/16.
//  Copyright © 2016 fan yang. All rights reserved.
//

#ifndef LinkedList_h
#define LinkedList_h

#include <stdio.h>
#include "studentConfiguration.h"

typedef struct Node Node;
typedef struct Node{
    PCB* data;
    struct Node* next;
    
    
}*Linkedlist;


//typedef struct{
//    myDISK* data;
//    struct disknode* next;
//
//} disknode,*Disklist;

void *create_new_list();
void *create_disk_list();

int add_to_list(Linkedlist L,PCB* new_PCB);
//int add_to_disklist(Disklist DL,myDISK*new_disk);
int remove_from_head(Linkedlist L);
//int remove_from_disk_list(Disklist DL,int diskid,int sectorid);
//int get_disk_list_length(Disklist DL);
PCB* get_front_PCB(Linkedlist L);

int get_length_of_list(Linkedlist L);

PCB* get_pcb_with_pid(Linkedlist L,INT32 pid);


PCB* get_pcb_with_name(Linkedlist L,char* name);


#endif /* LinkedList_h */


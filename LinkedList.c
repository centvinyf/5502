//
//  LinkedList.c
//  OSproject
//
//  Created by fan yang on 10/18/16.
//  Copyright © 2016 fan yang. All rights reserved.
//

#include "LinkedList.h"

//create a new linkedlist
void *create_new_list()
{
    Linkedlist list = (Linkedlist)calloc(1, sizeof(Node));
    //in case of any unknown exception to fail to create list
    if(list == NULL){
        printf("Fail to create new list!\n");
        return NULL;
    }
    //initialize the list
    list->data = NULL;
    list->next = NULL;
    return list;


}
////create a new disklist
//void *create_disk_list()
//{
//    Disklist l = (Disklist)calloc(1, sizeof(disknode));
//    if(l == NULL){
//        printf("Fail to create new list!\n");
//        return NULL;
//    }
//    l->data = NULL;
//    l->next = NULL;
//    return l;
//}
//
//add a pcb node to existing linkedlist
int add_to_list(Linkedlist L,PCB* new_PCB){
    // if the list does not exists, return false
    if(L==NULL) return 0;
    
    //if the list is empty, add the node to the front
    if(L->data == NULL){
        L->data = new_PCB;
        L->next = NULL;
        
        return 1;
    }
    
    //if the delay of new pcb is less than current least one,then put it at front of the list
    if(L->data->delay > new_PCB->delay){
        Node* new_node = (Node*)calloc(1, sizeof(Node));
        new_node->next = L->next;
        L->next = new_node;
        new_node->data = L->data;
        L->data = new_PCB;
        
        return 1;
    }
    
    //Otherwise put it into the queue according to delay time sequence
    Node* prev=NULL;
    Node* cur = L;
    while (cur !=NULL &&cur->data->delay<=new_PCB->delay)
    {
        prev = cur;
        cur = cur->next;
    }
    
    Node* new_node = (Node*)calloc(1, sizeof(Node));
    prev->next = new_node;
    new_node->next = cur;
    new_node->data = new_PCB;
    
    return 1;
    
}
////put a disk into disklisk
//int add_to_disklist(Disklist DL,myDISK*new_disk){
//    if(DL==NULL) return 0;
//    if (DL->data==NULL)
//    {
//        DL->data = new_disk;
//        DL->next= NULL;
//        return 1;
//    }
//    else{
//        disknode * dn = DL;
//        while(dn->next!=NULL){
//            dn = dn->next;
//        }
//        disknode* new_node = (disknode*)malloc(sizeof(disknode));
//        new_node->data = new_disk;
//        new_node->next = NULL;
//        dn->next = new_node;
//        return 1;
//    }
//    return 0;
//
//}
//remove disk from disk node,if success we have 1,else we have 0
//int remove_from_disk_list(Disklist DL,int diskid,int sectorid)
//{
//    if(DL==NULL||DL->data==NULL) return 0;
//    disknode* prev = NULL;
//    disknode* cur = DL;
//    while(cur!=NULL){
//        if(cur->data->diskid==diskid&&cur->data->sectorid==sectorid)
//        {
//            disknode* tem = cur->next;
//            if(tem==NULL)
//            {
//                cur->data =NULL;
//                cur->next = NULL;
//                return 1;
//            }
//            else
//            {
//                cur->data = tem->data;
//                cur->next = tem->next;
//                return 1;
//            }
//        }
//        else{
//            prev = cur;
//            cur=cur->next;
//        }
//    }
//    return 0;
//
//}
//get length of disk list
//int get_disk_list_length(Disklist DL){
//    int len = 0;
//    disknode* tem= DL;
//    while(tem!=NULL&&tem->data!=NULL){
//        len++;
//        tem=tem->next;
//    }
//    return len;
//
//}

//remove from head of a linkedlist
int remove_from_head(Linkedlist L){
    //if the list is empty
    if(L==NULL) return 0;
    
    //remove the first element
    Node* tem = L->next;
    //if there is only one element
    if(tem == NULL){
        L->data = NULL;
        L->next = NULL;
        
        return 1;
    }
    //Otherwise, remove the first node
    L->next = tem->next;
    L->data = tem->data;
    
    return 1;

}

//get front element from a list
PCB* get_front_PCB(Linkedlist L){
    return L->data;
}

//get length of a list
int get_length_of_list(Linkedlist L){
    int len = 0;
    Node* tem= L;
    while(tem!=NULL&&tem->data!=NULL){
        len++;
        tem=tem->next;
    }
    return len;
}
//return if any pcb has a given name
PCB* get_pcb_with_name(Linkedlist L,char* name){
    Node* cur = L;
    if(L==NULL||L->data==NULL) return NULL;
    while(cur!=NULL&&cur->data!=NULL){
        if(strcmp(cur->data->name,name )==0)
            return cur->data;
        else{
            cur = cur->next;
        }
    }
    return NULL;
}

//return if any pcb has a given id
PCB* get_pcb_with_pid(Linkedlist L,INT32 pid){
    Node* cur = L;
    if(L==NULL||L->data==NULL) return NULL;
    while(cur!=NULL&&cur->data!=NULL){
        if(cur->data->pid==pid){
            return (PCB*)cur->data;
        
        }
        else{
            cur = cur->next;
        }
    }
    return NULL;
}

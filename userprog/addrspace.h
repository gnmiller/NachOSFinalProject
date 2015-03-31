// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "bitmap.h"
#include "fd_list.h"
#include "noff.h"

#define SHM_CREATE 0
#define SHM_USE 1
#define UserStackSize		1024 	// increase this as necessary!
void* attachSharedMemory(int key);
int allocateSharedMemory(int key, int numbytes, int flag);

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"

    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch 
	
	FD_List open_files;			// store open file info
	
	///////added by Can Li///////////////////////
	AddrSpace(char* name);//create address space by its file name
	//Make a copy of the address space (for fork).
    AddrSpace(const AddrSpace& source);
	int createStackArgs(int argv_addr, char* name); //create stack space for this process, return stack_base
	
	char* read(int addr, int bytes);//read at virtual address
	void write(int addr, char* str, int bytes);//write str to virtual address
	
	int load_page(int virt_page);//load virt_page into memory
	int allow_writes(int virt_page);
	
	int readPage( int physicalPage, int virtualPage );//read from physicalPage
    int writePage( int physicalPage, int virtualPage );//write virtualPage to that physicalPage

    int addMoreSpace(int additionalPages);
    int attachShMem(int key, int start);
    void truncatePagesFrom(int page);
    
    int store_page();
    int try_store_page();
	/////////////////////////////////////////////

  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual 
					// address space
	///////added by Can Li///////////////////////				
    char* filename;
    NoffHeader noffH;
    BitMap* on_disk;
    int* page_sector;
    int victum_offset;
    int stack_base;
    int argc;
    int argv;
    bool forked;
    /////////////////////////////////////////////
					
};

#endif // ADDRSPACE_H

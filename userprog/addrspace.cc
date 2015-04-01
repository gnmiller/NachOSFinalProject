// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "interrupt.h"
#include "synch.h"
#include <map>
#include <vector>

#ifdef HOST_SPARC
#include <strings.h>
#endif

// *** FD_List Class defs *** //
/**
 * default constructor
 */
FD_List::FD_List( int i_size ) : map( i_size ), list( 0 ), lock( 0 ), size( i_size )
{
	list = new void*[ size ];
	lock = new Lock("FD_List Lock");
}

FD_List::FD_List() : map( 512 ), list( 0 ), lock( 0 ), size( 512 )
{
	list = new void*[ size ];
	lock = new Lock("FD_List Lock");
}

/**
 * destructor
 */
FD_List::~FD_List()
{
	if( list )
	{
		delete list;
	}
	if( lock )
	{
		delete lock;
	}
}

/**
 * Return a FD (if it exists) from the current FD list
 */
void* FD_List::fd_get( int i )
{
	/* test if the fd is in range and in the map */
	if( i < 0 || i >= size ) return 0;
	if( map.Test( i ) )
		return list[ i ];
}

/**
 * Place an FD into the list (checks if the FD already exists in map)
 */
int FD_List::fd_put( void *new_fd )
{
	int i; // store the next index for FDs
	
	/* check the map for next index... */
	lock->Acquire();
	i = map.Find();
	lock->Release();
	
	if( i != -1 )
		list[ i ] = new_fd;
	return i;
}

/**
 * Remove a FD from the list
 */
void* FD_List::fd_remove( int i )
{
	void *old_fd; // store the fd to remove for return
	
	if( i >= 0 && i < size )
	{
		lock->Acquire();
		if( map.Test( i ) )
		{
			map.Clear( i );
			old_fd = list[ i ];
			list[ i ] = 0;
		}
		lock->Release();
	}
	return old_fd;
}
// *** End FD_List *** //

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
    //NumPhysPages defined in machine.h ( = 32)
	//printf("numPages: %d, NumPhysPages: %d\n", numPages, NumPhysPages);
    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].physicalPage = i;
	pageTable[i].valid = TRUE;
	pageTable[i].use = FALSE;
	pageTable[i].dirty = FALSE;
	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

///////////added by Can Li///////////////////////
static int available_pages[NumPhysPages] = {0};
static BitMap available_sectors(NumSectors);

static std::vector<int> sharedMemIds;
static std::map<int, std::vector<int> > sharedMemory;
static std::map<int, int> attachedThreads;
#ifdef FILESYS_NEEDED
#include "synchdisk.h"
#endif
bool isSharedPage(int page)
{
    std::map<int, std::vector<int> >::iterator it;
    for(it = sharedMemory.begin(); it != sharedMemory.end(); it++)
    {
        std::vector<int>::iterator it2;
        for(it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            int pageAdd = *it2;
            if(page == pageAdd){
                return true;
            }
        } 
    } 

    return false;
}


AddrSpace::AddrSpace(char* name)
{
	OpenFile* executable = fileSystem->Open(name);
    if(executable == NULL)
    {		
    	printf("Unable to open file %s\n", name);
		return;
	}
    int size = 0;
	
    argc = 0;
    argv = 0;
    victum_offset = 0;
    forked = FALSE;

	// initialize the fd list to include stdin and stdout
	open_files.fd_put( (void*)0 );
	open_files.fd_put( (void*)0 );
	
    //Save this so the address space can be constructed later.
    
    filename = new char[strlen(name)];
    
    strcpy(filename, name); //copy including null terminator
	
    //Read the executable header.
    //printf("name: %s\n", filename);
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    
    delete executable;

    if ((noffH.noffMagic != NOFFMAGIC) &&
            (WordToHost(noffH.noffMagic) == NOFFMAGIC)) {
        SwapHeader(&noffH);
    }
    if(noffH.noffMagic != NOFFMAGIC) {
        DEBUG('u', "Attemted to execute non-noff executable\"%s\".",
              executable);
        throw 1;
    }

    // how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
           + UserStackSize;	// we need to increase the size
                                // to leave room for the stack

    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
    stack_base = size - 16;

    //At first, nothing is on disk.
    on_disk = new BitMap(numPages);

    //Nothing is initally on disk.
    page_sector = new int[numPages];
    for(int i = 0; i < numPages; i++) {
        page_sector[i] = -1;
    }
    

    DEBUG('u', "Initializing address space, num pages %d, size 0x%x\n",
        numPages, size);
    
    //An empty page table.
    //Only load pages on demand.
    pageTable = new TranslationEntry[numPages];
    for (int virt_page = 0; virt_page < numPages; virt_page++) {
        pageTable[virt_page].virtualPage = virt_page;
        pageTable[virt_page].physicalPage = -1;

        pageTable[virt_page].valid = FALSE;
        pageTable[virt_page].use = FALSE;
        pageTable[virt_page].dirty = FALSE;
        pageTable[virt_page].readOnly = FALSE;
    }
}

AddrSpace::AddrSpace(const AddrSpace& source)
{
	forked = TRUE;
    filename = source.filename;
    noffH = source.noffH;
    numPages = source.numPages;
    victum_offset = source.victum_offset;
    stack_base = source.stack_base;
    argc = source.argc;
    argv = source.argv;

    on_disk = new BitMap(numPages);
    pageTable = new TranslationEntry[numPages];
    page_sector = new int[numPages];

    for(int i = 0; i < numPages; i++) {
        pageTable[i] = source.pageTable[i];
        page_sector[i] = -1;
    }
}

int AddrSpace::createStackArgs(int argv_addr, char* name)
{
	AddrSpace* currentSpace = currentThread->space;
    int curr_argv[20] = {0};
    int stack_argv[19];
    int arg_ptr_str;

    DEBUG('u', "Stack starts at address 0x%x\n", stack_base);

    stack_base -= strlen(name) + 1;
    write(stack_base, name, 0);
    stack_argv[0] = stack_base;
    argc = 1;

    DEBUG('u', "Pushed filename onto stack at  0x%x\n",
        stack_base);

    //Read the pointers into argv
    for(int arg_num = 0; arg_num < 19; arg_num++, argc++) {
        int* arg_ptr = (int*)currentSpace->read(argv_addr + arg_num * 4, 4);

        curr_argv[arg_num] = *(arg_ptr);

        if(curr_argv[arg_num] == 0) {
            break;
        }
    }
    curr_argv[argc] = 0;

    //Copy argv vector from one address space to the other.
    for(int arg = 0; arg < argc - 1; arg++) {
        DEBUG('u', "Reading argv string from 0x%x\n", curr_argv[arg]);
        char* arg_str = currentSpace->read(curr_argv[arg], 0);

        DEBUG('u', "Read string '%s'\n", arg_str);

        stack_base -= strlen(arg_str) + 1;
        stack_argv[arg + 1] = stack_base;

        DEBUG('u', "String pushed onto stack at 0x%x\n", stack_base);

        write(stack_base, arg_str, 0);
    }

    //Argument strings have been pushed onto the stack. Now push the argv
    //array.
    stack_base -= argc * 4 + 4; //Make space for argc pointers in argv.
    stack_base &= ~0x3;         //Align the addresses.
    for(int arg_num = 0; arg_num < argc; arg_num++) {
        arg_ptr_str = stack_argv[arg_num];
        DEBUG('u', "Push pointer 0x%x onto stack at 0x%x\n",
            arg_ptr_str, stack_base + arg_num * 4);
        write(stack_base + arg_num * 4, (char*)&arg_ptr_str, 4);
    }

    //Null terminate the array.
    arg_ptr_str = 0;
    write(stack_base + argc * 4, (char*)&arg_ptr_str, 4);

    argv = stack_base;
    stack_base -= 8;

	return stack_base;
}

char* AddrSpace::read(int addr, int bytes)
{
    static char buf[1000];

    for(int i = 0; i < 1000 && (bytes == 0 || i < bytes); i++) 
    {
        int virt_page = (addr + i) / PageSize;
        TranslationEntry* page = &(pageTable[virt_page]);
        if(!page->valid || page->physicalPage == -1) 
        {
            load_page(virt_page);
        }

        buf[i] = machine->mainMemory[page->physicalPage * PageSize + (addr + i) % PageSize];

        if(bytes == 0 && buf[i] == '\0') 
        {
            break;
        }
    }

    return buf;
}

void AddrSpace::write(int addr, char* str, int bytes)
{
    for(int i = 0; i < 1000 && (bytes == 0 || i < bytes); i++) {
        int virt_page = (addr + i) / PageSize;
        TranslationEntry* page = &(pageTable[virt_page]);

        //Check if page is valid.
        //If physical page == -1 has not yet been set.
        if(!page->valid || page->physicalPage == -1) {
            load_page(virt_page);
        }

        machine->mainMemory[page->physicalPage * PageSize + 
            (addr + i) % PageSize] = str[i];

        if(bytes == 0 && str[i] == '\0') {
            break;
        }
    }	
	return;
}

int AddrSpace::readPage(int physicalPage, int virtualPage) {
    int sector = page_sector[virtualPage];
    if(sector < 0 || sector >= NumSectors) {
        return -1;
    }
    synchDisk->ReadSector(sector, &(machine->mainMemory[physicalPage * PageSize]));
    return sector;
}

int AddrSpace::writePage( int physicalPage, int virtualPage )
{
    int sector = page_sector[virtualPage];
    if(sector < 0 || sector >= NumSectors) {
        sector = available_sectors.Find();
        ASSERT(sector >= 0);
        available_sectors.Mark(sector);
        DEBUG('u', "Writing virtual page %d to sector %d:",
            virtualPage, sector);
        page_sector[virtualPage] = sector;
    }
    synchDisk->WriteSector(sector, &(machine->mainMemory[physicalPage * PageSize]));
    return sector;
}

//Load page number "virt_page" into memory. Select a victum page
//to swap out if necessary.
//Returns the frame number, or -1 on error. Process should die on error.
int AddrSpace::load_page(int virt_page) {
    TranslationEntry* page = &(pageTable[virt_page]);
    OpenFile* executable = NULL;

    DEBUG('u', "Load page 0x%x for thread %p\n", virt_page, this);

    if(virt_page >= numPages || virt_page < 0) {
        DEBUG('u', "Segmentation fault in load_page on page no:%d\n",
            virt_page);
        return -1;
    }

    if(page->virtualPage != virt_page) {
        DEBUG('u', "Error index into page table on virtual page %d\n",
            virt_page);
        return -1;
    }

    //If the page is already in memory, nothing needs to be done.
    if(page->valid == TRUE) {
        DEBUG('u', "Fault on virtual page %d, but it is already in frame %d.\n",
            virt_page, page->physicalPage);
        return page->physicalPage;
    }

    //If memory space is available, put the page in memory.
    DEBUG('s',"checking for space available\n");
    for(int i = 0; i < NumPhysPages; i++) {
        if(isSharedPage(i)){
            DEBUG('s',"is shared page, so dont add this\n");
            continue;
        }
        if(available_pages[i] == 0) {
            DEBUG('u', "Found free page %d\n", i);
            page->physicalPage = i;
            available_pages[i] = 1;
            break;
        }
    }

    //If no extra space exists in memory, push one of this processes current
    //pages to disk. If impossible, yeild and try again later.
    if(page->physicalPage == -1) {
        page->physicalPage = store_page();
        available_pages[page->physicalPage] = 1;
        page->use = TRUE;
    }

    if(on_disk->Test(virt_page)) {
        DEBUG('u', "Load page from disk into memory.\n");

        readPage(page->physicalPage, page->virtualPage);
        return -1;
    } else {
        //Zero pages that aren't coming from disk. This includes text and data
        //pages (before the data is generated). The page might contain stack
        //space too.
        bzero(&(machine->mainMemory[page->physicalPage * PageSize]), PageSize);
    }


    //if the page with this virtual address contains text, load it from the
    //executable.
    if(noffH.code.size > 0 && 
            virt_page >= noffH.code.virtualAddr / PageSize &&
            virt_page <= (noffH.code.virtualAddr + noffH.code.size) / PageSize
            ) {
        int start = page->physicalPage * PageSize;
        int end = -1;
        int file_offset = noffH.code.inFileAddr;

        DEBUG('u', "Generating text section from executable '%s'\n", filename);

        executable = fileSystem->Open(filename);
        if(executable == NULL) {
            DEBUG('u', "Unable to open file '%s'.\n", filename);
            return -1;
        }

        //If the text begins in this page, use the header to decide where to
        //set the offset. Otherwise start at the beginning of the page.
        if(virt_page == noffH.code.virtualAddr / PageSize) {
            start += noffH.code.virtualAddr % PageSize;
        } else {
            //Text doesn't start in this page, so set the offset.
            file_offset += virt_page * PageSize - noffH.code.virtualAddr;
        }

        //Only read into this page (not the one after).
        end = start + noffH.code.size;
        if(end / PageSize != page->physicalPage) {
            end = (page->physicalPage + 1) * PageSize;
        }

        DEBUG('u', "Initializing code beginning from VA 0x%x to 0x%x.\n",
            page->virtualPage * PageSize, page->virtualPage * PageSize + 
            end - start);
        DEBUG('u', "Main memory addresses from 0x%x to 0x%x\n",
            start, end);
        DEBUG('u', "File offset:0x%x\n", file_offset);
        executable->ReadAt(&(machine->mainMemory[start]), end - start,
            file_offset);
    } else {
        DEBUG('u', "Section contains no code.\n");
    }

    //Same as text, but with initalized data (literally copy & paste).
    if(noffH.initData.size > 0 && 
            virt_page >= noffH.initData.virtualAddr / PageSize &&
            virt_page <= (noffH.initData.virtualAddr + noffH.initData.size) / 
            PageSize) {
        int start = page->physicalPage * PageSize;
        int end = -1;
        int file_offset = noffH.initData.inFileAddr;

        DEBUG('u', "Generating data section from executable '%s'\n", filename);

        executable = fileSystem->Open(filename);
        if(executable == NULL) {
            DEBUG('u', "Unable to open file '%s'.\n", filename);
            return -1;
        }

        //If the text begins in this page, use the header to decide where to
        //set the offset. Otherwise start at the beginning of the page.
        if(virt_page == noffH.initData.virtualAddr / PageSize) {
            start += noffH.initData.virtualAddr % PageSize;
        } else {
            //Text doesn't start in this page, so set the offset.
            file_offset += virt_page * PageSize - noffH.initData.virtualAddr;
        }

        //Only read into this page (not the one after).
        end = start + noffH.initData.size - file_offset;
        if(end / PageSize != page->physicalPage) {
            end = (page->physicalPage + 1) * PageSize;
        }

        DEBUG('u', "Initializing data segment, from 0x%x to 0x%x\n",
            start, end);
        DEBUG('u', "File offset:0x%x\n", file_offset);
        executable->ReadAt(&(machine->mainMemory[start]), end - start,
            file_offset);
    } else {
        DEBUG('u', "Section contains no initData.\n");
    }

    page->valid = TRUE;

    delete executable;
    return page->physicalPage;
}

//Make a copy of a page, and unset the readOnly bit on the TranslationEntry
//Return the physical page number of the new page, or -1 on error.
int AddrSpace::allow_writes(int virt_page) {
    TranslationEntry* page = &(pageTable[virt_page]);
    int new_physical_page = -1;

    if(virt_page >= numPages || virt_page < 0) {
        DEBUG('u', "Segmentation fault in allow_writes on page no:%d\n",
            virt_page);
        return -1;
    }

    if(page->virtualPage == virt_page) {
        DEBUG('u', "Error index into page table on virtual page %d\n",
            virt_page);
        return -1;
    }

    //If the page isn't read only, there is nothing to do.
    if(!page->readOnly) {
        return page->physicalPage;
    }

    //The page isn't shared (anymore), so remove the read-only flag.
    if(available_pages[page->physicalPage] == 1) {
        page->readOnly = FALSE;
        return page->physicalPage;
    }

    //Search for a place to put a copy of the page.
    for(int i = 0; i < NumPhysPages; i++) {
        if(available_pages[i] == 0) {
            new_physical_page = i;
            available_pages[i] = 1;
        }
    }

    //If no extra memory exists, try to store something. If impossible, yeild
    //and try again later.
    new_physical_page = store_page();

    //Set the page to exclusively used.
    available_pages[new_physical_page] = 1;

    //Copy the old page to the new one.
    memcpy(&(machine->mainMemory[new_physical_page * PageSize]),
        &(machine->mainMemory[page->physicalPage * PageSize]),
        PageSize);

    available_pages[page->physicalPage]--;
    page->physicalPage = new_physical_page;
    page->readOnly = FALSE;

    return page->physicalPage;
}

int AddrSpace::addMoreSpace(int additionalPages)
{
    TranslationEntry *biggerPageTable = new TranslationEntry[numPages + additionalPages];
    
    for(int i = 0; i < numPages; i++){
        biggerPageTable[i] = pageTable[i];
    }

    pageTable = biggerPageTable;
    //TODO possibly delete the old pagetable here

    for (int i = numPages; i < numPages + additionalPages; i++) {
        pageTable[i].virtualPage = i;
        pageTable[i].physicalPage = -1;

        pageTable[i].valid = FALSE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;
    }
    
    numPages += additionalPages;

    return numPages - additionalPages;
}

int AddrSpace::attachShMem(int key, int start)
{
    std::vector<int>::iterator it;
    for(it = sharedMemory[key].begin(); it != sharedMemory[key].end(); it++)
    {
        pageTable[start].physicalPage = *it;
        pageTable[start].valid = TRUE;
        pageTable[start].readOnly = FALSE;
        available_pages[*it]++;
        start++;
    }
    
    
    return pageTable[start].virtualPage * PageSize;
}

void AddrSpace::truncatePagesFrom(int page)
{
    TranslationEntry *table = new TranslationEntry[page];
    for(int i = 0; i < page; i++)
    {
        table[i] = pageTable[i];
    }

    //TODO maybe delete bigger table
    numPages = page;
    pageTable = table;
}

int AddrSpace::try_store_page() {
    TranslationEntry* best_page = &(pageTable[victum_offset]);
    bool best_used = FALSE;
    int unused_offset = -1;
    int rtn = -1;

    DEBUG('u', "Swaping victum page to disk.\n");
    
    //Select the best page to replace.
    for(int i = 1; i < numPages; i++) {
        TranslationEntry* curr_page = 
            &(pageTable[(victum_offset + i) % numPages]);
        bool curr_used = curr_page->use;

        if(!curr_page->valid) {
            continue;
        }

        //Unset the used bits until an unused page is found.
        //Must be done before more important conditions skip the checking of
        //the use bit.
        if(unused_offset < 0) {
            if(curr_page->use) {
                curr_page->use = FALSE;
            } else {
                unused_offset = (victum_offset + i) % numPages;
            }
        }
        
        if(isSharedPage(best_page->physicalPage)){
            continue;
        }

        //If the current best selection uses non-read-only shared memory,
        //anything is better.
        if(!best_page->readOnly && 
                available_pages[best_page->physicalPage] > 1) {
            best_page = curr_page;
            best_used = curr_used;
            continue;
        }


        //If the current page is read_only memory
        if(best_page->readOnly && !curr_page->readOnly) {
            best_page = curr_page;
            best_used = curr_used;
            continue;
        }

        //Try to not swap out dirty pages. Swapping out non-dirty pages is
        //cheap, since they don't actually have to be written to disk.
        if(best_page->dirty && !curr_page->dirty) {
            best_page = curr_page;
            best_used = curr_used;
            continue;
        }

        //If all of the more important requirements are met, try to use an
        //unused page. Since this is the optimal page to swap out, stop
        //looking.
        if(best_used && !curr_used) {
            best_page = curr_page;
            best_used = curr_used;
            break;
        }
    }

    //This means all pages in the current address space are shared memory. This
    //means somehow the process set its own text section to shared. 
    if(!best_page->readOnly &&
            available_pages[best_page->physicalPage] > 1) {
        DEBUG('u', "Only shared memory found in current space.\n");
        return -1;
    }

    //Try to make it so the page we are swaping in isn't the next one that
    //is attempted to be swapped out.
    victum_offset = unused_offset;

    if(best_page->dirty || best_page->readOnly) {
        while(writePage(best_page->physicalPage, best_page->virtualPage) < 0) {
            currentThread->Yield();
        }
        best_page->valid = FALSE;
        best_page->physicalPage = -1;

        if(best_page->readOnly) {
            available_pages[best_page->physicalPage]--;
            best_page->readOnly = FALSE;
            return -1;
        }
    }
    
    DEBUG('u', "Swap out virt page %d, phys page %d\n",
        best_page->virtualPage, best_page->physicalPage);
    rtn = best_page->physicalPage;
    available_pages[rtn] = 0;
    best_page->physicalPage = -1;

    return rtn;
}

int AddrSpace::store_page() {
    int rtn;
    do {
        rtn = try_store_page();
    } while(rtn == -1);

    return rtn;
}

/////////////////////////////////////////////////

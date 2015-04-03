#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H
#include "copyright.h"
#include "synch.h"
#include "console.h"
#include "machine.h"
#include "system.h"

class SynchConsole{
	public:
		SynchConsole(char * inputFile = NULL, char * outputFile = NULL);
		~SynchConsole();

		char GetChar();
		void PutChar(char c);
		void ReadAvail();
		void WriteDone();

	private:
		Console *console;
		Lock *lock;
		Semaphore * SynchReadAvail;
		Semaphore * SynchWriteDone;
};
#endif

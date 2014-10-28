// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#include <string.h>

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!

//Initializing a lock which will be used for synchronization, debugName is used
//for debugging.
Lock::Lock(char* debugName) 
{
	int length = strlen(debugName);
	name = new char[length+1];
	strcpy(name, debugName);

	//Initializing lock values
	queue = new List();   //queue for threads
	holder = NULL; //curent lock holder
	old = false;   //past, used to see if thread has already held lock
	
}

//Command used to deallocate lock.
Lock::~Lock()
{
	//Turn interrupt off
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
		ASSERT( old == false);  //if nothing is holding the lock
		//else delallocate
		delete queue;
		delete name;
	(void) interrupt->SetLevel(oldLevel); //turn interrupt on
	
}

//Checks to see if the lock is taken or not, if not then it acquires it.
void Lock::Acquire()
{
	//interrupt off
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	if( old )
	{
		//Check if new thread has already had the lock
		ASSERT( holder != currentThread ) 
		queue->Append(currentThread);
		currentThread->Sleep();
	}

	//Check to see if thread exists
	ASSERT( holder == NULL );
	old = true;
	holder = currentThread;

	//interrupt on
	(void) interrupt->SetLevel(oldLevel);
}

//Allows the thread to release its own lock.
void Lock::Release()
{
	//Interrupt off
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	Thread *thread;
	ASSERT( old || holder ); //Make sure the thread has the lock

	//Check if queue is empty, shouldn't be when trying to remove a thread
	if( queue->IsEmpty() ) {}
	else
	{
		thread = (Thread*)queue->Remove();
		scheduler->ReadyToRun(thread);
	}

	//update flags
	holder = NULL;
	old = false;
	
	//INterrupt on
	(void) interrupt->SetLevel(oldLevel);
}

Condition::Condition(char* debugName) { }
Condition::~Condition() { }
void Condition::Wait(Lock* conditionLock) {
    ASSERT(FALSE);
}
void Condition::Signal(Lock* conditionLock) { }
void Condition::Broadcast(Lock* conditionLock) { }



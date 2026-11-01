#include "project3.h"
#include "multitasking.h"
#include "processes.h"

// An array to hold all of the processes we create
proc_t processes[MAX_PROCS];

// Keep track of the next index to place a newly created process in the process array
uint8 process_index = 0;

proc_t *prevprocess = 0;       // The previously ran user process
proc_t *runningprocess;    // The currently running process, can be either kernel or user process
proc_t *nextprocess;       // The next process to run
proc_t *kernelprocess;     // The kernel process

#if PROJECT == 3

void proca()
{
    putchar('A');
    exit();
}
void procb()
{
    putchar('B');
    yield();
    putchar('B');
    exit();
}

void procc()
{
    putchar('C');
    yield();
    putchar('C');
    yield();
    putchar('C');
    yield();
    putchar('C');
    exit();
}

void procd()
{
    putchar('D');
    yield();
    putchar('D');
    yield();
    putchar('D');
    exit();
}

void proce()
{
    putchar('E');
    yield();
    putchar('E');
    exit();
}

void prockernel()
{
    print("Kernel process has started...\n");

	// Create the user processes
	createuserprocess(proca, (void *) 0x10000);
    createuserprocess(procb, (void *)0x11000);
    createuserprocess(procc, (void *)0x12000);
    createuserprocess(procd, (void *)0x13000);
    createuserprocess(proce, (void *)0x14000);

	// Schedule the next process
	int readyUsers = ready_process_count();

	// As long as we have ready user processes to run
	while (readyUsers > 0)
{
		// Yield to them
		yield();
		readyUsers = ready_process_count();
	}

    print("\nKernel process has exited...\n");
    exit();
}

int kernel()
{
    startkernel(prockernel);
    return 0;
}

#endif

// Select the next user process (proc_t *next) to run
// Selection must be made from the processes array (proc_t processes[])
int schedule()
{
    if (process_index == 0)
    {
        return 0;
    }

    int startIndex = 0;

    if (prevprocess != 0)
    {
        startIndex = (prevprocess->pid + 1) % process_index;
    }

    for (int checked = 0; checked < process_index; checked++)
    {
        int candidateIndex = (startIndex + checked) % process_index;

        if (processes[candidateIndex].type == PROC_TYPE_USER &&
            processes[candidateIndex].status == PROC_STATUS_READY)
        {
            nextprocess = &processes[candidateIndex];
            return 1;
        }
    }

    return 0;
}

// Yield the current process
// This will give another process a chance to run
// If we yielded a user process, switch to the kernel process
// If we yielded a kernel process, switch to the next process
// The next process should have already been selected via scheduling
void yield()
{
    runningprocess->status = PROC_STATUS_READY;

    if (runningprocess->type == PROC_TYPE_USER)
    {
        prevprocess = runningprocess;
        nextprocess = kernelprocess;

        contextswitch();
        return;
    }

    if (runningprocess->type == PROC_TYPE_KERNEL)
    {
        if (schedule() == 1)
        {
            contextswitch();
        }
    }
}

// Terminate the process that is currently running (proc_t current)
// Assign the kernel as the next process to run
// Context switch to the kernel process
void exit()
{
        runningprocess->status = PROC_STATUS_TERMINATED;

    if (runningprocess->type == PROC_TYPE_USER)
    {
        prevprocess = runningprocess;
        nextprocess = kernelprocess;

        contextswitch();
    return;
    }

    // If the kernel terminates, simply return.
    return;
}


// Create a new user process
// When the process is eventually ran, start executing from the function provided (void *func)
// Initialize the stack top and base at location (void *stack)
// If we have hit the limit for maximum processes, return -1
// Store the newly created process inside the processes array (proc_t processes[])
int createuserprocess(void *func, void *stack)
{
    
    if (process_index >= MAX_PROCS)
    {
        return -1;
    }

    proc_t *newProcess = &processes[process_index];

    newProcess->pid = process_index;
    newProcess->type = PROC_TYPE_USER;
    newProcess->status = PROC_STATUS_READY;

    newProcess->esp = stack;
    newProcess->ebp = stack;
    newProcess->eip = func;

    process_index++;
    return 0;
}

# CampusCompute
Operating Systems (UE25CS242A) - Level 3 Mini-Project

CampusCompute is a command-line program written in C to demonstrate CPU scheduling, multithreading, inter-process communication, resource allocation, worker-process management and resource limits.

The project is developed for Linux/Ubuntu and can also be executed using Ubuntu on WSL.

## 1. Requirements

- Ubuntu Linux or WSL Ubuntu
- GCC compiler
- GNU Make
- POSIX threads and IPC support

Install the required build tools if they are not already available:

```bash
sudo apt update
sudo apt install build-essential
```

## 2. Compilation

Open the terminal in the project directory:

```bash
cd ~/campusCompute
```

Compile the project:

```bash
make
```

To remove the previously generated executable and object files and rebuild:

```bash
make clean
make
```

The executable generated is named `CampusCompute`.

## 3. Running the Program

To display the available commands:

```bash
./CampusCompute help
```

### Job Management

```bash
./CampusCompute jobs tests/jobs.txt
```

Reads and validates the job details, including job ID, arrival time, CPU burst and priority.

### CPU Scheduling

First Come First Serve:

```bash
./CampusCompute fcfs tests/jobs.txt
```

Round Robin with time quantum 2:

```bash
./CampusCompute rr tests/jobs.txt 2
```

Non-preemptive Priority Scheduling:

```bash
./CampusCompute priority tests/jobs.txt
```

Comparison of all three algorithms:

```bash
./CampusCompute compare tests/jobs.txt
```

The scheduling modules display execution timelines, completion times, turnaround times, waiting times and average scheduling metrics.

For FCFS and Priority Scheduling, ties are resolved using earlier arrival time and lower job ID. In Priority Scheduling, a lower priority number indicates higher priority.

Round Robin uses a FIFO ready queue. Jobs arriving at the same time are ordered by job ID, and newly arrived jobs are added before requeuing a job whose time quantum has expired.

### Producer-Consumer Logger

```bash
./CampusCompute logger
```

Uses POSIX threads to implement a bounded producer-consumer buffer. A mutex and condition variables are used for synchronization.

The generated events are written to:

```text
logs/activity.log
```

### Inter-Process Communication

```bash
./CampusCompute ipc
```

Demonstrates communication between parent and child processes using POSIX shared memory and named semaphores.

The created IPC resources are released after execution.

### Banker's Algorithm

```bash
./CampusCompute banker
```

Implements the Banker's safety algorithm using the Available, Allocation, Maximum and Need matrices.

Both safe and unsafe resource allocation states are tested. A safe sequence is displayed when one exists.

### Worker Process Management

```bash
./CampusCompute workers
```

Creates child processes using `fork()` and collects their termination status using `waitpid()`.

The program demonstrates normal worker completion and a controlled simulated failure.

### Resource Limits

```bash
./CampusCompute limits
```

Uses `setrlimit()` with `RLIMIT_NOFILE` to restrict the number of file descriptors that a child process can open.

The program attempts to open additional file descriptors until the limit is reached, resulting in an `EMFILE` error.

The resource limit is applied only to the test child process, leaving the parent process limits unchanged.

### Resource Cleanup

```bash
./CampusCompute cleanup
```

Executes the logger, IPC and worker modules and verifies their cleanup.

Threads are joined, child processes are collected, and the IPC resources created by the program are removed.

### Integrated Execution

```bash
./CampusCompute all tests/jobs.txt
```

Executes the scheduling, logging, IPC, Banker's algorithm, worker-process and resource-limit modules sequentially.

A final summary displays the number of functional module groups that passed.

## 4. Test Files

The `tests` directory contains the following input files:

- `jobs.txt` - Main job set used for scheduling.
- `jobs_idle.txt` - Additional test case for arrival gaps and scheduling ties.
- `jobs_invalid.txt` - Invalid job data used to test input validation.

Invalid input can be tested using:

```bash
./CampusCompute jobs tests/jobs_invalid.txt
./CampusCompute rr tests/jobs.txt 0
./CampusCompute unknown-command
```

The program displays error messages for invalid commands and input values.

## 5. Source Files

The implementation is divided into the following files:

- `src/main.c` - Command-line interface and integrated execution.
- `src/jobs.c` - Job input, validation and management.
- `src/scheduling.c` - FCFS, Round Robin and Priority Scheduling.
- `src/logger.c` - Producer-consumer logging using POSIX threads.
- `src/ipc.c` - Shared-memory communication and semaphore synchronization.
- `src/banker.c` - Banker's safety algorithm.
- `src/worker.c` - Worker-process creation and failure detection.
- `src/limits.c` - Resource-limit demonstration.
- `include/campus.h` - Shared structures, constants and function declarations.

The Makefile compiles the source files and generates the executable.

## 6. Cleanup

To remove the compiled executable and object files:

```bash
make clean
```

To remove the generated activity log:

```bash
rm -f logs/activity.log
```

The program automatically creates the `logs` directory when required.

Shared-memory objects and semaphores created by the program are removed after use. Child processes are collected using `waitpid()`, and the producer-consumer threads are joined before termination.

## 7. Submission

The project submission includes:

- C source files and header file
- Makefile
- README.md
- Test input files
- PDF containing terminal screenshots and their explanations

The screenshot evidence is provided in `PES1UG25CS198.pdf`.

The source files and build instructions are included in the project archive as required by the submission portal.

## 8. Notes

- CPU scheduling is simulated using the supplied job data, while the scheduling algorithms calculate the execution order and metrics.
- POSIX threads, shared memory, semaphores, child processes and resource limits use actual operating system mechanisms.
- Worker failure is deliberately simulated using a non-zero exit status.
- The Banker's algorithm uses predefined test matrices but performs the safety calculations during execution.
- Each module can be executed independently using the command-line interface.

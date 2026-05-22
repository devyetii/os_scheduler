# OS Scheduler & Memory Manager

A modular **Operating System simulator** written in C that emulates core OS components: a **Process Scheduler**, **System Clock**, **Inter-Process Communication (IPC)**, and **Memory Management**. It demonstrates how a real operating system coordinates process lifecycle, CPU scheduling, and resource synchronization using UNIX primitives.

This simulator supports multiple CPU scheduling algorithms and produces detailed performance metrics (CPU utilization, average waiting time, average turnaround time, and standard deviation of WTA).

---

## Operating System Components

This project emulates a miniature OS with the following architectural components:

### 1. System Clock (`clk.c`)
The heartbeat of the entire system. It simulates the OS clock using a shared memory segment (`shm`) that increments every real-world second.
- **Shared Memory**: All processes (generator, scheduler, worker processes) attach to the same clock segment to stay synchronized.
- **Signal Handling**: Cleans up shared memory on `SIGINT` to prevent leaks.
- **API**: `getClk()` returns the current simulation time from any process that includes `lib/clock.h`.

### 2. Process Generator (`process_generator.c`)
The **OS loader / long-term scheduler**. Its responsibilities:
- Reads the `processes.txt` input file containing process attributes (ID, arrival time, burst time, priority, size).
- Spawns the **Scheduler** as a child process.
- Spawns the **System Clock** as a child process.
- Dispatches processes to the Scheduler via **Message Queues** exactly when their arrival time matches the system clock.
- Uses **Semaphores** to synchronize hand-off between generator and scheduler.
- Handles `SIGINT` and `SIGSEGV` to clean up IPC resources gracefully.

### 3. CPU Scheduler (`scheduler.c`)
The **short-term scheduler** — the core of the OS. It manages the ready queue and dispatching of processes to the CPU.

**Key Responsibilities:**
- **Ready Queue Management**: Maintains a ready queue using custom data structures (`PriorityQueue` or `FIFOQueue`).
- **Process Dispatching**: Forks new worker processes (`process.out`) or resumes stopped ones using `SIGCONT`.
- **Context Switching**: Saves/restores process state (PCB) during preemption.
- **Preemption Logic**: Implements preemption for SRTN (shorter job arrives) and RR (quantum expires).
- **Performance Accounting**: Tracks waiting time, turnaround time, and weighted turnaround time for every process.
- **Logging & Stats**: Writes per-process events to `scheduler.log` and aggregated metrics to `scheduler.perf`.

**Supported Scheduling Algorithms:**

| Algorithm | Code | Type | Description |
|-----------|------|------|-------------|
| **HPF** | 0 | Non-Preemptive | Highest Priority First — prioritizes processes with the highest priority value |
| **SRTN** | 1 | Preemptive | Shortest Remaining Time Next — preempts if a new process has a shorter burst time |
| **RR** | 2 | Preemptive | Round Robin — each process gets a fixed time quantum (`q`) |

### 4. Worker Process (`process.c`)
The simulated **user process** that runs on the CPU.
- Decrements its own **remaining time** every clock tick.
- Uses shared memory (`remaining_time.h`) to communicate its remaining burst time back to the scheduler.
- Responds to `SIGSTOP` (preemption) and `SIGCONT` (resumption).
- Exits when remaining time reaches zero, sending `SIGCHLD` to the scheduler.

### 5. Inter-Process Communication (IPC) Layer (`lib/ipc.h`, `lib/remaining_time.h`)
The **OS communication subsystem** that connects all modules:
- **Message Queues**: Used by the Process Generator to send `ProcessData` structs to the Scheduler.
- **Semaphores**: Coordinate safe hand-off between generator and scheduler (producer-consumer pattern).
- **Shared Memory (Clock)**: One shared integer segment accessed by all processes.
- **Shared Memory (Remaining Time)**: Separate segment for each worker process to report its remaining burst time to the scheduler.

### 6. Process Control Block (PCB) & Data Structures (`lib/data_structures.h`)
The **OS metadata manager**:
- **PCB**: Stores process state (ID, arrival time, burst time, remaining time, waiting time, turnaround time, weighted TA, actual PID, state).
- **PriorityQueue**: Min-heap used for HPF and SRTN scheduling.
- **FIFOQueue**: Circular queue used for Round Robin.
- **Process States**: `STARTED`, `RESUMED`, `STOPPED`, `FINISHED`, `IDLE`.

### 7. I/O Subsystem (`lib/io.h`)
Handles all file operations:
- Reads `processes.txt` with a specific schema.
- Writes `scheduler.log` (per-process state transitions with timestamps).
- Writes `scheduler.perf` (aggregated CPU utilization, average WTA, average waiting time, standard deviation).

### 8. Test Generator (`test_generator.c`)
A utility to generate random process workloads for testing:
- Prompts for the number of processes.
- Randomly generates arrival times, burst times, priorities, and memory sizes.
- Outputs a properly formatted `processes.txt` file.

---

## Project Structure

```
.
├── clk.c                    # System Clock (shared memory incrementer)
├── process_generator.c      # OS Loader / Long-term Scheduler
├── scheduler.c              # Short-term CPU Scheduler
├── process.c                # Simulated User Process
├── test_generator.c         # Random process workload generator
├── Makefile                 # Build automation
├── lib/
│   ├── clock.h              # Clock shared memory API
│   ├── data_structures.h    # PCB, PriorityQueue, FIFOQueue
│   ├── io.h                 # File I/O helpers
│   ├── ipc.h                # Message queues & semaphores
│   ├── process_management.h # Fork, exec, signal wrappers
│   ├── remaining_time.h     # Per-process remaining time shared memory
│   ├── safeExit.h           # Graceful termination helpers
│   └── scheduling_algorithms.h # Scheduling algorithm helpers
├── processes.txt            # Input process data (generated or manual)
├── scheduler.log            # Runtime log (generated)
└── scheduler.perf           # Performance report (generated)
```

---

## How to Build & Run

### Prerequisites
- GCC / compatible C compiler
- Linux / UNIX environment (uses `sys/msg.h`, `sys/shm.h`, `sys/sem.h`)
- `make` utility

### Build
```bash
make
```

### Generate Test Input (Optional)
```bash
./test_generator.out
# Enter number of processes when prompted
```

### Run
```bash
make run
# or manually:
./process_generator.out
```

### Cleanup
```bash
make clean
```

### Input File Format (`processes.txt`)
```
#id arrival runtime priority size
1	0	5	3	200
2	2	3	1	150
3	4	7	2	300
```

### Output Files
- **`scheduler.log`** — Timestamped events (started, resumed, stopped, finished) for each process.
- **`scheduler.perf`** — Aggregated metrics:
  - CPU Utilization
  - Average Weighted Turnaround Time (WTA)
  - Average Waiting Time
  - Standard Deviation of WTA

---

## Scheduling Algorithm Details

### HPF (Highest Priority First) — Code 0
- **Type**: Non-preemptive
- **Data Structure**: Min-heap Priority Queue
- **Logic**: Processes are ordered by priority. Once a process starts, it runs to completion.
- **Use Case**: Batch systems where priority matters more than response time.

### SRTN (Shortest Remaining Time Next) — Code 1
- **Type**: Preemptive
- **Data Structure**: Min-heap Priority Queue keyed by remaining time
- **Logic**: If a newly arriving process has a shorter remaining burst time than the currently running process, the current process is preempted (`SIGSTOP`).
- **Use Case**: Minimizes average waiting time.

### RR (Round Robin) — Code 2
- **Type**: Preemptive
- **Data Structure**: FIFO Queue
- **Logic**: Each process runs for a fixed time quantum (`q`). When the quantum expires, the process is moved to the back of the queue.
- **Use Case**: Fair allocation in time-sharing systems.

---

## Key OS Concepts Demonstrated

| Concept | Implementation |
|---------|---------------|
| **Process Lifecycle** | Fork → Exec → SIGSTOP/SIGCONT → SIGCHLD → Exit |
| **Context Switching** | PCB save/restore during preemption |
| **CPU Scheduling** | HPF, SRTN, RR with custom queue data structures |
| **IPC** | Message queues (generator→scheduler), Semaphores (sync), Shared memory (clock, remaining time) |
| **Process Synchronization** | Generator produces process data; Scheduler consumes it via semaphore-guarded critical section |
| **Signal Handling** | SIGCHLD (child termination), SIGUSR1 (all processes sent), SIGINT/SIGSEGV (cleanup) |
| **Memory Management** | Shared memory segments for clock and remaining time |
| **I/O Management** | Log and performance file writing with formatted output |
| **Performance Monitoring** | Real-time WTA, waiting time, and CPU utilization calculation |

---

## Future Improvements

The following enhancements are planned to make this simulator more comprehensive and closer to a real operating system:

1. **Memory Management Integration** — Currently `psize` (process size) is read but not used. A full memory allocator (e.g., paging, segmentation, or buddy system) will be added to simulate process loading into physical memory.

2. **Multi-Level Feedback Queue (MLFQ)** — Add a more advanced scheduling algorithm with dynamic priority adjustment based on process behavior (CPU-bound vs I/O-bound).

3. **I/O Device Simulation** — Introduce simulated I/O devices (e.g., disk, printer) with device queues and blocking states, so processes can request I/O and move to a blocked queue.

4. **Deadlock Detection & Prevention** — Implement resource allocation graphs and Banker's algorithm for handling shared resources.

5. **Process Forking Tree** — Support process creation via `fork()` within the simulation, creating a process tree with parent-child relationships.

6. **Graphical / Visual Output** — Add a Gantt chart generator (text-based or web-based) to visualize scheduling timeline.

7. **Dynamic Priority Aging** — Prevent starvation in HPF by gradually increasing the priority of long-waiting processes.

8. **Real-Time Scheduling** — Add EDF (Earliest Deadline First) or Rate Monotonic scheduling for real-time process simulation.

9. **Virtual Memory & Paging** — Implement page tables, page faults, and a TLB (Translation Lookaside Buffer) simulation.

10. **Config-Driven Workloads** — Replace manual `processes.txt` with a JSON/YAML configuration format supporting multiple test scenarios.

---

## Author

[Ebrahim Gomaa](https://github.com/devyetii)

---

## License

Provided for educational and academic purposes.

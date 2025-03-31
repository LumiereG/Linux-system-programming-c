# Task Distribution System with POSIX Message Queues

This program implements a task distribution system for worker processes using POSIX message queues. The server (main process) generates random tasks and distributes them to worker processes, which perform the task and send back the results. The program simulates a real-world environment where multiple workers are assigned tasks, process them, and return results asynchronously.

### Overview

- The server generates random tasks, where each task consists of two floating-point numbers within the range of 0.0 to 100.0.
- Tasks are added to a task queue with the name `task_queue_{server_pid}`, where `{server_pid}` is the process ID of the server.
- The server periodically adds tasks to the queue at random intervals between 100 ms and 5000 ms.
- A specified number of worker processes (between 2 and 20) are created, each of which registers in the task queue and waits for available tasks.
- Workers retrieve tasks from the queue, process them by adding the two random numbers, and then sleep for a random period between 500 ms and 2000 ms to simulate work.
- Each worker sends the result back to the server through its own result queue, which is named `result_queue_{server_pid}_{worker_id}`.
- The server listens to each worker's result queue and prints the results in the format:  
  `"Result from worker {worker_pid}: {result}"`.

### Key Features

- **Task Distribution**: The server creates tasks, each with two random floating-point numbers, and adds them to the task queue.
- **Worker Processes**: Worker processes pull tasks from the task queue, compute the sum of the two numbers, and send the result back via their own result queue.
- **Message Queues**: The program uses POSIX message queues for communication between the server and workers. Each worker has a unique result queue.
- **Concurrency**: Workers handle tasks concurrently, with each worker processing up to 5 tasks before exiting.

### Execution Flow

1. **Server Process**:  
   The server process creates a task queue and starts the worker processes. It periodically generates new tasks and adds them to the queue. As workers finish processing, they send their results back to the server. The server collects and prints these results.
   
2. **Worker Processes**:  
   Each worker waits for tasks from the server, performs the task (adding two random floating-point numbers), and sends the result back to the server. Workers are created as child processes and communicate with the server via message queues.
   
3. **Result Handling**:  
   Workers use their own unique result queues to send back the results of the tasks they complete. The server listens to these result queues and prints the results once received.

### Queue Management

- Each process uses POSIX message queues with unique names to avoid conflicts across multiple instances of the program.  
  - The task queue name is `task_queue_{server_pid}`.
  - Each worker's result queue is named `result_queue_{server_pid}_{worker_id}`.

### Synchronization and Cleanup

- The server uses message queue notifications to receive results asynchronously from the workers.
- The server waits for all workers to finish before terminating.
- Resources, including message queues, are properly cleaned up at the end of the program.

### Important Notes

- The program ensures that multiple instances can run simultaneously without queue name conflicts, as each queue is uniquely identified by the server's process ID and, for result queues, the worker's process ID.
- The server handles tasks and results asynchronously, with each worker managing its own execution time (sleeping randomly after processing a task).

This program simulates a distributed system where workers independently process tasks and return results, showcasing the use of message queues for inter-process communication in a multi-process environment.

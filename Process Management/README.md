# Process Management Program

## Overview
This program creates a child process for each provided parameter. Each parameter must be an integer within the range [0-9]. The child process receives a number "n" from the parameters and randomly selects a size "s" in the range [10-100] KB. The child process then:
- Creates a file named `PID.txt` (where `PID` is the process ID).
- Writes blocks of size "s" filled with the digit "n" into the file each time it receives a `SIGUSR1` signal.
- Closes the file and terminates after approximately 1 second.

Meanwhile, the parent process:
- Sends the `SIGUSR1` signal to all child processes every 10 milliseconds for 1 second.
- Waits for all child processes to terminate before exiting.

## Compilation & Execution
### Compilation:
```sh
gcc -o process_manager process_manager.c -Wall
```

### Execution:
```sh
./process_manager 3 7 5
```
This command creates three child processes, each assigned the numbers `3`, `7`, and `5` respectively.

## File Output
Each child process generates a file named `<PID>.txt` containing repeated blocks of its assigned digit.
Example file content for a process with `n=3` and `s=12B`:
```
333333333333
```

## Signals
- `SIGUSR1` - Triggers the child process to write another block of data.

## Example Workflow
```sh
$ ./process_manager 2 4
```
- Creates two child processes: one writing "2" and another writing "4".
- Parent sends `SIGUSR1` to both processes repeatedly for 1 second.
- Each child writes its assigned digit and terminates.

## Error Handling
- Invalid input (non-integer or out-of-range values) results in an error message.
- System call failures (e.g., file operations, memory allocation) cause immediate program termination.

## License
This project is open-source and can be freely modified and distributed.

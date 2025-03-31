# Parallel File Transformer

This program uses multiple processes to transform the content of a specified text file in parallel. The program splits the file into chunks, processes each chunk with a child process, and then combines the results.

## Features:
- **Parallel Processing:** Utilizes multiple child processes to transform the content of the file concurrently.
- **Signal Handling:** Proper handling of signals, including `SIGUSR1` for starting work and `SIGINT` for interruption.
- **No Busy-Waiting:** The program employs non-blocking sleep intervals using `nanosleep` to avoid busy-waiting.
- **Character Transformation:** Every second character within the range `[a-zA-Z]` is toggled between uppercase and lowercase.

## Usage:
The program accepts two arguments:
1. The input file (`f`), which should be a text file to process.
2. The number of child processes (`n`), where 0 < n < 10. This defines how many processes will handle the parallel processing of the file.

```bash
$ ./file_transformer <num_processes> <file_name>
```

## Requirements:
- Linux-based operating system.
- A C compiler (`gcc` or similar).
- `nanosleep` and signal handling support in the operating system.

## Example:
```bash
$ ./file_transformer 3 my_file.txt

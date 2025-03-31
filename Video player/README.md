# Video Player with Multithreading, Mutexes, and Signals

## Overview

This program simulates the behavior of a movie player with multiple threads to handle the different stages of video processing. The goal is to achieve optimal performance and stability by offloading tasks to separate threads. The program uses a circular buffer for transferring video frames between threads, and it synchronizes the threads using mutexes to avoid race conditions.

The video player consists of four threads:

- **Main Thread**: Creates the other threads and handles signal processing.
- **Decoder Thread**: Responsible for decoding video frames (`decode_frame()`) and pushing them to the buffer.
- **Transformer Thread**: Takes frames from the buffer, transforms them (`transform_frame()`), and pushes them back into the buffer.
- **Display Thread**: Pops frames from the buffer and displays them (`display_frame()`) while maintaining a constant frame rate of 30 frames per second (FPS).

## Key Features

- **Circular Buffer**: Used to manage the flow of video frames between the decoder, transformer, and display threads. The buffer operates in a circular manner, wrapping around when the buffer is full or empty.
  
- **Synchronization**: Mutexes are used to synchronize access to the shared circular buffer. The threads wait (using busy waiting with 5ms sleep) until the buffer has space or contains data.
  
- **Signal Handling**: The program handles the `SIGINT` signal (Ctrl+C) gracefully. When this signal is received, the main thread signals the other threads to terminate, waits for them to finish, and frees up resources before exiting.

- **30 FPS Display**: The display thread ensures that video frames are shown at a constant frame rate of 30 FPS (one frame every 33.33 milliseconds).

## Structure

### Circular Buffer

The circular buffer structure consists of:
- A buffer array of fixed size (`BUFFER_SIZE`) for storing video frames.
- Two pointers: `head` (for inserting frames) and `tail` (for retrieving frames).
- A `count` variable to track the number of frames in the buffer.
- A `mutex` to protect the buffer and ensure thread-safe access.

### Thread Functions

1. **Decoder Thread**: 
    - Decodes a frame using the `decode_frame()` function.
    - Pushes the decoded frame to the circular buffer.

2. **Transformer Thread**:
    - Pops a frame from the buffer.
    - Transforms the frame using the `transform_frame()` function.
    - Pushes the transformed frame back into the buffer.

3. **Display Thread**:
    - Pops a frame from the buffer.
    - Displays the frame using the `display_frame()` function.
    - Ensures the display runs at 30 FPS.

### Signal Handling

- The main thread listens for the `SIGINT` signal (Ctrl+C) to terminate the program.
- Upon receiving `SIGINT`, the main thread sets a `terminate_flag`, which causes all other threads to exit their loops.
- The threads are canceled, and the program waits for all threads to finish before cleaning up resources and exiting.

## Dependencies

- **pthread**: For multithreading and synchronization.
- **time.h**: For sleeping (e.g., `nanosleep`).
- **signal.h**: For handling termination signals.

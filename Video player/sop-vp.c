#include "video-player.h"
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>

typedef unsigned int UINT;
typedef struct timespec timespec_t;

// Function to put the thread to sleep for a given time (in milliseconds)
void msleep(UINT milisec)
{
    time_t sec = (int)(milisec / 1000);
    milisec = milisec - (sec * 1000);
    timespec_t req = {0};
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    if (nanosleep(&req, &req))
        ERR("nanosleep");
}

// Structure for the circular buffer
typedef struct circular_buffer
{
    video_frame* buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mxbuffer; // Mutex to protect access to the buffer
} circular_buffer;

// Function to create a circular buffer
circular_buffer* circular_buffer_create() { 
    circular_buffer* cb = (circular_buffer*)malloc(sizeof(circular_buffer));
    if (!cb) {
        ERR("malloc");
    }
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    if (pthread_mutex_init(&cb->mxbuffer, NULL) != 0)
    {
      free(cb);
      ERR("pthread_mutex_init");
    }
    return cb;
}

// Function to push a new frame into the circular buffer
void circular_buffer_push(circular_buffer* buffer, video_frame* frame) {
    if (buffer == NULL)
      ERR("NULL passed as argument);
    while(1) {
        pthread_mutex_lock(&buffer->mxbuffer);
        if(buffer->count < BUFFER_SIZE) {
            buffer->buffer[buffer->head] = frame;
            buffer->head = (buffer->head + 1) % BUFFER_SIZE;
            buffer->count++;
            pthread_mutex_unlock(&buffer->mxbuffer);
            break;
        }
        pthread_mutex_unlock(&buffer->mxbuffer);
        msleep(5);
    }    
}

// Function to pop a frame from the circular buffer
video_frame* circular_buffer_pop(circular_buffer* buffer) {
    video_frame* frame;
    while (1) {
        pthread_mutex_lock(&buffer->mxbuffer);
        if (buffer->count > 0) {
            frame = buffer->buffer[buffer->tail];
            buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
            *buffer->count--;
            pthread_mutex_unlock(buffer->buffer);
            break;
        }
        pthread_mutex_unlock(buffer->buffer);
        usleep(5000); 
    }
    return frame;
}

// Function to destroy the circular buffer and free resources
void circular_buffer_destroy(circular_buffer* buffer) {
    if(buffer == NULL)
      return;
    pthread_mutex_destroy(&circular_buffer->->mxbuffer);
    free(buffer);
}

// Global flag to indicate termination signal
volatile sig_atomic_t terminate_flag = 0;

// Signal handler to set the termination flag when SIGINT (Ctrl+C) is received
void signal_handler(int sig)
{
    if (sig == SIGINT)
    {
        terminate_flag = 1;
    }
}

// Decoder thread function
void* decoder_thread(void* arg)
{
    circular_buffer* buffer = (circular_buffer*)arg;
    while (!terminate_flag)
    {
        video_frame* frame = decode_frame();
        circular_buffer_push(buffer, frame);
    }
    return NULL;
}

// Transformer thread function
void* transformer_thread(void* arg)
{
    circular_buffer* buffer = (circular_buffer*)arg;
    while (!terminate_flag)
    {
        video_frame* frame = circular_buffer_pop(buffer);
        transform_frame(frame);
        circular_buffer_push(buffer, frame);
    }
    return NULL;
}

// Display thread function
void* display_thread(void* arg)
{
    circular_buffer* buffer = (circular_buffer*)arg;
    while (!terminate_flag)
    {
        video_frame* frame = circular_buffer_pop(buffer);
        display_frame(frame);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    
    // Set up the signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, signal_handler);
    
    // Create the circular buffer
    circular_buffer* buffer = circular_buffer_create();

    pthread_t decoder_tid, transformer_tid, display_tid;

    // Create the decoder, transformer, and display threads
    pthread_create(&decoder_tid, NULL, decoder_thread, buffer);
    pthread_create(&transformer_tid, NULL, transformer_thread, buffer);
    pthread_create(&display_tid, NULL, display_thread, buffer);

    // Main thread waits for termination signal
    while (!terminate_flag)
    {
        sleep(1);
    }

    // Signal threads to terminate
    pthread_cancel(decoder_tid);
    pthread_cancel(transformer_tid);
    pthread_cancel(display_tid);

    // Wait for threads to finish
    pthread_join(decoder_tid, NULL);
    pthread_join(transformer_tid, NULL);
    pthread_join(display_tid, NULL);

    // Cleanup
    circular_buffer_destroy(buffer);
    return 0;
}

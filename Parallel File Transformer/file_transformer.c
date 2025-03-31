#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t last_signal = 0;
volatile sig_atomic_t sigint_flag = 0;

// Function to set signal handlers
void set_signal_handler(void (*handler)(int), int signal_number) {
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = handler;
    if (-1 == sigaction(signal_number, &act, NULL))
        ERR("sigaction");
}

// Signal handler for generic signals
void signal_handler(int sig) { last_signal = sig; }

// Signal handler for SIGINT (CTRL+C)
void sigint_handler(int sig) {
    sigint_flag++;
}

// Signal handler for SIGCHLD to reap zombie processes
void sigchld_handler(int sig) 
{
    pid_t pid;
    for (;;)
    {
        pid = waitpid(0, NULL, WNOHANG);
        if (pid == 0)
            return;
        if (pid <= 0)
        {
            if (errno == ECHILD)
                return;
            ERR("waitpid");
        }
    }
}

// Function to sleep with interruption handling
void spanko(struct timespec t)
{
    for (struct timespec ts = t; ts.tv_sec > 0 || ts.tv_nsec > 0; ) {
        if (nanosleep(&ts, &ts) == 0) {
            break;
        }
        if (errno != EINTR)
            ERR("nanosleep");
    }
}


void child_process(int child_id, int start_position, int segment_length, char* filename, sigset_t oldmask)
{
    while (!last_signal) 
        sigsuspend(&oldmask);
    if (sigint_flag) return;

    char buffer; // Buffer for file processing
    char output_filename[50]; // Name for the new output file
    snprintf(output_filename, sizeof(output_filename), "%s-%d.txt", filename, child_id);

     int input_fd, output_fd;
    if ((input_fd = open(filename, O_RDONLY)) < 0)
        ERR("open");
    if ((output_fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777)) < 0)
        ERR("open");

    lseek(input_fd, start_position, SEEK_SET);
    int read_count;
    int capitalize = 1;
    for (int i = 0; i < segment_length; i++)
    {
        if ((read_count = read(input_fd, buffer, 1)) < 0)
            ERR("read");

        // Capitalize every second letter
        if (('a' <= buffer && buffer <= 'z' ) || ('A' <= buffer && buffer <= 'Z')) capitalize++;
        if(capitalize == 2)
        {
            if('a' >= buffer && buffer <= 'z')
                buffer += 'A' - 'a';
            else buffer -= 'A' - 'a';
                capitalize = 0; 
        }

        struct timespec delay = {3, 250000000};
        if (sigint_flag) break;
        sleep_with_interrupt(delay);

         if ((read_count = write(output_fd, buffer, read_count)) < 0)
            ERR("write");
        
    }
    free(buffer);
    close(input_fd);
    close(output_fd);
}

// Function to create child processes
void create_children(char* filename, int file_size, int num_children, sigset_t oldmask) {
    int segment_size = file_size / num_children;
    int remaining_size = file_size % num_children;
    pid_t pid;
    
    for (int i = 0; i < num_children; i++) {
        set_signal_handler(signal_handler, SIGUSR1);
        set_signal_handler(sigint_handler, SIGINT);
        
        int start = i * segment_size;
        int length = segment_size;
        if (i == num_children - 1) {
            length += remaining_size;
        }
        
        if ((pid = fork()) < 0)
            ERR("fork");
        if (pid == 0) {
            child_process(i + 1, start, length, filename, oldmask);
            exit(EXIT_SUCCESS);
        }
    }
}

// Function executed by the parent process
void parent_process(sigset_t oldmask) {
    kill(0, SIGUSR1); // Send SIGUSR1 to all child processes
}

// Function to display correct program usage
void usage(char *program_name) {
    fprintf(stderr, "USAGE: %s <num_children> <filename>\n", program_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc != 3)
        usage(argv[0]);
    
    int num_children = atoi(argv[1]);
    if (num_children <= 0 || num_children >= 10)
        usage(argv[0]);
    
    char* filename = argv[2];
    struct stat file_stat;
    int input_fd;
    
    if ((input_fd = open(filename, O_RDONLY)) < 0)
        ERR("open");
    if (fstat(input_fd, &file_stat) == -1)
        ERR("fstat");
    int file_size = file_stat.st_size;
    if (close(input_fd))
        ERR("close");
    
    set_signal_handler(sigchld_handler, SIGCHLD);
    set_signal_handler(SIG_IGN, SIGUSR1);
    set_signal_handler(signal_handler, SIGINT);
    
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);
    
    create_children(filename, file_size, num_children, oldmask);
    parent_process(oldmask);
    
    while (wait(NULL) > 0);
    return EXIT_SUCCESS;
}

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

volatile sig_atomic_t sig_count = 0;
volatile sig_atomic_t last_signal = 0;

// Signal handler function
void sig_handler(int sig) {
    sig_count++;
    last_signal = sig;
}

// Function to set a signal handler
void sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    act.sa_flags = SA_RESTART;
    if (-1 == sigaction(sigNo, &act, NULL))
        ERR("sigaction");
}

// Handler for SIGCHLD to properly handle terminated child processes
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

// Function for safe bulk writing to a file descriptor
ssize_t bulk_write(int fd, char *buf, size_t s)
{
    ssize_t c;
    ssize_t len = 0;
    do
    {
        c = TEMP_FAILURE_RETRY(write(fd, buf, s));
        if (c < 0)
            return c;
        buf += c;
        len += c;
        s -= c;
    } while (s > 0);
    return len;
}

// Function executed by child processes
void child_work(int n, sigset_t oldmask)
{
    srand(time(NULL) * getpid());
    // Generate a random size for buffer (between 10KB and 100KB)
    int s = (10 + rand() % (100 - 10 + 1))*1024;
    int out;
    ssize_t count;
    char *buf = malloc(s);
    if (!buf)
        ERR("malloc");
    memset(buf, n + 48, s);

    // Generate a filename based on the process ID (PID)
    pid_t pid = getpid();
    char name[50];
    snprintf(name, sizeof(name), "%d.txt", pid);

     // Open file for writing
    if ((out = TEMP_FAILURE_RETRY(open(name, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0777))) < 0)
       ERR("open");
    
    // Wait for SIGUSR1 signal to write data
    while (1) {
        while (last_signal != SIGUSR1)
            sigsuspend(&oldmask);
        last_signal = 0;
        if ((count = bulk_write(out, buf, s)) < 0)
            ERR("read");
    }

    if (TEMP_FAILURE_RETRY(close(out)))
        ERR("close");
    free(buf);
}

// Function to display usage instructions
void usage(char *name)
{
    fprintf(stderr, "USAGE: %s 0<n\n", name);
    ERR("usage");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int n;
    if (argc < 2)
        usage(argv[0]);
    pid_t s;

    // Set signal handlers
    sethandler(sigchld_handler, SIGCHLD);
    sethandler(SIG_IGN, SIGUSR1);

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

     // Create child processes based on provided arguments
    for (int i = 1; i < argc; i++)
    {
        n = atoi(argv[i]);
        if (n < 0 || n > 9)
            usage(argv[0]);
        if ((s = fork()) < 0)
            ERR("Fork:");
        if (!s)
        {   
            sethandler(sig_handler, SIGUSR1);
            child_work(n, oldmask);
            exit(EXIT_SUCCESS);
        }
    }

    // Allow child processes to initialize
    sleep(1);
    struct timespec t = {15, 20000};

    // Send SIGUSR1 signal three times to all processes
    for(int i = 0; i < 3; i++)
    {
        nanosleep(&t, NULL);
        if (kill(0, SIGUSR1) < 0)
        ERR("kill");
    }
    
    // Wait for all child processes to terminate
    while (wait(NULL) > 0)
            ;
    return EXIT_SUCCESS;
}

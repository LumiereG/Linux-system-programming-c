#include <errno.h>
#include <mqueue.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wwait.h>
#include <time.h>
#include <unistd.h>

#define MAX_NUM 10
#define MAX_TASK_COUNT 5
#define WORKER_SLEEP_MIN 500
#define WORKER_SLEEP_MAX 2000
#define MAX_MSG_SIZE 128
#define MAX_WORKERS 20
#define TASK_QUEUE_NAME_MAX_LEN 64
#define RESULT_QUEUE_NAME_MAX_LEN 64

#define ERR(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

volatile sig_atomic_t children_left = 0;

typedef struct {
    mqd_t mq;
    char queue_name[RESULT_QUEUE_NAME_MAX_LEN];
} WorkerQueue;

// Function to handle message notification in the queue
void message_handler(union sigval sv) {
    WorkerQueue *worker_queue = (WorkerQueue *)sv.sival_ptr;
    char result[MAX_MSG_SIZE];
    
    // Receive result message from the worker's queue
    if (mq_receive(worker_queue->mq, result, MAX_MSG_SIZE, NULL) < 0) {
        perror("mq_receive");
        return;
    }

    // Extract the worker's PID from the queue name
    int worker_pid = atoi(worker_queue->queue_name + strlen("/result_queue_"));
    printf("Result from worker %d: %s\n", worker_pid, result);
}

// Function to add a task to the task queue
void add_task_to_queue(mqd_t task_queue) {
    double v1 = (rand() % 101) + (rand() % 100) / 100.0;
    double v2 = (rand() % 101) + (rand() % 100) / 100.0;

    char task[MAX_MSG_SIZE];
    snprintf(task, MAX_MSG_SIZE, "%.2f %.2f", v1, v2);

    // Try to add the task to the queue, handle full queue situation
    if (mq_send(task_queue, task, MAX_MSG_SIZE, 0) == -1) {
        perror("mq_send (server)");
        printf("Queue is full!\n");
    } else {
        printf("New task queued: [%.2f, %.2f]\n", v1, v2);
    }
}

// Parent process function that manages the tasks and workers
void parent_work(int n, mqd_t task_queue) {
    for (int i = 0; i < n * 5; ++i) {
        int wait_time = (rand() % 4001) + 1000; // Random wait between 1000 ms and 5000 ms
        usleep(wait_time * 1000);
        add_task_to_queue(task_queue); // Add new task to the queue
    }

    // Wait for child processes to finish
    srand(getpid());
    while (children_left)
        ;

    printf("All child processes have finished.\n");
}

// Worker function to process tasks from the task queue and send results
void child_work(mqd_t task_queue, mqd_t result_queue) {
    printf("[%d] Worker ready!\n", getpid());
    srand(getpid());
    
    for (int life = MAX_TASK_COUNT; life > 0; life--) {
        char task[MAX_MSG_SIZE];
        if (mq_receive(task_queue, task, MAX_MSG_SIZE, NULL) < 0) {
            ERR("mq_receive");
        }

        double v1, v2;
        sscanf(task, "%lf %lf", &v1, &v2);
        printf("[%d] Received task [%.2f, %.2f]\n", getpid(), v1, v2);

        // Simulate work - random sleep time
        int sleep_time = (rand() % (WORKER_SLEEP_MAX - WORKER_SLEEP_MIN + 1)) + WORKER_SLEEP_MIN;
        usleep(sleep_time * 1000);

        double result = v1 + v2;
        printf("[%d] Result [%.2f]\n", getpid(), result);

        // Send the result to the result queue
        char result_msg[MAX_MSG_SIZE];
        snprintf(result_msg, MAX_MSG_SIZE, "%.2f", result);
        if (mq_send(result_queue, result_msg, MAX_MSG_SIZE, 0) < 0) {
            perror("mq_send (worker)");
        }
    }

    printf("[%d] Exits!\n", getpid());
}

// Function to create worker processes
void create_children(int n, mqd_t task_queue, WorkerQueue *worker_queues) {
    while (n > 0) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            char result_queue_name[RESULT_QUEUE_NAME_MAX_LEN];
            snprintf(result_queue_name, RESULT_QUEUE_NAME_MAX_LEN, "/result_queue_%d_%d", getpid(), n);

            // Create result queue for the child
            mqd_t result_queue = mq_open(result_queue_name, O_RDWR | O_CREAT, 0600, NULL);
            if (result_queue == (mqd_t)-1) {
                perror("mq_open (child result queue)");
                exit(EXIT_FAILURE);
            }

            // Set up the notification for the result queue
            struct sigevent sev;
            sev.sigev_notify = SIGEV_THREAD;
            sev.sigev_notify_function = message_handler;
            sev.sigev_notify_attributes = NULL;
            sev.sigev_value.sival_ptr = &worker_queues[n - 1];

            if (mq_notify(result_queue, &sev) == -1) {
                perror("mq_notify (child)");
                exit(EXIT_FAILURE);
            }

            // Worker process work
            child_work(task_queue, result_queue);
            exit(0);
        } else if (pid > 0) {
            // Parent process
            snprintf(worker_queues[n - 1].queue_name, RESULT_QUEUE_NAME_MAX_LEN, "/result_queue_%d_%d", pid, n);
            worker_queues[n - 1].mq = mq_open(worker_queues[n - 1].queue_name, O_RDONLY);
            if (worker_queues[n - 1].mq == (mqd_t)-1) {
                perror("mq_open (parent)");
                exit(1);
            }

            children_left++; // Increment child counter
            n--;
        } else {
            perror("fork");
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    int n = 3; // Number of workers (children)

    printf("Server is starting...\n");

    mqd_t task_queue;
    char task_queue_name[TASK_QUEUE_NAME_MAX_LEN];
    snprintf(task_queue_name, TASK_QUEUE_NAME_MAX_LEN, "/task_queue_%d", getpid());
    task_queue = mq_open(task_queue_name, O_RDWR | O_CREAT, 0600, NULL);
    if (task_queue == (mqd_t)-1) {
        perror("mq_open (task queue)");
        exit(EXIT_FAILURE);
    }

    WorkerQueue worker_queues[MAX_WORKERS];

    // Create child worker processes
    create_children(n, task_queue, worker_queues);
    // Parent process manages tasks and workers
    parent_work(n, task_queue);

    printf("Server shutting down...\n");

    // Clean up resources
    mq_close(task_queue);
    for (int i = 0; i < n; i++) {
        mq_close(worker_queues[i].mq);
        mq_unlink(worker_queues[i].queue_name);
    }

    return EXIT_SUCCESS;
}

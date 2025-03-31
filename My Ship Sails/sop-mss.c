#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))
#define UNUSED(x) ((void)(x))

#define DECK_SIZE (4 * 13)
#define HAND_SIZE (7)
#define MAX_PLAYERS 7
#define MIN_PLAYERS 4

// Structure representing a player
typedef struct
{
    int hand[HAND_SIZE]; // Player's hand
    int id; // Player ID
    pthread_t thread; // Thread assigned to the player
} player_t;

pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for synchronizing game state
pthread_cond_t game_start = PTHREAD_COND_INITIALIZER; // Condition variable for starting the game
player_t players[MAX_PLAYERS]; // Array of players
int player_count = 0; // Current number of players
int game_running = 1; // Flag indicating if the game is running
sem_t game_sem; // Semaphore for controlling turns

// Function to shuffle an array (used for shuffling the deck)
void shuffle(int *array, size_t n)
{
    if (n > 1)
    {
        for (size_t i = 0; i < n - 1; i++)
        {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

// Function to print a player's hand
void print_hand(int *hand)
{
    for (int i = 0; i < HAND_SIZE; i++)
    {
        printf("%d ", hand[i]);
    }
    printf("\n");
}

// Thread function for a player
void *player_thread(void *arg)
{
    player_t *player = (player_t *)arg;
    pthread_mutex_lock(&game_mutex);
    while (player_count < MIN_PLAYERS)
    {
        pthread_cond_wait(&game_start, &game_mutex); // Wait for enough players to join
    }
    pthread_mutex_unlock(&game_mutex);

    printf("Player %d joined with hand: ", player->id);
    print_hand(player->hand);
    
    // Simulation of the game (card exchange) - to be expanded
    while (game_running)
    {
        sem_wait(&game_sem); // Synchronize turns using semaphore
        sleep(1); // Simulate turn duration
        sem_post(&game_sem);
    }

    return NULL;
}

// Signal handler for SIGUSR1 - adds a new player
void sigusr1_handler(int sig)
{
    UNUSED(sig);
    pthread_mutex_lock(&game_mutex);
    if (player_count < MAX_PLAYERS)
    {
        players[player_count].id = player_count + 1;
        pthread_create(&players[player_count].thread, NULL, player_thread, &players[player_count]);
        player_count++;
        if (player_count >= MIN_PLAYERS)
        {
            pthread_cond_broadcast(&game_start); // Start game when enough players join
        }
    }
    else
    {
        printf("No more seats available.\n");
    }
    pthread_mutex_unlock(&game_mutex);
}

// Signal handler for SIGINT - terminates the game
void sigint_handler(int sig)
{
    UNUSED(sig);
    game_running = 0; // Set flag to stop game
    for (int i = 0; i < player_count; i++)
    {
        pthread_join(players[i].thread, NULL); // Wait for all player threads to terminate
    }
    sem_destroy(&game_sem); // Destroy semaphore
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "USAGE: %s n\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);
    if (n < MIN_PLAYERS || n > MAX_PLAYERS)
    {
        fprintf(stderr, "Number of players must be between %d and %d.\n", MIN_PLAYERS, MAX_PLAYERS);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    signal(SIGUSR1, sigusr1_handler); // Register SIGUSR1 handler for adding players
    signal(SIGINT, sigint_handler); // Register SIGINT handler for termination
    sem_init(&game_sem, 0, 1); // Initialize semaphore

    printf("Game is ready. Send SIGUSR1 to add players.\n");
    while (game_running)
    {
        pause(); // Wait for signals
    }
    return EXIT_SUCCESS;
}

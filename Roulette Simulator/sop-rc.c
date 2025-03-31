#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define MIN_BET 1               // Minimum bet amount
#define MAX_NUMBER 36           // Maximum number on the roulette wheel
#define EXIT_PROBABILITY 10     // Probability (in %) that a player exits the game

// Function that simulates the player's behavior
void player_process(int id, int start_money, int write_fd, int read_fd) {
    int money = start_money;     // Player's starting money
    srand(time(NULL) ^ getpid()); // Seed random number generator based on the current time and process ID
    printf("%d: I have %d and I'm going to play roulette.\n", getpid(), money);

    while (money > 0) {  // Continue playing as long as the player has money
        // 10% chance that the player exits the game with remaining money
        if (rand() % 100 < EXIT_PROBABILITY) {
            printf("%d: I saved %d\n", getpid(), money);
            exit(0);
        }

        int bet = (rand() % money) + MIN_BET;   // Random bet (between 1 and the player's current money)
        int number = rand() % (MAX_NUMBER + 1); // Random number to bet on (between 0 and 36)
        
        // Send the bet and chosen number to the dealer (krupier)
        dprintf(write_fd, "%d %d\n", bet, number);
        money -= bet;  // Deduct the bet amount from player's money
        
        int result;
        // Read the result of the spin from the dealer
        if (read(read_fd, &result, sizeof(int)) > 0 && result == number) {
            int winnings = bet * 35;    // Calculate the winnings if the player wins
            money += winnings;          // Add winnings to the player's money
            printf("%d: I won %d.\n", getpid(), winnings);
        }
    }
    
    // If the player runs out of money, exit
    printf("%d: I'm broke.\n", getpid());
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {  // Check if the correct number of arguments is passed
        fprintf(stderr, "Usage: %s <number_of_players> <start_money>\n", argv[0]);
        return 1;
    }
    
    int num_players = atoi(argv[1]);   // Number of players (N)
    int start_money = atoi(argv[2]);   // Starting money (M)
    
    // Validate the input values
    if (num_players < 1 || start_money < 100) {
        fprintf(stderr, "Invalid input: N >= 1, M >= 100\n");
        return 1;
    }
    
    // Create pipes for communication between players and the dealer
    int pipes[num_players][2], result_pipes[num_players][2];
    pid_t pids[num_players];
    
    // Create player processes
    for (int i = 0; i < num_players; i++) {
        pipe(pipes[i]);           // Pipe for player to send bet data
        pipe(result_pipes[i]);    // Pipe for dealer to send result back to player
        
        if ((pids[i] = fork()) == 0) {  // Create a child process for each player
            close(pipes[i][0]);   // Close unused read end of pipe
            close(result_pipes[i][1]); // Close unused write end of result pipe
            player_process(i, start_money, pipes[i][1], result_pipes[i][0]);  // Call player process function
        }
        
        close(pipes[i][1]);      // Close unused write end of pipe
        close(result_pipes[i][0]);  // Close unused read end of result pipe
    }
    
    srand(time(NULL));  // Seed the random number generator for the dealer
    
    while (1) {
        int active_players = 0;  // Counter for active players (those still in the game)
        
        // Dealer receives bets from players
        for (int i = 0; i < num_players; i++) {
            int bet, number;
            // Read bet and number chosen by the player
            if (read(pipes[i][0], &bet, sizeof(int)) > 0 && read(pipes[i][0], &number, sizeof(int)) > 0) {
                printf("Dealer: %d placed %d on %d\n", pids[i], bet, number);
                active_players++;  // Increment active players count
            }
        }
        
        // If no active players, the casino always wins
        if (active_players == 0) {
            printf("Dealer: Casino always wins\n");
            break;
        }
        
        int lucky_number = rand() % (MAX_NUMBER + 1);  // Randomly select a lucky number
        printf("Dealer: %d is the lucky number.\n", lucky_number);
        
        // Send the lucky number to all players
        for (int i = 0; i < num_players; i++) {
            write(result_pipes[i][1], &lucky_number, sizeof(int));
        }
    }
    
    // Wait for all players to finish before exiting
    for (int i = 0; i < num_players; i++) {
        wait(NULL); 
    }
    return 0;  
}

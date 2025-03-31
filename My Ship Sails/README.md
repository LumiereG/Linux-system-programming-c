# Laboratory Task 4: Synchronization

## Table of Contents
- [Description](#description)
- [Game Rules](#game-rules)
- [Task Description](#task-description)
- [How the Code Works](#how-the-code-works)
- [Signals Used](#signals-used)
- [Compilation and Execution](#compilation-and-execution)

## Description
This project is a simulation of the card game **"My Ship Sails"**, implemented using **POSIX threads, mutexes, semaphores, and condition variables** for synchronization. The game consists of a dealer (main thread) and multiple players (threads). The program waits for players to join and then simulates the game until a player wins.

## Game Rules
Each player receives **7 cards** from a standard deck of **52 cards**. The game proceeds in turns, where all players simultaneously pass one card to the player on their right. The game continues until one or more players collect **all 7 cards of the same suit**, at which point they announce **"My Ship Sails!"**, and the game ends.

## Task Description
- The deck of cards is represented as an array of numbers from **0 to 51**.
- A card's **value** is determined by `card_number / 4`, and its **suit** is determined by `card_number % 4`.
- The dealer (main thread) coordinates the game by dealing cards and seating players at the table.
- Players join by sending the `SIGUSR1` signal. If there is an available seat, a new player thread is created and receives a hand of cards.
- Once the required number of players is met (between **4 and 7**), the game begins.
- Players check for a win condition in parallel, exchange cards, and continue until a winner is found.
- When a player wins, the game stops, and all threads terminate.
- The dealer waits for all player threads to finish, shuffles the deck, and starts over, waiting for new players.
- If the `SIGINT` signal is received, the game stops immediately, all threads terminate, and resources are released.

## Signals Used
- `SIGUSR1` - Adds a new player to the game.
- `SIGINT` - Terminates the game and cleans up resources.

## Compilation and Execution
To compile the program, use:
```sh
gcc -o my_ship_sails my_ship_sails.c -pthread
```
To run the program with `n` players:
```sh
./my_ship_sails n
```
Players can be added dynamically by sending the `SIGUSR1` signal:
```sh
kill -SIGUSR1 <PID>
```
To stop the program, send `SIGINT`:
```sh
kill -SIGINT <PID>
```


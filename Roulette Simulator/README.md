# Roulette Simulator using Pipes/FIFOs

## Overview

This program simulates a roulette game where multiple players place bets on a single number (from 0 to 36) with a payout of 35:1. The game is played using inter-process communication (IPC) via unnamed pipes between the dealer (main process) and the players (child processes). The dealer handles the game logic, such as drawing a random lucky number and announcing the results, while the players place their bets and determine whether they win or lose based on the lucky number.

## Features

- Multiple players can participate in the game (at least one player is required).
- Each player starts with an initial amount of money.
- Players place random bets on a number between 0 and 36, and the dealer picks a lucky number.
- If a player's bet matches the lucky number, they win 35 times their bet.
- A player has a 10% chance to exit the game with their remaining money in each round.
- The game continues until all players run out of money or exit voluntarily.
- The dealer announces the results after each round and ends the game when all players have left.

## How It Works

### Inputs
The program takes two command-line arguments:
1. **N (number of players)** - The number of players (must be ≥ 1).
2. **M (starting money)** - The starting money for each player (must be ≥ 100).

### Execution

1. The dealer (main process) creates child processes for each player.
2. Each player communicates with the dealer through an unnamed pipe:
   - They send a random bet (up to their remaining money) and a chosen number.
3. The dealer:
   - Receives the bet and chosen number from each player.
   - Draws a random lucky number from 0 to 36.
   - Announces the lucky number.
4. The dealer then informs each player of the lucky number.
   - If a player's bet matches the lucky number, they win 35 times their bet and have the winnings added to their balance.
   - If a player runs out of money or chooses to leave, they announce they are "broke" or "saved."
5. The game continues in rounds until no players remain.

### Output

- Players announce:
  - Their initial money and decision to play the game.
  - The outcome of their bet (win or lose).
  - Whether they saved their remaining money and left the game.
- The dealer announces:
  - The player's bet and chosen number.
  - The lucky number.
  - If the casino "always wins" (i.e., all players leave the game).

### Example Output

```sh
1234: I have 100 and I'm going to play roulette.
5678: I have 100 and I'm going to play roulette.
Dealer: 1234 placed 50 on 23
Dealer: 5678 placed 30 on 15
Dealer: 23 is the lucky number.
1234: I won 1750.
5678: I lost.
1234: I saved 1800
Dealer: Casino always wins

🕹️ **Hangman Game in C (Cross Platform Version)**

A console based Hangman game written in C. This version works on both Windows and Linux and demonstrates practical concepts like string handling, input validation, ASCII rendering, and basic terminal control.

The game supports two players:

* Player 1 enters a secret word (hidden with `*`)
* Player 2 guesses letters until they win or run out of lives

---

📌 **Features**

* Secret word input is hidden (masked with `*`)
* Works on Windows and Linux
* Input validation (only alphabet letters allowed)
* Prevents repeated guesses
* Tracks and displays guessed letters
* ASCII Hangman drawing that updates with lives
* Win and Game Over screens
* Exit anytime by entering `0`
* Clears screen between turns for clean gameplay

---

🛠️ **Technologies Used**

**C Programming Language**

**Standard Libraries:**

* stdio.h
* string.h
* ctype.h
* stdlib.h
* stdint.h

**Platform Specific Libraries:**

* conio.h (Windows for `_getch()`)
* termios.h and unistd.h (Linux for hidden input)

---

▶️ **How the Game Works**

1. Player 1 enters a secret word (recommended 6 to 9 letters).
2. The word is masked with `*` while typing so it remains hidden.
3. Player 2 guesses one letter at a time.

If the guess is correct:

* The letter is revealed in the word.

If incorrect:

* A life is lost.
* The hangman drawing updates.

The game ends when:

* The word is fully guessed (Win)
* Lives reach 0 (Game Over)

---

❤️ **Lives System**

* The player starts with 6 lives.
* Each wrong guess:

    * Reduces lives by 1
    * Updates the hangman drawing
* When lives reach 0, the full hangman is displayed and the game ends.

---

📂 **Project Structure**

Single file program:

`hangman.c`

Main functions:

* `clear_screen()` → Clears console
* `get_secret_word()` → Securely captures hidden word
* `draw_hangman(int lives)` → Displays ASCII hangman
* `main()` → Controls full game logic

---

🖥️ **How to Compile and Run**

### Using GCC (Windows – MinGW / MSVC)

```bash
gcc hangman.c -o hangman.exe
hangman.exe
```

### Using GCC (Linux)

```bash
gcc hangman.c -o hangman
./hangman
```

No external libraries are required.

---

🎯 **Learning Concepts Covered**

This project helps you understand:

* Character arrays (strings)
* String functions (`strlen`, `strcmp`, `strchr`)
* Conditional statements
* Loops
* Switch-case
* Input validation
* ASCII art rendering
* Cross platform terminal handling
* Basic game logic
* Exit handling with `exit(0)`

---

⚠️ **Limitations**

* Console based UI only
* No multiplayer over network
* No file based word storage
* No graphical interface

---

🚀 **Future Improvements (Ideas)**

* Add difficulty levels
* Add word categories
* Random word generator
* Score tracking
* Replay option
* Timer mode
* Colored console output
* Store words in a file

---

📸 **Sample Gameplay (Console View)**

```
WORD: _ A _ _ _ A N
LIVES: 4
GUESSED: A, N, T

Enter a letter to guess (0 to Exit).
Input guess:
```

---

📄 **License**

This project is open source and free to use for learning and educational purposes. You may distribute it under the MIT License if desired.

---

👨‍💻 **Author**

Created as a beginner friendly C programming project. Feel free to fork, improve, and experiment with it.

If this helped you, consider giving it a ⭐ on GitHub.

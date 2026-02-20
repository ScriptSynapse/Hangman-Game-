# 🕹️ Hangman Game in C

A simple console-based Hangman game written in C.
This project is designed for beginners who want to understand loops, conditionals, arrays, strings, and basic input handling in C.

The game supports two players:

* Player 1 enters a secret word (hidden with `*`)
* Player 2 guesses letters until they either win or run out of lives

---

## 📌 Features

* Secret word input is hidden using `_getch()`
* Input validation (only letters allowed)
* Prevents repeated guesses
* Tracks and displays guessed letters
* ASCII Hangman drawing that updates with lives
* Win and Game Over screens
* Exit anytime by entering `0`

---

## 🛠️ Technologies Used

* C Programming Language
* Standard Libraries:

    * `stdio.h`
    * `string.h`
    * `ctype.h`
    * `stdlib.h`
    * `conio.h` (for `_getch()`)

---

## ▶️ How the Game Works

1. Player 1 enters a secret word (recommended 6 to 9 letters).
2. The word is masked with `*` so it is not visible.
3. Player 2 guesses one letter at a time.
4. If the guess is correct:

    * The letter is revealed in the word.
5. If incorrect:

    * A life is lost.
    * The hangman drawing updates.
6. The game ends when:

    * The word is fully guessed (Win)
    * Lives reach 0 (Game Over)

---

## ❤️ Lives System

The player starts with **6 lives**.

Each wrong guess:

* Reduces lives by 1
* Updates the hangman drawing

When lives reach 0, the full hangman is displayed and the game ends.

---

## 📂 Project Structure

Single file program:

```
hangman.c
```

Main functions:

* `clear_screen()` → Clears console
* `get_secret_word()` → Securely captures hidden word
* `draw_hangman(int lives)` → Displays ASCII hangman
* `main()` → Controls full game logic

---

## 🖥️ How to Compile and Run

### Using GCC (Windows)

```bash
gcc hangman.c -o hangman
hangman
```

> Note: This program uses `conio.h` and `_getch()`, so it works best on Windows (MinGW / Turbo C / MSVC).

---

## 🎯 Learning Concepts Covered

This project helps you understand:

* Character arrays (strings)
* String functions (`strlen`, `strcmp`, `strchr`)
* Conditional statements
* Loops
* Switch-case
* Input validation
* ASCII art rendering
* Basic game logic
* Exit handling with `exit(0)`

---

## ⚠️ Limitations

* Designed primarily for Windows due to `conio.h`
* No multiplayer over network
* No file-based word storage
* Console-based UI only

---

## 🚀 Future Improvements (Ideas)

* Add difficulty levels
* Add word categories
* Random word generator
* Score tracking
* Timer mode
* Cross-platform support (remove `conio.h`)
* Colored console output

---

## 📸 Sample Gameplay (Console View)

```
WORD: _ A _ _ _ A N
LIVES: 4
GUESSED: A, N, T

Enter a letter to guess (0 to Exit).
Input guess:
```

---

## 📄 License

This project is open-source and free to use for learning purposes.

---

## 👨‍💻 Author

Created as a beginner-friendly C programming project.
Feel free to fork, improve, and experiment with it.

---

If you found this helpful, consider giving it a ⭐ on GitHub.

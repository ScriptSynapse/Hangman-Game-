## 🕹️ Hangman Game in C (Cross-Platform)

A clean, console-based Hangman game written in C.
Built for beginners who want hands-on practice with loops, conditionals, arrays, strings, and platform-specific input handling.

Two-player format:

* **Player 1** enters a secret word (hidden with `*`)
* **Player 2** guesses letters until they win or run out of lives

Simple. Fun. Great for learning core C concepts.

---

## 📌 Features

* 🔒 Hidden secret word input (masked with `*`)
* 🖥️ Works on **Windows and Linux**
* ✅ Input validation (letters only)
* ⚠️ Prevents repeated guesses
* 📋 Tracks and displays guessed letters
* 🎨 Dynamic ASCII Hangman drawing
* 🏆 Win screen and 💀 Game Over screen
* 🚪 Exit anytime by entering `0`
* 🧹 Clears screen between turns for clean gameplay

---

## 🛠️ Technologies Used

### Language

* C Programming Language

### Standard Libraries

* `stdio.h`
* `string.h`
* `ctype.h`
* `stdlib.h`
* `stdint.h`

### Platform-Specific Libraries

* **Windows:** `conio.h` (for `_getch()`)
* **Linux:** `termios.h`, `unistd.h` (for hidden input handling)

---

## ▶️ How the Game Works

1. Player 1 enters a secret word (recommended 6–9 letters).
2. The word is masked while typing so it stays hidden.
3. Player 2 guesses one letter at a time.

### If the guess is correct:

* The letter appears in the word.

### If the guess is wrong:

* One life is lost.
* The hangman drawing updates.

### The game ends when:

* ✅ The word is fully guessed (Win)
* ❌ Lives reach 0 (Game Over)

---

## ❤️ Lives System

* The player starts with **6 lives**
* Each incorrect guess:

  * Reduces lives by 1
  * Updates the hangman drawing
* At 0 lives, the full hangman appears and the game ends

---

## 📂 Project Structure

Single file program:

```
hangman.c
```

### Main Functions

* `clear_screen()` → Clears the console
* `get_secret_word()` → Captures hidden input securely
* `draw_hangman(int lives)` → Displays ASCII hangman
* `main()` → Controls full game logic

---

## 🖥️ How to Compile and Run

### 🔹 Windows (GCC / MinGW / MSVC)

```bash
gcc hangman.c -o hangman.exe
hangman.exe
```

### 🔹 Linux

```bash
gcc hangman.c -o hangman
./hangman
```

No external libraries required.

---

## 🎯 Learning Concepts Covered

This project strengthens your understanding of:

* Character arrays (strings)
* String functions (`strlen`, `strcmp`, `strchr`)
* Conditional statements
* Loops
* Switch-case
* Input validation
* ASCII art rendering
* Cross-platform terminal handling
* Basic game logic
* Exit handling using `exit(0)`

If you're a first-year BTech student, this is a strong portfolio project because it shows real control over strings and OS-level input behavior.

---

## ⚠️ Limitations

* Console-based UI only
* No network multiplayer
* No file-based word storage
* No graphical interface

---

## 🚀 Future Improvements (Ideas)

* Difficulty levels
* Word categories
* Random word generator
* Score tracking system
* Replay option
* Timer mode
* Colored console output
* Store words in a file



---

## 📄 License

Open-source and free to use for learning purposes.
You may distribute it under the MIT License if desired.

---

## 👨‍💻 Author

Created as a beginner-friendly C programming project.
Feel free to fork it, improve it, and experiment with new features.

If you found it useful, consider giving it a ⭐ on GitHub.

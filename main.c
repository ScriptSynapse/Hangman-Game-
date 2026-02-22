#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h> // for exit(), malloc, realloc, free
#include <ctype.h>  // for isalpha(), toupper()
#include <string.h> // for memcpy, memcmp, memset
#include <stdint.h>

#ifdef _WIN32
    #include <conio.h>
    const int  KEY_ENTER = 13;
    const int  KEY_BACKSPACE = 8;
    void clear_screen(){system("cls");}
    int get_char_hidden() {
        return _getch();
    }
#endif
#ifdef __linux__
    #include <termios.h>
    #include <unistd.h>
    const unsigned char  KEY_ENTER = '\n';
    const int  KEY_BACKSPACE = 127;

        void clear_screen(){system("clear");}

    int get_char_hidden() {
        struct termios oldt, newt;
        int ch;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO); // &= is like +=
        // | is for ORing bit flags and ~ as for negating them
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return ch;
    }
#endif
void get_secret_word(char word[]) {
    int index = 0;
    int ch;

    printf("Enter the Secret Word (recommended [6-9] letters) >> ");

    for (;;) {
        ch = get_char_hidden();
        // Capture input without printing
        //gets the input from the keyboard directly
        //The _getch() function returns an 8-bit value
        //the OS has a predefined number for each input

        if (ch == EOF) exit(0);

        if (ch == KEY_ENTER) {
            if (index > 0) {
                word[index] = '\0';
                break;
            }
        }
        else if (ch == KEY_BACKSPACE ) { // Check both for safety
            if (index > 0) {
                index--;
                printf("\b \b");
                    //\b (first Backspace): Moves the cursor one position to the left. It does not erase anything; it just moves the "pen."
    //(Space): Prints a space character over the old letter. This effectively hides the character from the user.
    //\b (second Backspace): Moves the cursor back to the left again. Without this, the cursor would be sitting one space to the right of where it needs to be for the next input.

            }
        }
        else if (isalpha(ch)) {
            word[index] = (char)toupper(ch);//take the input
            index++;
            printf("*");//and mask the input by printing a    *
        }
    }
    printf("\n"); // Move to next line after word is entered
}

void draw_hangman(int lives) {
    switch (lives) {
   case 6: // Empty
        printf("   +---------------+ \n");
        printf("   |               | \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   =======================\n");
        break;
    case 5: // Head
        printf("   +---------------+ \n");
        printf("   |               | \n");
        printf("   |            _______   \n");
        printf("   |           |       |   \n");
        printf("   |           |       |   \n");
        printf("   |           |_______| \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   =======================\n");
        break;
    case 4: // Body
        printf("   +---------------+\n");
        printf("   |               |\n");
        printf("   |            _______   \n");
        printf("   |           |       |   \n");
        printf("   |           |       |   \n");
        printf("   |           |_______| \n");
        printf("   |               | \n");
        printf("   |            ___|___\n");
        printf("   |               |  \n");
        printf("   |               |   \n");
        printf("   |               |    \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   =======================\n");
        break;
    case 3: // Left Arm
        printf("   +---------------+\n");
        printf("   |               |\n");
        printf("   |           _______   \n");
        printf("   |      \\   |       |   \n");
        printf("   |       \\  |       |   \n");
        printf("   |        \\ |_______| \n");
        printf("   |         \\    |    \n");
        printf("   |          \\___|___\n");
        printf("   |              |  \n");
        printf("   |              |   \n");
        printf("   |              |    \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   =======================\n");
        break;
    case 2: // Right Arm
        printf("   +---------------+\n");
        printf("   |               |\n");
        printf("   |           _______   \n");
        printf("   |      \\   |       |   /\n");
        printf("   |       \\  |       |  / \n");
        printf("   |        \\ |_______| /\n");
        printf("   |         \\    |    /\n");
        printf("   |          \\___|___/\n");
        printf("   |              |  \n");
        printf("   |              |   \n");
        printf("   |              |    \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   |               \n");
        printf("   =======================\n");
        break;
    case 1: // Left Leg
        printf("   +---------------+\n");
        printf("   |               |\n");
        printf("   |           _______   \n");
        printf("   |      \\   |       |   /\n");
        printf("   |       \\  |       |  / \n");
        printf("   |        \\ |_______| /\n");
        printf("   |         \\    |    /\n");
        printf("   |          \\___|___/\n");
        printf("   |              |   \n");
        printf("   |              |   \n");
        printf("   |              |    \n");
        printf("   |             / \n");
        printf("   |            /    \n");
        printf("   |           /    \n");
        printf("   |          /   \n");
        printf("   |         /    \n");
        printf("   =======================\n");
        break;
    case 0: // GAME OVER
        printf("   +---------------+\n");
        printf("   |               |\n");
        printf("   |            _______   \n");
        printf("   |           |       |    \n");
        printf("   |           | x   x |    \n");
        printf("   |           |_______|  \n");
        printf("   |               |     \n");
        printf("   |            ___|___ \n");
        printf("   |           /   |   \\ \n");
        printf("   |          /    |    \\ \n");
        printf("   |         /     |     \\ \n");
        printf("   |        /     / \\     \\ \n");
        printf("   |       /     /   \\     \\ \n");
        printf("   |            /     \\ \n");
        printf("   |           /       \\ \n");
        printf("   |          /         \\ \n");
        printf("   =======================\n");
        break;
    }
}

int main() {
    char secret_word[100], guess_word[100];
    char guessed_letters[27] = "";
    char message[100] = "Enter a letter to guess (0 to Exit).";
    int lives = 6;
    char input;

    clear_screen();
    get_secret_word(secret_word);

    size_t word_length = strlen(secret_word);
    for (size_t i = 0; i < word_length; i++) {
        guess_word[i] = '_';
    }    guess_word[word_length] = '\0';

    while (lives > 0 && strcmp(secret_word, guess_word) != 0) {
        clear_screen();
        draw_hangman(lives);
        printf("\nWORD: %s\n", guess_word);
        printf("LIVES: %d\n", lives);
        printf("GUESSED: ");

        size_t guessed_len = strlen(guessed_letters);

        for (size_t j = 0; j < guessed_len; j++) {
            printf("%c", guessed_letters[j]);
            if (j < guessed_len - 1) {
                printf(", ");
            }
        }

        printf("\n\n%s\nInput guess: ", message);

        //can use either the try buffer or the while getchar(){this is much safer} loop

        char try_buffer[100];
        scanf("%s", try_buffer);
        input = try_buffer[0];
        getchar();//get the newline

        //or

        //scanf(" %c", &input);
        //int c;
        //while ((c = getchar()) != '\n' && c != EOF);

        if (input == '0') exit(0);

        input = (char)toupper(input);
        //sprintf overwrite the buffer
        if (!isalpha(input)) {
            sprintf(message, "ERROR: '%c' is not a letter!", input);
        }
        else if (strchr(guessed_letters, input)) {
            sprintf(message, "WARNING: You already guessed '%c'.", input);
        }
        else {
            // Add to guessed letters
            size_t len = strlen(guessed_letters);
            guessed_letters[len] = input;
            guessed_letters[len + 1] = '\0';

            if (strchr(secret_word, input)) {
                sprintf(message, "Good job! Found '%c'.", input);

                for (size_t i = 0; i < word_length; i++) {
                    if (secret_word[i] == input) guess_word[i] = input;
                }

            } else {
                lives--;
                sprintf(message, "Sorry, '%c' is not in the word.", input);
            }
        }
    }
    clear_screen();
    draw_hangman(lives);
    if (lives > 0) {
        printf("\nWORD: %s\n\nYOU WIN! The word was: %s\n", guess_word, secret_word);
        printf("W     W  IIIII  N   N  N   N  EEEEE  RRRR  \n");
        printf("W     W    I    NN  N  NN  N  E      R   R \n");
        printf("W  W  W    I    N N N  N N N  EEEE   RRRR  \n");
        printf("W W W W    I    N  NN  N  NN  E      R  R  \n");
        printf("WW   WW  IIIII  N   N  N   N  EEEEE  R   R \n");
    } else {
        printf("\nGAME OVER! The word was: %s\n\nYou guessed: %s\n", secret_word, guess_word);
    }
        printf("\nPress Enter to Exit...");
        getchar();
    return(0);
}
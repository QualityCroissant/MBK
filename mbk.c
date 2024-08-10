/* MBK - A tool for putting books into Minecraft
 * Copyright (C) 2024 Finn Chipp
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>

#define ALLOC_SIZE 50

#define MAX_PAGE_CHARS 196

#define COMMAND_START "give @p written_book[written_book_content={"
#define COMMAND_END "}]"

#define TITLE_START "title:\""
#define TITLE_END "\""

#define AUTHOR_START "author:\""
#define AUTHOR_END "\""

#define PAGES_START "pages:["
#define PAGES_END "]"

#define PAGE_START "'{\"text\":\""
#define PAGE_END "\"}'"

#define SEPARATOR ","

#define NEWLINE "\\\\n"
#define DOUBLE_QUOTE "\\\\\""
#define SINGLE_QUOTE "\\\\'"

#define NO_UTF8_ENCODINGS 3 // Number of bitmasks that if matched, indicate the start of a multibyte character

struct utf8_encoding {
    unsigned char mask,
                  no_bytes;
} UTF8_ENCODINGS[NO_UTF8_ENCODINGS] = { // All multibyte encodings, and their corresponding number of trailling bytes for the multibyte char
    {0b11110000, 3},
    {0b11100000, 2},
    {0b11000000, 1}
};

int main(void) {
    size_t titleSize = ALLOC_SIZE,
           authorSize = ALLOC_SIZE,
           filenameSize = ALLOC_SIZE,
           maxChars,
           wordLength,
           oldPos;
    ssize_t titleLength,
            authorLength,
            filenameLength;
    char *title,
         *author,
         *filename;
    unsigned char input;
    FILE *f;

    // Initialisations:

    if((title = calloc(titleSize, sizeof(char))) == NULL) { // Attempt to allocate memory for title
        perror("MBK -> Could not allocate memory");

        return 1;
    }

    if((author = calloc(authorSize, sizeof(char))) == NULL) { // Attempt to allocate memory for author
        perror("MBK -> Could not allocate memory");

        free(title);

        return 1;
    }

    if((filename = calloc(filenameSize, sizeof(char))) == NULL) {  // Attempt to allocate memory for filename
        perror("MBK -> Could not allocate memory");

        free(author);
        free(title);

        return 1;
    }

    printf("Welcome :3\nEnsure that the text file containing your book is encoded in UTF-8!\n\nTitle:                     "); // Prompt for title entry

    if((titleLength = getline(&title, &titleSize, stdin)) == -1) { // Input title
        perror("MBK -> Couldn't allocate memory for title");

        free(title);
        free(author);
        free(filename);

        return 1;
    }

    title[--titleLength] = '\0'; // Makes sure it doesn't end with '\n'

    printf("Author:                    "); // Prompt for author entry

    if((authorLength = getline(&author, &authorSize, stdin)) == -1) { // Input author
        perror("MBK -> Couldn't allocate memory for author's name");

        free(title);
        free(author);
        free(filename);

        return 1;
    }

    author[--authorLength] = '\0'; // Make sure it doesn't end with '\n'

    printf("File containing book text: "); // Prompt for filename

    if((filenameLength = getline(&filename, &filenameSize, stdin)) == -1) { // Input filename
        perror("MBK -> Couldn't allocate memory for filename");
    
        free(title);
        free(author);
        free(filename);
    
        return 1;
    }

    filename[--filenameLength] = '\0'; // Make sure it doesn't end with '\n'

    if((f = fopen(filename, "r")) == NULL) { // Attempt to open file
        perror("MBK -> Could not open specified file");

        free(title);
        free(author);
        free(filename);

        return 1;
    }

    // Get amount of bytes in file:

    fseek(f, 0, SEEK_END); // Go to the end
    maxChars = ftell(f); // The chars is the offset from the start to this point

    rewind(f); // Go back to the beginning

    // Begin output:

    printf("\nCommand to generate book:\n\n"
           COMMAND_START
           TITLE_START
           "%s"
           TITLE_END
           SEPARATOR
           AUTHOR_START
           "%s"
           AUTHOR_END
           SEPARATOR
           PAGES_START
           PAGE_START,

           title,
           author);

    for(size_t charCount = 1, pageChars = 1, charBytesRemaining = 0; charCount < maxChars + 1; charCount++, pageChars++) { // Read file byte-by-byte
        input = fgetc(f); // Get char
    
        if(charBytesRemaining) { // If this byte is a trailling byte of a multibyte UTF-8 character
            charBytesRemaining--;

            START_MULTIBYTE:

            printf("%c", input); // Output it and do nothing else
            continue;
        } else { // Otherwise, it's the start of a character (multibyte or not)
            for(size_t i = 0; i < NO_UTF8_ENCODINGS; i++) { // Find out if it matches any encodings for multibyte chars
                if((input & UTF8_ENCODINGS[i].mask) == UTF8_ENCODINGS[i].mask) {
                    charBytesRemaining = UTF8_ENCODINGS[i].no_bytes; // Set the number of trailling bytes if it does
                    break;
                }
            }

            if(charBytesRemaining) // If we're in a multibyte char, output all bytes of the char without doing anything else
                goto START_MULTIBYTE;
        }

        // If it's not a multibyte char:

        switch(input) { // Convert it to its Minecraft equivalent if need-be, or leave it alone
            case '\n':
                printf(NEWLINE);
                break;
            case '"':
                printf(DOUBLE_QUOTE);
                break;
            case '\'':
                printf(SINGLE_QUOTE);
                break;
            default:
                printf("%c", input);
        }

        if(input == ' ') { // If it's a space
            oldPos = ftell(f);

            for(wordLength = 0; !(fgetc(f) == ' ' || wordLength + 1 == maxChars); wordLength++); // Find the length of the following word until the next space

            fseek(f, oldPos, SEEK_SET); // Go back to old position before length-calculation
        }

        if((input == ' ' && pageChars + wordLength > MAX_PAGE_CHARS) || charCount == maxChars) { // If there needs to be a pagebreak
            printf(PAGE_END); // End the page

            if(charCount + 1 < maxChars) { // And if it's not the last page, print the start of a new page
                printf(SEPARATOR
                       PAGE_START);

                pageChars = 0;
            }
        }
    }

    // Finalise:

    printf("%s%s\n\n[Press enter to exit]",
           PAGES_END,
           COMMAND_END);

    // Do cleanup before admission of input to ensure it's performed in any case:

    fclose(f);

    free(title);
    free(author);
    free(filename);

    getchar(); // Wait for user to terminate program manually

    // Done!

    return 0;
}

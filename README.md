# MBK
MBK (Minecraft Book maKer) is a tool for putting books into minecraft.
Upon running the application and filling out the details about the book, a command will be generated which can be run in Minecraft in order to give the nearest player a book with those details and content.
The program will prompt you for a book title, author, and file.
The file should be a UTF-8 encoded text-document, for full Unicode support.
To build the application for your platform, all that is required is a C compiler.
The compilation of the binary in /main was performed on a Linux AMD64 system with GCC.
The command used was: gcc -Wall -Wextra -O3 mbk.c -o mbk

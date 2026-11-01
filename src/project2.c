#include "project2.h"
#include "project1.h"
#include "io.h"

#define VIDEO_MEM 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define TEXT_COLOR 0x07

// Define a keymap to convert keyboard scancodes to ASCII
static volatile char keymap[128] = {
	[0x1E] = 'a',
    [0x30] = 'b',
    [0x2E] = 'c',
    [0x20] = 'd',
	[0x12] = 'e',
	[0x21] = 'f',
	[0x22] = 'g',
	[0x23] = 'h',
	[0x17] = 'i',
	[0x24] = 'j',
	[0x25] = 'k',
	[0x26] = 'l',
	[0x32] = 'm',
	[0x31] = 'n',
	[0x18] = 'o',
	[0x19] = 'p',
	[0x10] = 'q',
	[0x13] = 'r',
	[0x1F] = 's',
	[0x14] = 't',
	[0x16] = 'u',
	[0x2F] = 'v',
	[0x11] = 'w',
	[0x2D] = 'x',
	[0x15] = 'y',
	[0x2C] = 'z',
	[0x0B] = '0',
	[0x02] = '1',
	[0x03] = '2',
	[0x04] = '3',
	[0x05] = '4',
	[0x06] = '5',
	[0x07] = '6',
	[0x08] = '7',
	[0x09] = '8',
	[0x0A] = '9',
    [0x1C] = '\n',
    [0x39] = ' ',
    [0x35] = '/',
    [0x34] = '.',
    // Added remaining ascii characters for each scan code
};

#if PROJECT == 2
// This function runs when compiled with the #define PROJECT 2 flag for make
int kernel()
{
	// Asking the user to type in stuff forever

    char buffer[100];

    while (1)
    {
        print("Please type something: ");

        scan(buffer);

        print("You entered: ");
        print(buffer);
        putchar('\n');
    }
	
	return 0;
}
#endif

// Gets the character from the keyboard
// This is a blocking function that does not use interrupts
// Only I/O ports and polling are used
char getchar()
{
    //(void)keymap; // Remove this when implementing your function
	// Convert the scancode to an ASCII character using the key map
	unsigned char status;
    unsigned char scancode;

    while (1)
    {
        status = inb(0x64);

        if (status & 0x01)
        {
            scancode = inb(0x60);

            // Only process key presses, not key releases
            if ((scancode & 0x80) == 0)
            {
                char c = keymap[scancode];

                if (c != 0)
                {
                    return c;
                }
            }
        }
    }
}

// Read characters from the keyboard until the user hits the enter key
// Accepts a character array to fill with characters
// Terminates string with NULL terminator when done
void scan(char string[])
{
	int i = 0;
    char c;

    while (1)
    {
        c = getchar();

        if (c == '\n')
        {
            putchar('\n');
            string[i] = '\0';
            return;
        }

        string[i] = c;
        i++;

        putchar(c);
    }
}

void scroll(int rows)
{

    if (rows <= 0)
    {
        return;
    }

    if (rows > SCREEN_HEIGHT)
    {
        rows = SCREEN_HEIGHT;
    }

    unsigned char *video = (unsigned char *)VIDEO_MEM;

    int bytesPerRow = SCREEN_WIDTH * 2;
    int shift = rows * bytesPerRow;
    int bytesToMove = (SCREEN_HEIGHT - rows) * bytesPerRow;

    // Move the remaining screen contents upward
    for (int i = 0; i < bytesToMove; i++)
    {
        video[i] = video[i + shift];
    }

    // Clear the empty rows at the bottom
    for (int row = SCREEN_HEIGHT - rows; row < SCREEN_HEIGHT; row++)
    {
        for (int col = 0; col < SCREEN_WIDTH; col++)
        {
            int index = 2 * (col + row * SCREEN_WIDTH);

            video[index] = ' ';
            video[index + 1] = TEXT_COLOR;
        }
    }
}
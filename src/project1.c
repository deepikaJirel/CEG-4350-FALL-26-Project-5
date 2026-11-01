#include "project1.h"
#include "project2.h"

// Define our constants that will be widely used
#define TEXT_COLOR 0x07
#define VIDEO_MEM 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// Track the current cursor's row and column
static int cursorCol = 0;
static int cursorRow = 0;

#if PROJECT == 1
// This function runs when compiled with the #define PROJECT 1 flag
int kernel()
{
	print("Hello World!\n");// Print hello world to the display
	return 0;
}
#endif

// Setting the cursor does not display anything visually
// Setting the cursor is simply used by putchar() to find where to print next
// This can also be set independently of putchar() to print at any x, y coordinate on the screen
int setcursor(int x, int y)
{
	cursorCol = x % SCREEN_WIDTH;
cursorRow = y + (x / SCREEN_WIDTH);
if(cursorRow>= SCREEN_HEIGHT){
	scroll(cursorRow - SCREEN_HEIGHT + 1);
	cursorRow = SCREEN_HEIGHT - 1 ;
}
	return 0;
}

// Using a pointer to video memory we can put characters to the display
// Every two addresses contain a character and a color
char putchar(char character)
{
if (character == '\n')
{
    setcursor(0, cursorRow + 1);
    return character;
}

uint8 *addr = (uint8 *)
    (2 * (cursorCol + cursorRow * SCREEN_WIDTH) + VIDEO_MEM);

*addr = character;
*(addr + 1) = TEXT_COLOR;

setcursor(cursorCol + 1, cursorRow);
return character;
}


// Print the character array (string) using putchar()
// Print until we find a NULL terminator (0)
int print(char string[]) 
{
    int characterCount = 0;

    while (string[characterCount] != 0)
    {
        putchar(string[characterCount]);
        characterCount++;
    }	
return characterCount;
}

// Clear the screen by placing a ' ' character in every character location
void clearscreen()
{
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
        uint8 *addr = (uint8 *)(2 * i + VIDEO_MEM);
        *addr = ' ';
        *(addr + 1) = TEXT_COLOR;
    }

    setcursor(0, 0);
    return;
	
}
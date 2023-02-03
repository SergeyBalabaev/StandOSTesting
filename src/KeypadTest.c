#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pigpio.h>
#define ROWS 4
#define COLS 3
char pressedKey = '\0';

char keys[ROWS][COLS] = { { '1', '2', '3' },
			  { '4', '5', '6' },
			  { '7', '8', '9' },
			  { '*', '0', '#' } };

int rowPins[ROWS] = { 4, 25, 11, 8 }; // R0, R1, R2, R3
int colPins[COLS] = { 7, 6, 5 }; // C0, C1, C2

void init_keypad()
{
	for (int c = 0; c < COLS; c++) {
		gpioSetMode(colPins[c], PI_INPUT);
		gpioWrite(colPins[c], 0);
	}

	for (int r = 0; r < ROWS; r++) {
		gpioSetMode(rowPins[r], PI_OUTPUT);
		gpioWrite(rowPins[r], 1);
	}
}

int findHighCol()
{
	for (int c = 0; c < COLS; c++) {
		if (gpioRead(colPins[c]) == 1)
			return c;
	}
	return -1;
}

char get_key()
{
	int colIndex;

	for (int r = 0; r < ROWS; r++) {
		gpioWrite(rowPins[r], 1);
		colIndex = findHighCol();
		if (colIndex > -1) {
			if (!pressedKey)
				pressedKey = keys[r][colIndex];
			return pressedKey;
		}
		gpioWrite(rowPins[r], 0);
	}

	pressedKey = '\0';
	return pressedKey;
}

void KeypadTest()
{
	gpioInitialise();
	init_keypad();
	for (int r = 0; r < ROWS; r++)
		for (int c = 0; c < COLS; c++) {
			printf("Нажмите символ %c\n", keys[r][c]);
			while (1) {
				char x = get_key();

				if (x) {
					printf("%c\n", x);
				}
				time_sleep(0.5);
				if (x == keys[r][c])
					break;
				fflush(stdout);
			}
		}
    printf("OK\n");
	gpioTerminate();
}
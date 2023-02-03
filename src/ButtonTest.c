#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>
#include "ButtonTest.h"

void ButtonTest()
{
	wiringPiSetup();
	pinMode(B0, INPUT);
	pinMode(B1, INPUT);
	sleep(0.5);
	if (digitalRead(B0) == 1 && digitalRead(B1) == 1) {
		printf("Зажмите кнопки BUTTON0 и BUTTON1\n");
		sleep(2);
		if (digitalRead(B0) == 0 && digitalRead(B1) == 0) {
			printf("OK!\n");
		}
	}
}
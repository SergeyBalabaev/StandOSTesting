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
#include "MicrophoneTest.h"

void MicrophoneTest()
{
	wiringPiSetup();
	pinMode(GPIO_WPI_11, INPUT);
	sleep(0.5);
    printf("Не шумите\n");
    time_t begintime = time(NULL);
    while(time(NULL) - begintime < 3)
    {
        if(digitalRead(GPIO_WPI_11) != 0)
            {
                printf("No OK\n");
                return;
            }
    }
    printf("OK\n");
    printf("Хлопните в ладоши\n");
    begintime = time(NULL);
    while(time(NULL) - begintime < 3)
    {
        if(digitalRead(GPIO_WPI_11) != 0)
            {
                printf("OK\n");
                return;
            }
    }
    printf("No OK");
}
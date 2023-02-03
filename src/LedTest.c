#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "LedTest.h"


int GPIOExport(int pin)
{
#define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		Exiting(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return (0);
}

int GPIOUnexport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;

	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		Exiting(-1);
	}

	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return (0);
}

int GPIODirection(int pin, int dir)
{
	static const char s_directions_str[] = "in\0out";

#define DIRECTION_MAX 35
	char path[DIRECTION_MAX];
	int fd;

	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing!\n");
		Exiting(-1);
	}

	if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3],
			IN == dir ? 2 : 3)) {
		fprintf(stderr, "Failed to set direction!\n");
		Exiting(-1);
	}

	close(fd);
	return (0);
}

int GPIOWrite(int pin, int value)
{
	static const char s_values_str[] = "01";

	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing!\n");
		Exiting(-1);
	}

	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		Exiting(-1);
	}

	close(fd);
	return (0);
}

void Exiting(int parameter)
{
	GPIOUnexport(LEDR);
	GPIOUnexport(LEDY);
	GPIOUnexport(LEDG);
	exit(parameter);
}

void RedBlink(int delay){
    GPIOWrite(LEDR, 1);
	GPIOWrite(LEDY, 0);
	GPIOWrite(LEDG, 0);
	printf("Light:R\n");
	fflush(stdout);
	usleep(delay);
    GPIOWrite(LEDR, 0);
	GPIOWrite(LEDY, 0);
	GPIOWrite(LEDG, 0);
}

void YellowBlink(int delay){
    GPIOWrite(LEDR, 0);
	GPIOWrite(LEDY, 1);
	GPIOWrite(LEDG, 0);
	printf("Light:Y\n");
	fflush(stdout);
	usleep(delay);
    GPIOWrite(LEDR, 0);
	GPIOWrite(LEDY, 0);
	GPIOWrite(LEDG, 0);
}

void GreenBlink(int delay){
    GPIOWrite(LEDR, 0);
	GPIOWrite(LEDY, 0);
	GPIOWrite(LEDG, 1);
	printf("Light:G\n");
	fflush(stdout);
	usleep(delay);
    GPIOWrite(LEDR, 0);
	GPIOWrite(LEDY, 0);
	GPIOWrite(LEDG, 0);
}

void LedTest()
{
	GPIOExport(LEDR);
	GPIOExport(LEDY);
	GPIOExport(LEDG);
	GPIODirection(LEDR, OUT);
	GPIODirection(LEDY, OUT);
	GPIODirection(LEDG, OUT);

	sleep(0.5);
    RedBlink(DELAY_TIME);
    YellowBlink(DELAY_TIME);
    GreenBlink(DELAY_TIME);

    GPIOUnexport(LEDR);
	GPIOUnexport(LEDY);
	GPIOUnexport(LEDG);
}
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

//int TRIG = 8; // GPIO PIN TRIG
//int ECHO = 11; // GPIO PIN ECHO

void Exiting(int);

static int GPIOExport(int pin)
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

static int GPIOUnexport(int pin)
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

static int GPIODirection(int pin, int dir)
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

static int GPIORead(int pin)
{
#define VALUE_MAX 30
	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for reading!\n");
		Exiting(-1);
	}

	if (-1 == read(fd, value_str, 3)) {
		fprintf(stderr, "Failed to read value!\n");
		Exiting(-1);
	}

	close(fd);

	return (atoi(value_str));
}

static int GPIOWrite(int pin, int value)
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

#define TIMEOUT_SEC 2

void RangefinderExport(int TRIG, int ECHO)
{
	GPIOExport(TRIG);
	GPIOExport(ECHO);
	sleep(0.05);
	GPIODirection(TRIG, OUT);
	GPIODirection(ECHO, IN);
}

void RangefinderWorker(int TRIG, int ECHO)
{
	double search_time = 0;
	while (1) {
		GPIOWrite(TRIG, 1);
		usleep(10);
		GPIOWrite(TRIG, 0);
		while (!GPIORead(ECHO)) {
		}
		double start_time = clock();
		int flag = 0;
		while (GPIORead(ECHO)) {
			if (clock() - start_time >
			    TIMEOUT_SEC * CLOCKS_PER_SEC) {
				flag = 1;
				break;
			}
		}
		if (flag) {
			printf("Timeout reached, sleeping for one second\n");
			sleep(1);
			continue;
		}
		double end_time = clock();
		search_time = (end_time - start_time) / CLOCKS_PER_SEC;

		fflush(stdout);
		double L = search_time * 340 / 2 * 100;
		printf("%lf\n", L);
		usleep(100 * 1000);
		if (L < 26 && L > 25)
			break;
	}
}

void RangefinderHcsr04Test()
{

    int TRIG = 8;
    int ECHO = 11;
	RangefinderExport(TRIG, ECHO);
    printf("Джампер в положении Ind\n");
    printf("Расположите преграду на расстоянии 25 см от дальномера H0\n");
	RangefinderWorker(TRIG, ECHO);
	printf("OK\n");
	GPIOUnexport(TRIG);
	GPIOUnexport(ECHO);
    
    TRIG = 26;
    ECHO = 27;
	RangefinderExport(TRIG, ECHO);
    printf("Расположите преграду на расстоянии 25 см от дальномера H1\n");
	RangefinderWorker(TRIG, ECHO);
	printf("OK\n");
	GPIOUnexport(TRIG);
	GPIOUnexport(ECHO);

    printf("Джампер в положении Sync\n");
    TRIG = 8;
    int ECHO_1 = 11;
    int ECHO_2 = 27;
    GPIOExport(TRIG);
	GPIOExport(ECHO_1);
    GPIOExport(ECHO_2);
	sleep(0.05);
	GPIODirection(TRIG, OUT);
	GPIODirection(ECHO_1, IN);
    GPIODirection(ECHO_2, IN);
    printf("Расположите преграду на расстоянии 25 см от дальномера H0\n");
    RangefinderWorker(TRIG, ECHO_1);
	printf("OK\n");
    printf("Расположите преграду на расстоянии 25 см от дальномера H1\n");
    RangefinderWorker(TRIG, ECHO_2);
    printf("OK\n");
    GPIOUnexport(TRIG);
	GPIOUnexport(ECHO_1);
    GPIOUnexport(ECHO_2);

}

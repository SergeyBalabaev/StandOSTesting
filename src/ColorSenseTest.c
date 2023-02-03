#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "LedTest.h"

#define SLEEP_TIME 100

typedef struct ColorSenseData {
	int cData;
	int red;
	int green;
	int blue;
	float luminance;
} ColorSenseData;

void ColorSenseConfig(int *file)
{
	char *bus = "/dev/i2c-1";
	if ((*file = open(bus, O_RDWR)) < 0) {
		printf("Failed to open the bus. \n");
		exit(1);
	}
	ioctl(*file, I2C_SLAVE, 0x29);
	char config[2] = { 0 };
	config[0] = 0x80;
	config[1] = 0x03;
	write(*file, config, 2);
	config[0] = 0x81;
	config[1] = 0x00;
	write(*file, config, 2);
	config[0] = 0x83;
	config[1] = 0xFF;
	write(*file, config, 2);
	config[0] = 0x8F;
	config[1] = 0x00;
	write(*file, config, 2);
}

ColorSenseData getColorSenseData(int *file)
{
	ColorSenseData tempColorSenseData;
	char reg[1] = { 0x94 };
	write(*file, reg, 1);
	char data[8] = { 0 };
	if (read(*file, data, 8) != 8) {
		printf("Erorr : Input/output Erorr \n");
	} else {
		tempColorSenseData.cData = (data[1] * 256 + data[0]);
		tempColorSenseData.red = (data[3] * 256 + data[2]);
		tempColorSenseData.green = (data[5] * 256 + data[4]);
		tempColorSenseData.blue = (data[7] * 256 + data[6]);
		tempColorSenseData.luminance =
			(-0.32466) * (tempColorSenseData.red) +
			(1.57837) * (tempColorSenseData.green) +
			(-0.73191) * (tempColorSenseData.blue);
		if (tempColorSenseData.luminance < 0) {
			tempColorSenseData.luminance = 0;
		}
	}
	return tempColorSenseData;
}

void ColorSensePrintData()
{
	int file;
	ColorSenseConfig(&file);
	sleep(1);
	ColorSenseData ColorSenseDataParametr;
	ColorSenseDataParametr = getColorSenseData(&file);
	printf("RGB: %d %d %d\n", ColorSenseDataParametr.red,
	       ColorSenseDataParametr.green, ColorSenseDataParametr.blue);
	printf("IR luminance : %d lux \n", ColorSenseDataParametr.cData);
	printf("Ambient Light Luminance : %.2f lux \n",
	       ColorSenseDataParametr.luminance);
	fflush(stdout);
	usleep(SLEEP_TIME * 1000);
}

void ColorSenseTest()
{
	int file;
	ColorSenseData ColorSenseDataParametr;
	ColorSenseConfig(&file);
	GPIOExport(LEDR);
	GPIOExport(LEDY);
	GPIOExport(LEDG);
	GPIODirection(LEDR, OUT);
	GPIODirection(LEDY, OUT);
	GPIODirection(LEDG, OUT);
	sleep(1);
	GPIOWrite(LEDR, 1);
	GPIOWrite(LEDY, 0);
	GPIOWrite(LEDG, 0);
	sleep(1);
	ColorSenseDataParametr = getColorSenseData(&file);
	sleep(1);
	if (ColorSenseDataParametr.red > ColorSenseDataParametr.green &&
	    ColorSenseDataParametr.red > ColorSenseDataParametr.blue)
		printf("OK!");
	GPIOWrite(LEDR, 0);
	GPIOWrite(LEDY, 0);
	GPIOWrite(LEDG, 0);
	GPIOUnexport(LEDR);
	GPIOUnexport(LEDY);
	GPIOUnexport(LEDG);
}

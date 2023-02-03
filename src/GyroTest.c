#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

// минимальное значение скорости поворота
#define MIN_VAL 500

// значение чувствительности из datasheet
#define SENSITIVITY_250 8.75f
#define SENSITIVITY_500 17.5f
#define SENSITIVITY_2000 70.0f

// значения чувствительности * калибровочный коэффициент
#define MAGIC_CONST_X (SENSITIVITY_2000 * 2)
#define MAGIC_CONST_Y (SENSITIVITY_2000 * 2)
#define MAGIC_CONST_Z (SENSITIVITY_2000 * 2)

// адрес шины i2c
#define I2C "/dev/i2c-1"

// время опроса (ms)
#define TIME 10.0

#define N_iterations 100

uint8_t xSign, ySign, zSign;
float xPosition = 0;
float yPosition = 0;
float zPosition = 0;

float askGiro(int file, char pos, double time)
{
	char reg[1];
	char data[1];
	char data_0, data_1;
	if (pos == 'X') {
		reg[0] = 0x28;
		write(file, reg, 1);
		read(file, data, 1);
		data_0 = data[0];
		reg[0] = 0x29;
		write(file, reg, 1);
		read(file, data, 1);
		data_1 = data[0];
		int16_t xGyro = (data_1 << 8 | data_0);
		if (xGyro < MIN_VAL && xGyro > -MIN_VAL)
			xGyro = 0;
		if (xGyro > 32767)
			xGyro -= 65536;
		if ((xGyro & 0x8000) == 0)
			xSign = 0;
		else {
			xSign = 1;
			xGyro &= 0x7FFF;
			xGyro = 0x8000 - xGyro;
		}
		if (xSign == 0)
			xPosition +=
				MAGIC_CONST_X * xGyro * (time / 1000) / 1000;
		else
			xPosition -=
				MAGIC_CONST_X * xGyro * (time / 1000) / 1000;
		return xPosition;
	}

	if (pos == 'Y') {
		reg[0] = 0x2A;
		write(file, reg, 1);
		read(file, data, 1);
		data_0 = data[0];
		reg[0] = 0x2B;
		write(file, reg, 1);
		read(file, data, 1);
		data_1 = data[0];
		int16_t yGyro = (data_1 << 8 | data_0);
		if (yGyro < MIN_VAL && yGyro > -MIN_VAL)
			yGyro = 0;
		if (yGyro > 32767)
			yGyro -= 65536;
		if ((yGyro & 0x8000) == 0)
			ySign = 0;
		else {
			ySign = 1;
			yGyro &= 0x7FFF;
			yGyro = 0x8000 - yGyro;
		}
		if (ySign == 0)
			yPosition +=
				MAGIC_CONST_Y * yGyro * (time / 1000) / 1000;
		else
			yPosition -=
				MAGIC_CONST_Y * yGyro * (time / 1000) / 1000;
		return yPosition;
	}

	if (pos == 'Z') {
		reg[0] = 0x2C;
		write(file, reg, 1);
		read(file, data, 1);
		char data_0 = data[0];
		reg[0] = 0x2D;
		write(file, reg, 1);
		read(file, data, 1);
		char data_1 = data[0];
		int16_t zGyro = (data_1 << 8 | data_0);
		if (zGyro < MIN_VAL && zGyro > -MIN_VAL)
			zGyro = 0;
		if (zGyro > 32767)
			zGyro -= 65536;
		if ((zGyro & 0x8000) == 0)
			zSign = 0;
		else {
			zSign = 1;
			zGyro &= 0x7FFF;
			zGyro = 0x8000 - zGyro;
		}
		if (zSign == 0)
			zPosition +=
				MAGIC_CONST_Z * zGyro * (time / 1000) / 1000;
		else
			zPosition -=
				MAGIC_CONST_Z * zGyro * (time / 1000) / 1000;
		return zPosition;
	}
}

void GyroConfig(int *file)
{
	char *bus = I2C;
	if ((*file = open(bus, O_RDWR)) < 0) {
		printf("Failed to open the bus. \n");
		exit(1);
	}
	ioctl(*file, I2C_SLAVE, 0x69);
	char config[2] = { 0 };
	config[0] = 0x20;
	config[1] = 0x0F;
	write(*file, config, 2);
	config[0] = 0x23;
	config[1] = 0x30; // 2000 dps
	write(*file, config, 2);
	sleep(1);
}
void PrintData(float X, float Y, float Z)
{
	printf("X: %f\n Y: %f\n Z: %f\n", X, Y, Z);
}

void GyroTest()
{
	int file;
	GyroConfig(&file);
	float X = 0, Y = 0, Z = 0;
	printf("Поверните гироскоп на 90 градусов по одной из осей\n");
	for (size_t i = 0; i < N_iterations; ++i) {
		X = askGiro(file, 'X', TIME);
		Y = askGiro(file, 'Y', TIME);
		Z = askGiro(file, 'Z', TIME);
		PrintData(X, Y, Z);
		fflush(stdout);
		usleep(TIME * 1000);
	}

	if (abs(X) > 80 && abs(X) < 100)
		printf("X: OK\n");
	if (abs(Y) > 80 && abs(Y) < 100)
		printf("Y: OK\n");
	if (abs(Z) > 80 && abs(Z) < 100)
		printf("Z: OK\n");
}

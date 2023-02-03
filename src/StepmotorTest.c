#include <stdio.h>
#include <bcm2835.h>
#include <stdlib.h>
#include <string.h>
#define STEP_DELAY 1

//***************************//
#define Pin1 8
#define Pin2 11
#define Pin3 26
#define Pin4 27
//***************************//

int step = 0;

void loop()
{
	switch (step) {
	case 0:
		bcm2835_gpio_write(Pin1, LOW);
		bcm2835_gpio_write(Pin2, LOW);
		bcm2835_gpio_write(Pin3, LOW);
		bcm2835_gpio_write(Pin4, HIGH);
		break;
	case 1:
		bcm2835_gpio_write(Pin1, LOW);
		bcm2835_gpio_write(Pin2, LOW);
		bcm2835_gpio_write(Pin3, HIGH);
		bcm2835_gpio_write(Pin4, HIGH);
		break;
	case 2:
		bcm2835_gpio_write(Pin1, LOW);
		bcm2835_gpio_write(Pin2, LOW);
		bcm2835_gpio_write(Pin3, HIGH);
		bcm2835_gpio_write(Pin4, LOW);
		break;
	case 3:
		bcm2835_gpio_write(Pin1, LOW);
		bcm2835_gpio_write(Pin2, HIGH);
		bcm2835_gpio_write(Pin3, HIGH);
		bcm2835_gpio_write(Pin4, LOW);
		break;
	case 4:
		bcm2835_gpio_write(Pin1, LOW);
		bcm2835_gpio_write(Pin2, HIGH);
		bcm2835_gpio_write(Pin3, LOW);
		bcm2835_gpio_write(Pin4, LOW);
		break;
	case 5:
		bcm2835_gpio_write(Pin1, HIGH);
		bcm2835_gpio_write(Pin2, HIGH);
		bcm2835_gpio_write(Pin3, LOW);
		bcm2835_gpio_write(Pin4, LOW);
		break;
	case 6:
		bcm2835_gpio_write(Pin1, HIGH);
		bcm2835_gpio_write(Pin2, LOW);
		bcm2835_gpio_write(Pin3, LOW);
		bcm2835_gpio_write(Pin4, LOW);
		break;
	case 7:
		bcm2835_gpio_write(Pin1, HIGH);
		bcm2835_gpio_write(Pin2, LOW);
		bcm2835_gpio_write(Pin3, LOW);
		bcm2835_gpio_write(Pin4, HIGH);
		break;
	default:
		bcm2835_gpio_write(Pin1, LOW);
		bcm2835_gpio_write(Pin2, LOW);
		bcm2835_gpio_write(Pin3, LOW);
		bcm2835_gpio_write(Pin4, LOW);
		break;
	}
}

void StepmotorTest()
{
	int i, rotate_dir, angle;
	int step_delay = STEP_DELAY;
	angle = 360;

	if (!bcm2835_init())
		return;

	bcm2835_gpio_fsel(Pin1, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(Pin2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(Pin3, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(Pin4, BCM2835_GPIO_FSEL_OUTP);

	int intang = abs(angle) * 11.377;
	if (rotate_dir)
		step = 7;
	for (i = 0; i < intang; i++)
		if (rotate_dir) {
			loop();
			step--;
			if (step < 0)
				step = 7;
			bcm2835_delay(step_delay);
		} else {
			loop();
			step++;
			if (step > 7)
				step = 0;
			bcm2835_delay(step_delay);
		}
	bcm2835_close();
	printf("OK\n");
}
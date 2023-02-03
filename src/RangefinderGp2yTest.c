#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <ads1115.h>
#include <math.h>

#define AD_BASE 122
#define AD_ADDR 0x48 
#define ADC_PIN 2

void RangefinderGp2yTest()
{
	wiringPiSetup();
	ads1115Setup(AD_BASE, AD_ADDR);
	digitalWrite(AD_BASE, 0);
    printf("Расположите преграду на расстоянии 25 см от дальномера\n");
	while (1) {
		int ADC_VAL = analogRead(AD_BASE + ADC_PIN);
        double L = 61.3899*pow(1000/(ADC_VAL*0.1875),1.1076);
        printf("%lf\n", L);
		fflush(stdout);
		usleep(1000 * 100);
        if(L < 26 && L > 25)
            break;
	}
    printf("OK\n");
}
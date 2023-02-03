#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <ads1115.h>

#define AD_BASE 122
#define AD_ADDR 0x48 //i2c address
#define ADC_PIN 1
#define DELAY_MS 1000
#define NumOfCounts 10
void LightDetectTest()
{
	int ADC_VAL, ADC_VAL_SUM[2] = {0}, CURRENT_VAL = 0;
	wiringPiSetup();
	ads1115Setup(AD_BASE, AD_ADDR);
	digitalWrite(AD_BASE, 0);
	int CurrentCount = 0;
	for (size_t CurrentCount = 0; CurrentCount < NumOfCounts;
	     CurrentCount++) {
		if (CurrentCount == 5){
            CURRENT_VAL++;
			printf("Закройте фотодатчик\n");
            sleep(1);
        }
		ADC_VAL = analogRead(AD_BASE + ADC_PIN);
        ADC_VAL_SUM[CURRENT_VAL] += ADC_VAL;
		printf("ADC: %d \n", ADC_VAL);
		fflush(stdout);
		usleep(1000 * DELAY_MS);
	}
    printf("ADC[0]: %d, ADC[1]: %d \n", ADC_VAL_SUM[0]/5, ADC_VAL_SUM[1]/5);
    if(ADC_VAL_SUM[0]/5 > ADC_VAL_SUM[1]/5)
        printf("OK\n");
}
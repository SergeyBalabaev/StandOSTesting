#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pigpio.h>
#include <stdlib.h>

#define GPIO_PIN_A 8
#define GPIO_PIN_B 11

static int GlobalVar = 0;

typedef void (*Pi_Renc_CB_t)(int);

typedef struct _Pi_Renc_s
{
   int gpioA;
   int gpioB;
   Pi_Renc_CB_t callback;
   int levA;
   int levB;
   int lastGpio;
} Pi_Renc_t;

static void _cb(int gpio, int level, uint32_t tick, void *user)
{
   Pi_Renc_t *renc;
   renc = user;
   if (gpio == renc->gpioA)
      renc->levA = level;
   else
      renc->levB = level;
   if (gpio != renc->lastGpio) /* debounce */
   {
      renc->lastGpio = gpio;
      if ((gpio == renc->gpioA) && (level == 1))
      {
         if (renc->levB)
            (renc->callback)(1);
      }
      else if ((gpio == renc->gpioB) && (level == 1))
      {
         if (renc->levA)
            (renc->callback)(-1);
      }
   }
}

Pi_Renc_t *Pi_Renc(int gpioA, int gpioB, Pi_Renc_CB_t callback)
{
   Pi_Renc_t *renc;
   renc = malloc(sizeof(Pi_Renc_t));
   renc->gpioA = gpioA;
   renc->gpioB = gpioB;
   renc->callback = callback;
   renc->levA = 0;
   renc->levB = 0;
   renc->lastGpio = -1;
   gpioSetMode(gpioA, PI_INPUT);
   gpioSetMode(gpioB, PI_INPUT);
   gpioSetPullUpDown(gpioA, PI_PUD_UP);
   gpioSetPullUpDown(gpioB, PI_PUD_UP);
   gpioSetAlertFuncEx(gpioA, _cb, renc);
   gpioSetAlertFuncEx(gpioB, _cb, renc);
   return renc;
}

void Pi_Renc_cancel(Pi_Renc_t *renc)
{
   if (renc)
   {
      gpioSetAlertFunc(renc->gpioA, 0);
      gpioSetAlertFunc(renc->gpioB, 0);
      free(renc);
   }
}

void callback(int way)
{
	static int pos = 0;
	pos += way*360/20;
	printf("%d\n", pos);
    GlobalVar = pos;
	fflush(stdout);
}

void EncoderTest()
{
	Pi_Renc_t *renc;
	if (gpioInitialise() < 0){
        printf("Error in gpioInitialise!\n");
    }
	renc = Pi_Renc(GPIO_PIN_A, GPIO_PIN_B, callback);
    printf("Поверните энкодер на 360 градусов по часовой стрелке\n");
    while(GlobalVar < 360){}
	printf("OK\n");
    Pi_Renc_cancel(renc);
	gpioTerminate();
}

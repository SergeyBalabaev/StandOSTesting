####### Files
SOURCES = 	./obj/ButtonTest.o \
			./obj/MicrophoneTest.o \
			./obj/LedTest.o \
			./obj/ColorSenseTest.o \
			./obj/LightDetectTest.o \
			./obj/GyroTest.o \
			./obj/EncoderTest.o \
			./obj/KeypadTest.o \
			./obj/RangefinderGp2yTest.o \
			./obj/RangefinderHcsr04Test.o \
			./obj/StepmotorTest.o \
			./obj/DisplayTest.o \
			./obj/Test.o

all:
	gcc -c Test.c -o ./obj/Test.o
	gcc -c ./src/LedTest.c -o ./obj/LedTest.o
	gcc -c ./src/ButtonTest.c -o ./obj/ButtonTest.o
	gcc -c ./src/ColorSenseTest.c -o ./obj/ColorSenseTest.o
	gcc -c ./src/LightDetectTest.c -o ./obj/LightDetectTest.o
	gcc -c ./src/GyroTest.c -o ./obj/GyroTest.o 
	gcc -c ./src/MicrophoneTest.c -o ./obj/MicrophoneTest.o 
	gcc -c ./src/EncoderTest.c -o ./obj/EncoderTest.o
	gcc -c ./src/KeypadTest.c -o ./obj/KeypadTest.o
	gcc -c ./src/RangefinderGp2yTest.c -o ./obj/RangefinderGp2yTest.o
	gcc -c ./src/RangefinderHcsr04Test.c -o ./obj/RangefinderHcsr04Test.o
	gcc -c ./src/StepmotorTest.c -o ./obj/StepmotorTest.o
	gcc -c ./src/DisplayTest.c -o ./obj/DisplayTest.o
	gcc $(SOURCES) -o Test -lwiringPi -lpigpio -lm -lbcm2835 -lgd -lfreetype
OBJS = main.o l3gd20_wiringPi.o adxl345_wiringPi.o
OBJS2 = main2.o l3gd20_wiringPi.o adxl345_wiringPi.o
OBJS3 = l3gd20_wiringPi.o adxl345_wiringPi.o
OBJS4 = serverIOT.o iotModule.o l3gd20_wiringPi.o adxl345_wiringPi.o
CC = gcc
CC2 = g++
LDFLAGS = -lwiringPi
OPENCV = -I/usr/local/include/opencv -lpthread -ldl -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video -lopencv_contrib -lopencv_legacy -lm


main2:$(OBJS2)
	$(CC) -o $@ $(OBJS2) $(LDFLAGS)

main:$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

serverIOT:$(OBJS4)
	$(CC) -pthread -o $@ $(OBJS4) $(LDFLAGS) $(OPENCV)

serverIOT.o:serverIOT.c
	$(CC) -c $< ${OPENCV}

iotModule.o:iotModule.c iotModule.h
	$(CC) -c $<

gyroSave:gyroSave.o l3gd20_wiringPi.o adxl345_wiringPi.o
	$(CC) -o $@ $< $(OBJS3) $(LDFLAGS)

gyroSave.o:gyroSave.c
	$(CC) -c $<

main2.o:main2.c
	$(CC) -c $<

main.o:main.c
	$(CC) -c $<

l3gd20_wiringPi.o:l3gd20_wiringPi.c l3gd20_wiringPi.h
	$(CC) -c $<

adxl345_wiringPi.o:adxl345_wiringPi.c adxl345_wiringPi.h
	$(CC) -c $<

motorTest:motorTest.c
	$(CC) -o $@ $< $(LDFLAGS)

clean:
	$(RM) $(OBJS) main

clean2:
	$(RM) $(OBJS2) main2

clean4:
	$(RM) $(OBJS4) serverIOT

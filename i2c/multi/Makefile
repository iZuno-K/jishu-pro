OBJS = main.o l3gd20.o adxl345.o i2c.o
OBJS2 = serverIO.o ioModules.o l3gd20.o adxl345.o i2c.o
CC = gcc
CC2 = g++
LDFLAGS = -lwiringPi
OPENCV = -I/usr/local/include/opencv -lpthread -ldl -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video -lopencv_contrib -lopencv_legacy -lm

main:$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

serverIO:$(OBJS2)
	$(CC) -pthread -o $@ $(OBJS) $(LDFLAGS) $(OPENCV)

main.o:main.c
	$(CC) -c $<

serverIO.o:serverIO.c
	$(CC) -c $< ${OPENCV}

ioModules.o:ioModules.c ioModules.h
	$(CC) -c $<

l3gd20.o:l3gd20.c l3gd20.h
	$(CC) -c $<

adxl345.o:adxl345.c adxl345.h
	$(CC) -c $<

i2c.o:i2c.c i2c.h
	$(CC) -c $<

gyroSave:gyroSave.o l3gd20.o adxl345.o i2c.o
	$(CC) -o $@ gyroSave.o l3gd20.o adxl345.o i2c.o $(LDFLAGS)

gyroSave.o:gyroSave.c
	$(CC) -c $<


clean:
	$(RM) $(OBJS)

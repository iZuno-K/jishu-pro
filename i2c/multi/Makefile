OBJS = main.o l3gd20.o adxl345.o i2c.o
CC = gcc
LDFLAGS = -lwiringPi

main:$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

main.o:main.c
	$(CC) -c $<

l3gd20.o:l3gd20.c l3gd20.h
	$(CC) -c $<

adxl345.o:adxl345.c adxl345.h
	$(CC) -c $<

i2c.o:i2c.c i2c.h
	$(CC) -c $<

clean:
	$(RM) $(OBJS)

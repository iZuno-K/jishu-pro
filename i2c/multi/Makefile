OBJS = main.o l3gd20.o adxl345.o i2c.o
CC = gcc

main:$(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	$(RM) $(OBJS)
	

CC = gcc
CFLAGS = -I. -lpthread -g
OBJ = main.o include/process.o include/helper.o
run: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
clean:
	rm -rf include/*.o
	rm -rf ./*.o
test: $(OBJ)
	gcc -c test.c
	gcc -o test test.o include/helper.o $(CFLAGS)
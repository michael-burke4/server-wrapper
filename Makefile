CC = clang
CFLAGS = -Weverything -std=c99
BIN = controller

$(BIN): main.o
	$(CC) $< -o $@ $(CFLAGS)
	
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
clean:
	rm *.o $(BIN)

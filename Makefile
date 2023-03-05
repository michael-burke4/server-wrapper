CC = clang
CFLAGS = -Weverything -std=c99
BIN = controller

$(BIN): main.o
	$(CC) $< -o $@ $(CFLAGS)
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
format:
	clang-format -i -style=file *.[ch]
clean:
	rm *.o $(BIN)

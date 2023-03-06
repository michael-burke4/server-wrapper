CC = clang
CFLAGS = -Weverything -std=c99
BIN = controller

$(BIN): main.o mcserver.o
	$(CC) $^ -o $@
%.o: %.[ch]
	$(CC) -c $< -o $@ $(CFLAGS)
format:
	clang-format -i -style=file *.[ch]
clean:
	rm *.o $(BIN)

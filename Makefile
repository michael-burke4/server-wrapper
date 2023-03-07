CC = clang
CFLAGS = -Weverything
BIN = controller

$(BIN): main.o mcserver.o discbot.o
	$(CC) $^ -o $@
%.o: %.c *.h
	$(CC) -c $< -o $@ $(CFLAGS)
format:
	clang-format -i -style=file *.[ch]
clean:
	rm *.o $(BIN)

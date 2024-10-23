CC = g++
FLAGS = -Wall -Wextra -Werror -lncurses

TARGET = snake

$(TARGET):
	$(CC) ./src/main.cpp $(FLAGS) -o $(TARGET)
	./snake

clean:
	rm $(TARGET)

.PHONY: $(TARGET)
OBJ_PATH = objects
SRC_PATH = source

SRCS = $(wildcard $(SRC_PATH)/*.cpp)

OBJS = $(patsubst %.cpp, $(OBJ_PATH)/%.o, $(notdir $(SRCS)))

CC = g++ -Wall -pedantic -std=c++11 -ggdb -DDEBUG

all: mts-app

mts-app: $(OBJS)
	$(CC) $^ -o $@

$(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp
	$(CC) -o $@ -c $<

clean:
	-rm -f $(OBJ_PATH)/*.o mts-app

run:
	./mts-app

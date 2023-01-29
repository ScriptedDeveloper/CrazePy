.PHONY: all

override CC	:= g++

all : crazepy

CFLAGS		+= -g
CFLAGS		+= -Wall 
CFLAGS 		+= -Werror
CFLAGS		+= -Wall 
CFLAGS		+= -Wextra 
CFLAGS		+= -Wshadow 
CFLAGS		+= -Wpedantic 
CFLAGS		+= -Wconversion
CFLAGS		+= -Weffc++
CFLAGS		+= -std=c++17
#CFLAGS		+= -O3 For production only

SRC		+= $(shell find ./* -type f -name '*.cpp')
OBJ		:= $(SRC:.cpp=.o)


crazepy : $(OBJ)
	$(CC) $(CFLAGS) $(SRC) -o crazepy

$(OBJ): $(SRC)
	$(CC) -c $< $(CFLAGS) -o $@

$(SRC) :
	clang-format -style=file $@ -i

clean :
	rm -rf crazepy
	rm -rf *.o

override CC	:= g++

CFLAGS		+= -g
CFLAGS		+= -Wall 
CFLAGS 		+= -Werror
CFLAGS		+= -Wall 
CFLAGS		+= -Wextra 
CFLAGS		+= -Wshadow 
CFLAGS		+= -Wpedantic 
CFLAGS		+= -Wconversion
CFLAGS		+= -Weffc++
#CFLAGS		+= -O3 For production only

SRC		+= $(shell find ./* -type f -name '*.cpp')
OBJ		:= $(SRC:.cpp=.o)

all : crazepy

crazepy : $(OBJ)
	$(CC) $(CFLAGS) $(SRC) -o crazepy


clean :
	rm -rf crazepy
	rm -rf *.o

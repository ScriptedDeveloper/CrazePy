override CC	:= g++

CFLAGS		+= -g
CFLAGS		+= -O3

SRC		+= $(shell find ./* -type f -name '*.cpp')
OBJ		:= $(SRC:.cpp=.o)

all : crazepy

crazepy : $(OBJ)
	$(CC) $(CFLAGS) $(SRC) -o crazepy
	rm -rf *.o


clean :
	rm -rf crazepy

override CC	:= g++

CFLAGS		+= -g

SRC		+= $(shell find ./* -type f -name '*.cpp')
OBJ		:= $(SRC:.cpp=.o)

all : crazepy

crazepy : $(OBJ)
	$(CC) $(CFLAGS) $(SRC) -o crazepy


clean :
	rm -rf crazepy
	rm -rf *.o

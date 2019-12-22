CXX = gcc
CXXFLAGS = -Wall -Werror -Wextra -pedantic -std=c99 -g -fsanitize=address
LDFLAGS =  -fsanitize=address
LBLIBS = -lSDL2

SRC = main.c
OBJ = $(SRC:.cc=.o)
EXEC = Simulator

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $(OBJ) $(LBLIBS)

clean:
	rm -rf $(EXEC)
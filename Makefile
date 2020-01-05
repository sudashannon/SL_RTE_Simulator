# 指令编译器和选项
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -g -fsanitize=address
LDFLAGS = -fsanitize=address
LBLIBS = -lSDL2 -lpthread

# 目标文件
TARGET = Simulator
SRCS = 	main.c \
		SL_RTE/source/* \
		SL_RTE/utils/cmsis_os2/source/*

INCS = 	-I./SL_RTE/include \
		-I./SL_BSP/include \
		-I./SL_RTE/utils/cmsis_os2/include

OBJS = $(SRCS:.cc=.o)


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCS) -o $@ $(OBJS) $(LBLIBS)

clean:
	rm -rf $(TARGET)
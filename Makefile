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
include SL_RTE/utils/lvgl/src/lv_core/lv_core.mk
include SL_RTE/utils/lvgl/src/lv_hal/lv_hal.mk
include SL_RTE/utils/lvgl/src/lv_objx/lv_objx.mk
include SL_RTE/utils/lvgl/src/lv_font/lv_font.mk
include SL_RTE/utils/lvgl/src/lv_misc/lv_misc.mk
include SL_RTE/utils/lvgl/src/lv_themes/lv_themes.mk
include SL_RTE/utils/lvgl/src/lv_draw/lv_draw.mk

INCS = 	-I./SL_RTE/include \
		-I./SL_BSP/include \
		-I./SL_RTE/utils/cmsis_os2/include \
		-I./SL_RTE/utils/lvgl/

OBJS = $(SRCS:.cc=.o)


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCS) -o $@ $(OBJS) $(LBLIBS)

clean:
	rm -rf $(TARGET)
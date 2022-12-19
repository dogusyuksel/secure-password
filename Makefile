
CC = gcc
RM = rm -rf
CFLAGS = -Wall -Wextra -Werror -g3 -O0
TARGET_EXECUTABLE_NAME=easy-md

SUBDIRS = 

LIBS :=

C_SRCS = \
main.c

OBJS += \
./main.o

.PHONY: subdirs all clean

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

all:$(OBJS)
	$(CC) -o ./$(TARGET_EXECUTABLE_NAME) $(C_SRCS) $(CFLAGS) $(LIBS)
	@echo ' Finish all'

clean:
	$(RM) $(OBJS) ./$(TARGET_EXECUTABLE_NAME)

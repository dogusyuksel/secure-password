include Config

CC = gcc
RM = rm -rf
CFLAGS = -Wall -Wextra -g3 -O0 -Wno-format-truncation
TARGET_EXECUTABLE_NAME=spwd

ifeq (${OPEN_DEBUG},y)
CFLAGS += -DDEBUG_OPEN
endif

SUBDIRS = 

LIBS := -lssl -lcrypto

C_SRCS = \
aes.c    \
main.c

OBJS += \
./aes.o \
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


CC = gcc
CFLAGS = -pthread -ldl
INCLUDE = -I ./include


SRCDIRS1 := ./src1
SRCS1 := $(foreach dir,$(SRCDIRS1),$(wildcard $(dir)/*.c))
OBJS1 := $(SRCS1:.c=.o)

SRCDIRS2 := ./src2
SRCS2 := $(foreach dir,$(SRCDIRS2),$(wildcard $(dir)/*.c))
OBJS2 := $(SRCS2:.c=.o)

TARGET1 := s
TARGET2 := c
all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJS1)
	$(CC) $(OBJS1) -o $(TARGET1) $(CFLAGS) $(INCLUDE)

$(TARGET2): $(OBJS2)
	$(CC) $(OBJS2) -o $(TARGET2) $(INCLUDE)


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clear:
	rm -f $(OBJS1) $(TARGET1) $(OBJS2) $(TARGET2)



CC = gcc
CFLAGS = -Wall -O2
AR = ar
ARFLAGS = rcs
TARGET = libUdpImageSender.a

SRCS = UDP_img.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f $(OBJS) $(TARGET)


FFMPEG=/c/libs/ffmpeg-20160828-a37e6dd-win64-dev

CC=gcc
RM=rm -rf

CFLAGS=-g -I$(FFMPEG)/include/
LDFLAGS=-g
LDLIBS=-L$(FFMPEG)/lib/ -lavutil -lavcodec -lswscale

MAIN_SRCS=decoding_encoding.c demuxing_decoding.c remuxing.c transcoding.c
MAIN_OBJS=$(subst .c,.o,$(MAIN_SRCS))

%.o: %.c
	$(CC) $(CFLAGS) -c $<

all: decoding_encoding.exe

decoding_encoding.exe: decoding_encoding.o
	$(CC) $(LDFLAGS) -o decoding_encoding.exe decoding_encoding.o $(LDLIBS)
 
clean:
	$(RM) *.o
	$(RM) decoding_encoding.exe
	$(RM) *.pgm
	$(RM) *.h264

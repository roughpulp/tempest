FFMPEG=/c/libs/ffmpeg-20160828-a37e6dd-win64-dev

CC =gcc
CXX=g++
RM=rm -rf

CXXFLAGS=-g -std=c++14 -I$(FFMPEG)/include/
LDFLAGS=-g
LDLIBS=-L$(FFMPEG)/lib/ -lavutil -lavcodec -lswscale

MAIN_SRCS=mmath.cc util.cc tempest.cc
MAIN_OBJS=$(subst .cc,.o,$(MAIN_SRCS))

UT_SRCS=mmath.ut.cc unit-tests.cc
UT_OBJS=$(subst .cc,.o,$(UT_SRCS))

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

all: tempest-codec.exe unit-tests.exe

tempest-codec.exe: $(MAIN_OBJS) main.o
	$(CXX) $(LDFLAGS) -o tempest-codec.exe main.o $(MAIN_OBJS) $(LDLIBS)

unit-tests.exe: $(MAIN_OBJS) $(UT_OBJS)
	$(CXX) $(LDFLAGS) -o unit-tests.exe $(MAIN_OBJS) $(UT_OBJS) $(LDLIBS)	

main.o: main.cc

unit-tests.o: unit-tests.cc

mmath.o: mmath.cc mmath.h

mmath.ut.o: mmath.ut.cc

util.o: util.cc util.h

tempest.o: tempest.cc tempest.h

tempest.ut.o: tempest.ut.cc

ffmpeg_encode_test.exe: ffmpeg_encode_test.o
	$(CXX) $(LDFLAGS) -o ffmpeg_encode_test.exe ffmpeg_encode_test.o $(LDLIBS)

ffmpeg_encode_test.o: ffmpeg_encode_test.cc
 
clean:
	$(RM) *.o
	
dist-clean: clean
	$(RM) tempest-codec.exe
	$(RM) unit-tests.exe

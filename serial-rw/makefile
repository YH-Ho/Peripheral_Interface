SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
TARGET  := serial-rw
CC      := arm-linux-gnueabihf-gcc
CXX     :=
DEFINES :=
INCLUDE :=
LIBS    +=
CFLAGS  += -g -Wall -O3 $(DEFINES) $(INCLUDE)
LDFLAGS := 
CXXFLAGS：= $(CFLAGS) -DHAVE_CONFIG_H

  
.PHONY : everything objs clean distclean rebuild 

all : $(TARGET)

objs : $(OBJS)

rebuild: distclean all            

clean :
	rm -rf *~
	rm -rf *.o

distclean : clean
	rm -rf $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

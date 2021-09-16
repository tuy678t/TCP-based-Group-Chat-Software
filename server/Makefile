CXX = g++
CXXFLAGS = -std=c++11 -Wall -g
CXXFLAGS += `mysql_config --cflags`
EXE = server
OBJS = common.o Connections.o Database.o MD5.o TCPServer.o Log.o

.PHONY: all clean

all: $(EXE)

$(EXE): main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ `mysql_config --libs`

$(OBJS): %.o : %.h

clean:
	$(RM) $(EXE) $(OBJS)

CC=g++
S = $(wildcard *.h)
C_SRCS = $(wildcard *.c)
CPP_SRCS = $(wildcard *.cpp)
CXX_SRCS = $(wildcard *.cxx)
OBJS = $(patsubst %.cpp, %.o, $(CPP_SRCS)) $(patsubst %.c, %.o, $(C_SRCS)) $(patsubst %.cxx, %.o, $(CXX_SRCS))

TARGET=yuv420ptorgb24
INCLUDE_DIR=-I/usr/local/ffmpeg/include \


SO_LIB_DIR=-L/usr/local/ffmpeg/lib \


SO_LIB_LIST=-lavcodec \
	-lavdevice \
	-lavfilter \
	-lavformat \
	-lavutil \
	-lswresample \
	-lswscale \


%.o:%.cxx
	$(CC) $(INCLUDE_DIR) -c $< -o $@
%.o:%.cpp
	$(CC) $(INCLUDE_DIR) -c $< -o $@

$(TARGET):$(OBJS)
	echo $(INCLUDE_DIR)
	$(CC) $^ $(SO_LIB_DIR) $(SO_LIB_LIST) -o $(TARGET)

clean:
	rm -rf *.o $(TARGET)

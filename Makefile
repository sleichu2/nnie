# Hisilicon Hi35xx sample Makefile
include $(PWD)/../Makefile.param

CFLAGS += -I$(PWD)/src
CFLAGS += -I$(PWD)/third_party/hi_nnie/include

CXXFLAGS += -I$(PWD)/src
CXXFLAGS += -I$(PWD)/third_party/hi_nnie/include
CXXFLAGS += -I$(PWD)/third_party/opencv4/include/opencv4
CXXFLAGS += -L$(PWD)/third_party/opencv4/lib
LD_OPENCV_LIBS += -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_videoio -lopencv_dnn -lopencv_ml -lopencv_photo \
			-lopencv_objdetect -lopencv_stitching -lopencv_flann -lopencv_imgcodecs -lopencv_gapi -lopencv_core -lopencv_calib3d \
			-lopencv_features2d

CXXFLAGS += ${LD_OPENCV_LIBS}
CXXFLAGS += -std=c++11
CXXFLAGS += -Wno-error
CXXFLAGS += -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math -fno-rtti -fno-exceptions -fpermissive
CXXFLAGS += -O3

SRCS := $(wildcard ./src/*.c)

TARGET := sample_nnie_main


# target source

OBJS  = $(SRCS:%.c=%.o)

CXX = aarch64-himix100-linux-g++
.PHONY : clean all

all: $(TARGET)

$(TARGET):  ./sample/sample_nnie_main.o ./src/ins_nnie_interface.o $(OBJS) 
	$(CXX) $(CXXFLAGS) $(LIBS_LD_CFLAGS) -lpthread -lm -o $@ $^ -Wl,--start-group $(MPI_LIBS) $(SENSOR_LIBS) $(AUDIO_LIBA) $(REL_LIB)/libsecurec.a -Wl,--end-group



clean:
	@rm -f $(TARGET) ./sample/sample_nnie_main.o ./src/ins_nnie_interface.o ./src/util.o
	@rm -f $(OBJS)
	@rm -f $(COMM_OBJ)

cleanstream:
	@rm -f *.h264
	@rm -f *.h265
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4
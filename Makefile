RACK_DIR ?= ../..

FLAGS += 
CFLAGS += 

ifeq ($(wildcard $(RACK_DIR)/include/rack.hpp),)
CXXFLAGS += -DMIRACK
DISTRIBUTABLES += res 
else
CXXFLAGS += 
DISTRIBUTABLES += $(wildcard LICENSE*) $(wildcard *.sh) $(wildcard *.bat) res demos 
endif


include $(RACK_DIR)/arch.mk
ifdef ARCH_WIN
LDFLAGS += -lwsock32
else
LDFLAGS +=
endif  

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/*.c)



# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

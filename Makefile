RACK_DIR ?= ../..
SLUG = HolonicSystems-Free
VERSION = 0.6.2

FLAGS +=
CFLAGS += 
CXXFLAGS +=

include $(RACK_DIR)/arch.mk
ifdef ARCH_WIN
LDFLAGS += -lwsock32
else
LDFLAGS +=
endif  

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/*.c)

DISTRIBUTABLES += $(wildcard LICENSE*) $(wildcard *.sh) $(wildcard *.bat) res demos 

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
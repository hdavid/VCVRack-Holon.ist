RACK_DIR ?= ../..
SLUG = HolonicSystems-Holonist
VERSION = 0.6.0

FLAGS +=
CFLAGS += 
CXXFLAGS +=

ifeq ($(OS),Windows_NT)
LDFLAGS += -lwsock32
else
LDFLAGS +=
endif  

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)
SOURCES += $(wildcard src/*.c)

DISTRIBUTABLES += $(wildcard LICENSE*) $(wildcard *.html) res

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
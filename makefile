# project
TARGET = 6502-burken
TARGET_TESTS = 6502-burken-tests



# make flags
MAKEFLAGS += --jobs $(shell nproc)

# compiler
CC 		= g++

# SUBPROJ_SRCDIRS returns "src/ src/imgui/ ..." etc.
SUBPROJ_SRCDIRS = $(sort $(dir $(wildcard $(SRCDIR)/*/)))
SUBPROJ_OBJDIRS = $(subst $(SRCDIR), $(OBJDIR), $(SUBPROJ_SRCDIRS))
SUBPROJ_SRCDIRS_INCLUDE = $(patsubst %, -I%, $(SUBPROJ_SRCDIRS))
SUBPROJ_OBJDIRS_INCLUDE = $(patsubst %, -aO%, $(SUBPROJ_OBJDIRS))

SOURCES := $(wildcard $(SRCDIR)/*.cpp)   \
			$(wildcard $(SRCDIR)/*/*.cpp)

OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# For release build
CFLAGS 	= 	-Iinclude \
			-Iinclude/imgui \
			-Isrc/imgui \
			-lglfw \
			-O2 -march=native \
			-std=c++2a \
			-Wall -pedantic -Wunused

#-DSUPPORT_DECIMAL_MODE \

# uncomment for debug build
CFLAGS += -O0 -pg -g

test: CFLAGS += -DTEST

DEBUG_FLAGS = #-DDEBUG_PRINTS

# linker
LINKER 	= g++
LFLAGS 	= -lpthread -ldl -lglfw -lm #-pg

# directories
OBJDIR 	= obj
SRCDIR 	= src
BINDIR 	= .

SOURCES		:= $(wildcard $(SRCDIR)/*.cpp) \
			   $(wildcard $(SRCDIR)/imgui/*.cpp)
INCLUDES 	:= $(wildcard $(SRCDIR)/*.h) \
			   $(wildcard $(SRCDIR)/imgui/*.h)
OBJECTS 	:= $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

build: tags $(OBJDIR) $(TARGET)

test: tags $(OBJDIR) $(TARGET_TESTS)

tags:
	ctags -R src/

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LFLAGS) -o $@

$(BINDIR)/$(TARGET_TESTS): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LFLAGS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)
	mkdir $(OBJDIR)/imgui

clean:
	rm $(BINDIR)/$(TARGET) &\
	rm $(BINDIR)/$(TARGET_TESTS) &\
	rm $(OBJECTS)&\
	rm tags & \
	rmdir $(OBJDIR) \

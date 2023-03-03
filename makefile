###############################################################################
# General configuration
###############################################################################

.PHONY: test build static-analysis clean tags

# project
TARGET = 6502-burken
TARGET_TESTS = 6502-burken-tests

# make flags
MAKEFLAGS += --jobs $(shell nproc)

# compiler
CC = g++

# SUBPROJ_SRCDIRS returns "src/ src/imgui/ ..." etc.
SUBPROJ_SRCDIRS = $(sort $(dir $(wildcard $(SRCDIR)/*/)))
SUBPROJ_OBJDIRS = $(subst $(SRCDIR), $(OBJDIR), $(SUBPROJ_SRCDIRS))
SUBPROJ_SRCDIRS_INCLUDE = $(patsubst %, -I%, $(SUBPROJ_SRCDIRS))
SUBPROJ_OBJDIRS_INCLUDE = $(patsubst %, -aO%, $(SUBPROJ_OBJDIRS))

SOURCES := $(wildcard $(SRCDIR)/*.cpp)   \
			$(wildcard $(SRCDIR)/*/*.cpp)

OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# For release build
WFLAGS  =   -Werror -Wall -Wpedantic -Wextra -Wvla -Wnull-dereference \
            -Wswitch-enum -Wno-deprecated -Wduplicated-cond \
            -Wduplicated-branches -Wpointer-arith -Wcast-qual \
            -Winit-self -Wuninitialized -Wcast-align -Wstrict-aliasing \
            -Wformat=2 -Wwrite-strings

CFLAGS 	= 	-isystem include \
			-isystem include/imgui \
			-Isrc/imgui \
			-lglfw \
			-O2 -march=native \
			-std=c++2a \
			$(WFLAGS)
#-DDEBUG_PRINTS
#-DSUPPORT_DECIMAL_MODE \


CPPCHECKFLAGS   = --max-ctu-depth=3 --enable=all --inline-suppr --suppress=variableScope \
                  --suppress=missingInclude --suppress=missingIncludeSystem \
                  --suppress=unmatchedSuppression --suppress=unusedFunction --std=c++11 \
                  --language=c++
CLANGTIDYFLAGS  = -checks=bugprone-*,clang-analyzer-*$\

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

###############################################################################
# Build rules 
###############################################################################
build: $(OBJDIR) $(TARGET)

test: $(OBJDIR) $(TARGET_TESTS)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LFLAGS) -o $@

$(BINDIR)/$(TARGET_TESTS): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LFLAGS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)
	mkdir $(OBJDIR)/imgui

#
# other rules
#
static-analysis:
	cppcheck $(CPPCHECKFLAGS) -isrc/imgui -isrc/glad.cpp $(SRCDIR)/*
	clang-tidy $(CLANGTIDYFLAGS) $(SRCDIR)/*.cpp $(SRCDIR)/*.h \
               -- $(CFLAGS) -Wno-unknown-warning-option

tags:
	ctags -R src/

clean:
	rm $(BINDIR)/$(TARGET) &\
	rm $(BINDIR)/$(TARGET_TESTS) &\
	rm $(OBJECTS)&\
	rm tags & \
	rmdir $(OBJDIR) \

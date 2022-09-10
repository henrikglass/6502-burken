# TODO remake this makefile

# project
TARGET 	= 6502-burken

# make flags
MAKEFLAGS += --jobs $(shell nproc)

# compiler
CC 		= g++

# other flags
#CFLAGS 	= -Iinclude -lglfw -O2 -std=c++2a -Wall -pedantic -march=native #-pg -g
CFLAGS 	= 	-Iinclude \
			-Iinclude/imgui \
			-Isrc/imgui \
			-lglfw \
			-pg -g \
			-std=c++2a \
			-Wall -pedantic

DEBUG_FLAGS = -DDEBUG_PRINTS

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

# targets
build: tags $(OBJDIR) $(TARGET)

mkdbg:
	echo $(OBJECTS)

tags:
	ctags -R src/

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LFLAGS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)
	mkdir $(OBJDIR)/imgui

clean:
	rm $(BINDIR)/$(TARGET) &\
	rm $(OBJDIR)/*.o &\
	rm $(OBJDIR)/imgui/*.o &\
	rm tags & \
	rmdir $(OBJDIR) \

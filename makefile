# project
TARGET 	= 6502-burken

# compiler
CC 		= g++

# other flags
CFLAGS 	= -Iinclude -lglfw -O2 -std=c++2a -Wall -pedantic -march=native #-pg -g
DEBUG_FLAGS = -DDEBUG_PRINTS

# linker
LINKER 	= g++
LFLAGS 	= -ldl -lglfw -lm #-pg

# directories
OBJDIR 	= obj
SRCDIR 	= src
BINDIR 	= .

SOURCES		:= $(wildcard $(SRCDIR)/*.cpp)
INCLUDES 	:= $(wildcard $(SRCDIR)/*.h)
OBJECTS 	:= $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# targets
build: tags $(OBJDIR) $(TARGET)

tags:
	ctags -R src/

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LFLAGS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm $(BINDIR)/$(TARGET) & rm $(OBJDIR)/*.o
	rm tags

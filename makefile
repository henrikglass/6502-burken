# project
TARGET 	= 6502-emul

# compiler
CC 		= g++

# compiler specific flags
CSFLAGS = #-fopenmp # gcc

# other flags
CFLAGS 	= -O2 -std=c++2a -Wall -pedantic -march=native #-pg -g

# linker
LINKER 	= g++
LFLAGS 	= -lm #-pg

# directories
OBJDIR 	= obj
SRCDIR 	= src
BINDIR 	= .

SOURCES		:= $(wildcard $(SRCDIR)/*.cpp)
INCLUDES 	:= $(wildcard $(SRCDIR)/*.h)
OBJECTS 	:= $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# targets
build: $(OBJDIR) $(TARGET)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(LINKER) $(OBJECTS) $(LFLAGS) $(CSFLAGS) -o $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(CSFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm $(BINDIR)/$(TARGET) & rm $(OBJDIR)/*.o

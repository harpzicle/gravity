SHELL := powershell.exe
.SHELLFLAGS := -NoProfile -Command


CC = gcc

TARGET := gravity

SRCDIR := src
INCDIR := inc
OBJDIR := obj
RESDIR := res
BINDIR := bin

CFLAGS := -mwindows -march=native -mtune=native -O3
LIBS := -lSDL3
INC := -I$(INCDIR)


SOURCES := $(shell resolve-path -relative $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.c=.o))

all: resources $(TARGET)

remake: cleaner all

resources: directories

directories:
	@mkdir -Force $(OBJDIR)
	@mkdir -Force $(BINDIR)

clean:
	@rm -Recurse -ErrorAction SilentlyContinue $(OBJDIR)

cleaner: clean
	@rm -Recurse -ErrorAction SilentlyContinue $(BINDIR)

$(TARGET): $(OBJDIR)/gravity.o $(OBJDIR)/sim.o $(OBJDIR)/render.o $(OBJDIR)/app.o
	$(CC) -o $(BINDIR)/$(TARGET) $^ $(LIBS) $(CFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

.PHONY: all remake clean cleaner resources
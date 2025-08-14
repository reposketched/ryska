CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
SRCDIR = src
OBJDIR = obj

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Target executable
TARGET = chess_engine

# Default target
all: $(TARGET)

# Create object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Clean build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Uninstall (optional)
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Run the engine
run: $(TARGET)
	./$(TARGET)

# Test the engine
test: $(TARGET)
	echo "uci" | ./$(TARGET)
	echo "isready" | ./$(TARGET)
	echo "position startpos" | ./$(TARGET)
	echo "go depth 4" | ./$(TARGET)

.PHONY: all clean install uninstall run test
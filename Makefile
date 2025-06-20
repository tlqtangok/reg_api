# Makefile for registry handler project using Visual Studio cl compiler
# cl.exe /EHsc /W4 /std:c++14  reg_api.cpp main.cpp /link advapi32.lib /out:reg_api.exe && reg_api.exe
# Compiler and flags
CC = cl
CFLAGS = /EHsc /W4 /std:c++17
LIBS = advapi32.lib

# Directories
BUILD_DIR = build
BIN_DIR = bin

# Source files
SRCS = reg_api.cpp main.cpp
OBJS = $(BUILD_DIR)\reg_api.obj $(BUILD_DIR)\main.obj
TARGET = $(BIN_DIR)\reg_api.exe

.PHONY: all run clean directories

all: directories $(TARGET)

directories:
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)

$(BUILD_DIR)\reg_api.obj: reg_api.cpp reg_api.h
	$(CC) $(CFLAGS) /c reg_api.cpp /Fo$(BUILD_DIR)\reg_api.obj

$(BUILD_DIR)\main.obj: main.cpp reg_api.h
	$(CC) $(CFLAGS) /c main.cpp /Fo$(BUILD_DIR)\main.obj

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) /Fe$(TARGET)

run: all
	@echo Running $(TARGET)...
	@$(TARGET)

clean:
	@echo Cleaning up...
	@if exist $(BUILD_DIR)\*.obj del /Q $(BUILD_DIR)\*.obj
	@if exist $(BIN_DIR)\*.exe del /Q $(BIN_DIR)\*.exe

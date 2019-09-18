# Builds every source in src/ into build/. Use `make` on Linux and macOS, `mingw32-make` with MinGW on Windows.

CC         := gcc
C_STD      := c99
CFLAGS     := -std=$(C_STD) -Wall -Wextra -Iinclude

SRC_DIR    := src
BUILD_DIR  := build
TARGET     := $(BUILD_DIR)/tic-tac-toe

SOURCES    := $(wildcard $(SRC_DIR)/*.c)
OBJECTS    := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
DEPENDS    := $(OBJECTS:.o=.d)

# Pinning the shell on Windows keeps the recipes below identical whether make is started from cmd, from PowerShell, or from a POSIX shell such as Git Bash.
ifeq ($(OS),Windows_NT)
    SHELL       := cmd.exe
    .SHELLFLAGS := /C
    TARGET      := $(TARGET).exe

    make_dir     = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"
    remove_dir   = if exist "$(subst /,\,$1)" rmdir /s /q "$(subst /,\,$1)"
    run_command  = cd "$(subst /,\,$(BUILD_DIR))" && ".\$(notdir $(TARGET))"
else
    make_dir     = mkdir -p "$1"
    remove_dir   = rm -rf "$1"
    run_command  = cd "$(BUILD_DIR)" && ./$(notdir $(TARGET))
endif

.PHONY: all run clean rebuild

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# -MMD -MP writes the header dependencies beside each object, so edited headers trigger a rebuild.
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@$(call make_dir,$(BUILD_DIR))

# Run from the build folder, so the save file the program writes lands beside the executable.
run: $(TARGET)
	@$(run_command)

clean:
	@$(call remove_dir,$(BUILD_DIR))

# Sub-makes rather than prerequisites, because the prerequisites of one target may run in parallel under -j.
rebuild:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory all

-include $(DEPENDS)

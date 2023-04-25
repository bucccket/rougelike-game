TARGET_EXEC ?= app

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

CC = clang

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

LIB_INCLUDE := -Iinclude/minifb
LDFLAGS += -L./lib
LDFLAGS += -l:libminifb.a -lGL -lX11 -lz -lpng

CPPFLAGS ?= $(LIB_INCLUDE) $(INC_FLAGS) -MMD -MP -g -O1 -fsanitize=address -omit-frame-pointer -fPIC -fPIE -Wall -Wextra -Wpedantic

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) -g -fsanitize=address -static-libsan

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: all clean profile

all: $(BUILD_DIR)/$(TARGET_EXEC)

clean:
	$(RM) -r $(BUILD_DIR)

profile:
	perf record $(BUILD_DIR)/$(TARGET_EXEC) && perf report

-include $(DEPS)

MKDIR_P ?= mkdir -p

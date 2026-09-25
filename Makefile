SHELL := /bin/sh
.DELETE_ON_ERROR:

TARGET ?= jcc
MODE ?= release
RUN_ENV ?=

# If a src/ directory exists, build sources from there. Otherwise build C files
# from the project root recursively. Override with e.g. SRC_DIRS := src lib.
SRC_DIRS ?= $(if $(wildcard src),src,.)
INC_DIRS ?= include $(SRC_DIRS)

BUILD_ROOT ?= build
BUILD_DIR := $(BUILD_ROOT)/$(MODE)
BIN_DIR ?= bin
TARGET_PATH := $(BIN_DIR)/$(MODE)/$(TARGET)

CC ?= cc
RM ?= rm -f

CSTD ?= -std=c17

WARNFLAGS ?= \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wformat=2 \
	-Wshadow \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Wold-style-definition \
	-Wundef \
	-Wcast-align \
	-Wwrite-strings \
	-Wconversion \
	-Wsign-conversion \
	-Wdouble-promotion \
	-Wnull-dereference \
	-Wimplicit-fallthrough \
	-Wvla \
	-Wno-unused-variable \
	-Wno-unused-parameter \
	-Wno-unused-function \
	-Wno-unused-but-set-variable

# Keep warnings visible, but do not treat them as build-stopping errors.
NOERRORFLAGS ?= -Wno-error

ifeq ($(MODE),debug)
MODE_CPPFLAGS :=
MODE_CFLAGS := -Og -g3
MODE_LDFLAGS :=
else ifeq ($(MODE),release)
MODE_CPPFLAGS := -DNDEBUG
MODE_CFLAGS := -O2
MODE_LDFLAGS :=
else ifeq ($(MODE),sanitize)
MODE_CPPFLAGS :=
MODE_CFLAGS := -O1 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer
MODE_LDFLAGS := -fsanitize=address,undefined
else
$(error Unknown MODE '$(MODE)'. Use debug, release, or sanitize)
endif

SRCS_RAW ?= $(shell find $(SRC_DIRS) -type f -name '*.c' \
	-not -path './$(BUILD_ROOT)/*' \
	-not -path '$(BUILD_ROOT)/*' \
	-not -path './$(BIN_DIR)/*' \
	-not -path '$(BIN_DIR)/*' 2>/dev/null)
SRCS := $(sort $(patsubst ./%,%,$(SRCS_RAW)))
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

CPPFLAGS += $(MODE_CPPFLAGS) $(addprefix -I,$(sort $(wildcard $(INC_DIRS)))) $(CPPFLAGS_EXTRA)
CFLAGS += $(CSTD) $(WARNFLAGS) $(MODE_CFLAGS) $(CFLAGS_EXTRA) $(NOERRORFLAGS)
LDFLAGS += $(MODE_LDFLAGS) $(LDFLAGS_EXTRA)
LDLIBS += $(LDLIBS_EXTRA)

ifeq ($(V),1)
Q :=
else
Q := @
endif

.PHONY: all
all: check-sources $(TARGET_PATH)

.PHONY: check-sources
check-sources:
	@test -n "$(strip $(SRCS))" || { \
		echo "No C sources found. Set SRC_DIRS='src lib' or SRCS='main.c foo.c'."; \
		exit 1; \
	}

$(TARGET_PATH): $(OBJS)
	@echo "LD  $@"
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: %.c
	@echo "CC  $<"
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

.PHONY: debug
debug:
	$(Q)$(MAKE) MODE=debug all

.PHONY: release
release:
	$(Q)$(MAKE) MODE=release all

.PHONY: sanitize
sanitize:
	$(Q)$(MAKE) MODE=sanitize all

.PHONY: run
run: all
	$(Q)$(RUN_ENV) ./$(TARGET_PATH) $(ARGS)

.PHONY: run-release
run-release:
	$(Q)$(MAKE) MODE=release run

.PHONY: run-sanitize
run-sanitize:
	$(Q)$(MAKE) MODE=sanitize run

.PHONY: clean
clean:
	$(RM) -r $(BUILD_ROOT) $(BIN_DIR)

.PHONY: distclean
distclean: clean
	$(RM) compile_flags.txt compile_commands.json

.PHONY: compile_flags.txt
compile_flags.txt:
	@printf '%s\n' -x c $(CSTD) $(WARNFLAGS) $(CPPFLAGS) $(NOERRORFLAGS) > $@
	@echo "Wrote $@ for clangd"

.PHONY: compile_commands.json
compile_commands.json:
	@command -v bear >/dev/null 2>&1 || { \
		echo "bear is not installed. Use 'make compile_flags.txt' or install bear."; \
		exit 1; \
	}
	$(Q)bear -- $(MAKE) clean all

.PHONY: tidy
tidy:
	@command -v clang-tidy >/dev/null 2>&1 || { \
		echo "clang-tidy is not installed."; \
		exit 1; \
	}
	$(Q)clang-tidy $(SRCS) -- $(CPPFLAGS) $(CFLAGS)

.PHONY: print-%
print-%:
	@printf '%s=%s\n' '$*' '$($*)'

-include $(DEPS)


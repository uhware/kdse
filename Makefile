# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later
# This file is part of KDSE.

CC ?= cc
AR ?= ar
INSTALL ?= install
PREFIX ?= /usr/local
DESTDIR ?=
BUILD_DIR ?= build

CPPFLAGS ?=
CPPFLAGS += -Iinclude
CFLAGS ?= -O2
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Werror
LDFLAGS ?=
LDLIBS ?=

CHECKED_OBJECT := $(BUILD_DIR)/obj/kdse8_checked.o
COMPUTE_OBJECT := $(BUILD_DIR)/obj/kdse8_compute.o
CHECKED_LIBRARY := $(BUILD_DIR)/lib/libkdse8_checked.a
COMPUTE_LIBRARY := $(BUILD_DIR)/lib/libkdse8_compute.a
CHECKED16_OBJECT := $(BUILD_DIR)/obj/kdse16_checked.o
COMPUTE16_OBJECT := $(BUILD_DIR)/obj/kdse16_compute.o
CHECKED16_LIBRARY := $(BUILD_DIR)/lib/libkdse16_checked.a
COMPUTE16_LIBRARY := $(BUILD_DIR)/lib/libkdse16_compute.a
CLI_OBJECT := $(BUILD_DIR)/obj/cli_kdse.o
CLI := $(BUILD_DIR)/bin/kdse
CLI16_OBJECT := $(BUILD_DIR)/obj/cli_kdse16.o
CLI16 := $(BUILD_DIR)/bin/kdse16
EXAMPLE_COMPUTE_OBJECT := $(BUILD_DIR)/obj/example_compute.o
EXAMPLE_CANONICALIZE_OBJECT := $(BUILD_DIR)/obj/example_canonicalize.o
EXAMPLE_COMPUTE := $(BUILD_DIR)/bin/example_compute
EXAMPLE_CANONICALIZE := $(BUILD_DIR)/bin/example_canonicalize
EXAMPLE_COMPUTE16_OBJECT := $(BUILD_DIR)/obj/example_compute16.o
EXAMPLE_CANONICALIZE16_OBJECT := $(BUILD_DIR)/obj/example_canonicalize16.o
EXAMPLE_COMPUTE16 := $(BUILD_DIR)/bin/example_compute16
EXAMPLE_CANONICALIZE16 := $(BUILD_DIR)/bin/example_canonicalize16
TEST_CHECKED_OBJECT := $(BUILD_DIR)/obj/test_checked.o
TEST_COMPUTE_OBJECT := $(BUILD_DIR)/obj/test_compute.o
TEST_CHECKED := $(BUILD_DIR)/bin/test_checked
TEST_COMPUTE := $(BUILD_DIR)/bin/test_compute
TEST_CHECKED16_OBJECT := $(BUILD_DIR)/obj/test_checked16.o
TEST_COMPUTE16_OBJECT := $(BUILD_DIR)/obj/test_compute16.o
TEST_CHECKED16 := $(BUILD_DIR)/bin/test_checked16
TEST_COMPUTE16 := $(BUILD_DIR)/bin/test_compute16

.PHONY: all libraries examples test clean install

all: libraries $(CLI) $(CLI16) examples

libraries: $(CHECKED_LIBRARY) $(COMPUTE_LIBRARY) \
		$(CHECKED16_LIBRARY) $(COMPUTE16_LIBRARY)

examples: $(EXAMPLE_COMPUTE) $(EXAMPLE_CANONICALIZE) \
		$(EXAMPLE_COMPUTE16) $(EXAMPLE_CANONICALIZE16)

$(BUILD_DIR)/obj/kdse8_checked.o: src/kdse8_checked.c \
		include/kdse/kdse8.h include/kdse/kdse8_checked.h include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/obj/kdse8_compute.o: src/kdse8_compute.c \
		include/kdse/kdse8.h include/kdse/kdse8_compute.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(CHECKED_LIBRARY): $(CHECKED_OBJECT)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(COMPUTE_LIBRARY): $(COMPUTE_OBJECT)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(BUILD_DIR)/obj/kdse16_checked.o: src/kdse16_checked.c \
		include/kdse/kdse16.h include/kdse/kdse16_checked.h \
		include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/obj/kdse16_compute.o: src/kdse16_compute.c \
		include/kdse/kdse16.h include/kdse/kdse16_compute.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(CHECKED16_LIBRARY): $(CHECKED16_OBJECT)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(COMPUTE16_LIBRARY): $(COMPUTE16_OBJECT)
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $^

$(CLI_OBJECT): cli/kdse.c include/kdse/kdse8.h \
		include/kdse/kdse8_checked.h include/kdse/kdse8_compute.h \
		include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(CLI): $(CLI_OBJECT) $(CHECKED_LIBRARY) $(COMPUTE_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(CLI16_OBJECT): cli/kdse16.c include/kdse/kdse16.h \
		include/kdse/kdse16_checked.h include/kdse/kdse16_compute.h \
		include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(CLI16): $(CLI16_OBJECT) $(CHECKED16_LIBRARY) $(COMPUTE16_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXAMPLE_COMPUTE_OBJECT): examples/compute.c include/kdse/kdse8.h \
		include/kdse/kdse8_checked.h include/kdse/kdse8_compute.h \
		include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(EXAMPLE_CANONICALIZE_OBJECT): examples/canonicalize.c \
		include/kdse/kdse8.h include/kdse/kdse8_checked.h \
		include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(EXAMPLE_COMPUTE): $(EXAMPLE_COMPUTE_OBJECT) \
		$(CHECKED_LIBRARY) $(COMPUTE_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXAMPLE_CANONICALIZE): $(EXAMPLE_CANONICALIZE_OBJECT) $(CHECKED_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXAMPLE_COMPUTE16_OBJECT): examples/compute16.c include/kdse/kdse16.h \
		include/kdse/kdse16_checked.h include/kdse/kdse16_compute.h \
		include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(EXAMPLE_CANONICALIZE16_OBJECT): examples/canonicalize16.c \
		include/kdse/kdse16.h include/kdse/kdse16_checked.h \
		include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(EXAMPLE_COMPUTE16): $(EXAMPLE_COMPUTE16_OBJECT) \
		$(CHECKED16_LIBRARY) $(COMPUTE16_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(EXAMPLE_CANONICALIZE16): $(EXAMPLE_CANONICALIZE16_OBJECT) \
		$(CHECKED16_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TEST_CHECKED_OBJECT): tests/test_checked.c include/kdse/kdse8.h \
		include/kdse/kdse8_checked.h include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TEST_COMPUTE_OBJECT): tests/test_compute.c include/kdse/kdse8.h \
		include/kdse/kdse8_checked.h include/kdse/kdse8_compute.h \
		include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TEST_CHECKED): $(TEST_CHECKED_OBJECT) $(CHECKED_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TEST_COMPUTE): $(TEST_COMPUTE_OBJECT) \
		$(CHECKED_LIBRARY) $(COMPUTE_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -lm -o $@

$(TEST_CHECKED16_OBJECT): tests/test_checked16.c include/kdse/kdse16.h \
		include/kdse/kdse16_checked.h include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TEST_COMPUTE16_OBJECT): tests/test_compute16.c include/kdse/kdse16.h \
		include/kdse/kdse16_checked.h include/kdse/kdse16_compute.h \
		include/kdse/status.h
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TEST_CHECKED16): $(TEST_CHECKED16_OBJECT) $(CHECKED16_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(TEST_COMPUTE16): $(TEST_COMPUTE16_OBJECT) \
		$(CHECKED16_LIBRARY) $(COMPUTE16_LIBRARY)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -lm -o $@

test: $(TEST_CHECKED) $(TEST_COMPUTE) $(TEST_CHECKED16) \
		$(TEST_COMPUTE16) $(CLI) $(CLI16)
	$(TEST_CHECKED)
	$(TEST_COMPUTE)
	sh tests/test_cli.sh $(CLI)
	$(TEST_CHECKED16)
	$(TEST_COMPUTE16)
	sh tests/test_cli16.sh $(CLI16)

install: all
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/include/kdse
	$(INSTALL) -m 644 include/kdse/*.h $(DESTDIR)$(PREFIX)/include/kdse
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -m 644 $(CHECKED_LIBRARY) $(COMPUTE_LIBRARY) \
		$(CHECKED16_LIBRARY) $(COMPUTE16_LIBRARY) \
		$(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 755 $(CLI) $(DESTDIR)$(PREFIX)/bin/kdse
	$(INSTALL) -m 755 $(CLI16) $(DESTDIR)$(PREFIX)/bin/kdse16

clean:
	rm -rf -- $(BUILD_DIR)

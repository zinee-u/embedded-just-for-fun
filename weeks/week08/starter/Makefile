CC ?= cc
CPPFLAGS += -Iinclude -Ihost
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
COMMON = src/sensor.c host/host.c
HEADERS = include/sensor.h include/platform.h host/host.h
.PHONY: all test demo clean
all: build/test_sensor build/demo
build:
	mkdir -p build
build/test_sensor: $(COMMON) tests/test_sensor.c tests/test_student.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(COMMON) tests/test_sensor.c tests/test_student.c -o $@
build/demo: $(COMMON) host/demo.c $(HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(COMMON) host/demo.c -o $@
test: build/test_sensor
	./build/test_sensor
demo: build/demo
	./build/demo
clean:
	rm -rf build

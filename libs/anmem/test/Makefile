TESTS=test_basic.c
CONFIG_TESTS=test_configs.c
OBJECTS=../build/*.o $(wildcard ../libs/*/build/*.o)
LIBS=anlock analloc anpages
TEST_HEADERS=-I../include -I../libs/analloc/src -I../libs/anpages/src -I./include

tests: build ../build libs_build
	for test in $(TESTS); do \
		gcc $(OBJECTS) $(TEST_HEADERS) $$test -o build/`basename $$test .c`; \
	done

config_tests: build buildconfig libs_build
	for test in $(CONFIG_TESTS); do \
		gcc $(OBJECTS) $(TEST_HEADERS) $$test -o build/`basename $$test .c`; \
	done

libs_build:
	for lib in $(LIBS); do \
		cd ../libs/$$lib && make && cd -; \
	done

../build:
	cd .. && make INCLUDES=-I./test/include CFLAGS=-DIGNORE_4GB_RULE=1

buildconfig:
	cd .. && make INCLUDES=-I./test/include

build:
	mkdir build

clean:
	cd .. && make clean
	rm -rf build
	for lib in $(LIBS); do \
		cd ../libs/$$lib && make clean && cd -; \
	done

GCC = g++ -ggdb3 -Wall -std=c++0x

SRC = 				\
	block.cpp		\
	communicate.cpp		\
	compress.cpp		\
	config.cpp		\
	crypt.cpp		\
	db.cpp			\
	mode.cpp		\
	root.cpp		\
	system.cpp		\
	transport.cpp		\


HEADER = cryptar.h

OBJECT = $(SRC:%.cpp=%.o)

LIBS = 					\
	-lboost_program_options		\
	-lboost_serialization		\
	-lboost_thread			\
	-lboost_filesystem		\
	-lboost_system			\
	-lcrypto++			\
	-lbz2				\

TEST_LIBS = 				\
	-lboost_prg_exec_monitor	\
	-lboost_unit_test_framework	\

all : TAGS test cryptar libs

%.o : %.cpp cryptar.h
	$(GCC) -c -fpic -o $@ $<

%.o : %.cpp
	$(GCC) -c -fpic -o $@ $<

cryptar : main.o $(HEADER) $(OBJECT) Makefile TAGS
	$(GCC) -o cryptar main.o $(OBJECT) $(LIBS)

libs : libcryptar.so libcryptar.a

libcryptar.a : $(OBJECT)
	ar rcs $@ $^

libcryptar.so : $(OBJECT)
	$(GCC) -shared -Wl,-soname,$@.1 -o $@.1.0.1 $^

TESTS = 			\
	block_test		\
	compress_test		\
	communicate_test	\
	config_test		\
	crypt_test 		\
	db_test			\
	header_test		\
	mode_test 		\
	root_test		\
	transport_test		\

# For a more verbose test, try e.g.
#    $ CRYPTAR_TEST_LOG_LEVEL=test_suite make test
#    $ CRYPTAR_TEST_LOG_LEVEL=test_suite make crypt_test
test : $(TESTS)
	./random.sh

LOG_LEVEL=$(shell if [ $$CRYPTAR_TEST_LOG_LEVEL ]; then echo --log_level=$$CRYPTAR_TEST_LOG_LEVEL; fi)

test_text.o : test_text.h TAGS

# random.sh will compile random
random : random.o libcryptar.so
	$(GCC) -o random random.o -L. -lcryptar $(LIBS)

%_test : %_test.o test_text.o mode.o $(OBJECT)
	$(GCC) -o $@ $^ $(TEST_LIBS) $(LIBS)
	-./$@ $(LOG_LEVEL)

header_test :
	./build-test-header

clean :
	rm -f $(OBJECT) *.o *~ cryptar TAGS *_test tmp_h_test_*
	rm -rf /tmp/cryptar-$LOGNAME-[0-9]*-[0-9]*
	rm -f libcryptar.a libcryptar.so*

TAGS : *cpp *h
	etags *cpp *h


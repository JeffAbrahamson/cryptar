GCC = g++ -ggdb3 -Wall
#GCC = g++ -ggdb3 -Wall -std=c++0x
# c++0x gives us rvalue references, most notably

SRC = 			\
	compress.cpp	\
	crypt.cpp	\
	mode.cpp	\


HEADER = cryptar.h

OBJECT = $(SRC:%.cpp=%.o)

LIBS = 					\
	-lboost_program_options		\
	-lboost_serialization		\
	-lcrypto++			\
	-lbz2				\

TEST_LIBS = 				\
	-lboost_prg_exec_monitor	\
	-lboost_unit_test_framework	\

all : cryptar test TAGS

%.o : %.cpp %.h
	$(GCC) -c -fpic -o $@ $<

%.o : %.cpp
	$(GCC) -c -fpic -o $@ $<

cryptar : main.o $(HEADER) $(OBJECT) Makefile
	$(GCC) -o cryptar main.o $(OBJECT) $(LIBS)

TESTS = 			\
	compress_test		\
	crypt_test 		\
	mode_test 		\

# For a more verbose test, try e.g.
#    $ CRYPTAR_TEST_LOG_LEVEL=test_suite make test
#    $ CRYPTAR_TEST_LOG_LEVEL=test_suite make crypt_test
test : $(TESTS)

LOG_LEVEL=$(shell if [ $$CRYPTAR_TEST_LOG_LEVEL ]; then echo --log_level=$$CRYPTAR_TEST_LOG_LEVEL; fi)

%_test : %_test.o test_text.o mode.o $(OBJECT)
	$(GCC) -o $@ $^ $(TEST_LIBS) $(LIBS)
	-./$@ $(LOG_LEVEL)

clean :
	rm -f $(OBJECT) *.o *~ cryptar TAGS *_test

#clean-test :
#	rm -rf srd-test-*/ test_[0-9]*\.[0-9]*

TAGS : *cpp *h
	etags *cpp *h




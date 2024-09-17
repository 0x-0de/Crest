GCC = g++
CPP_FLAGS = -std=c++11 -Wall

DIR_OBJ = bin/obj
DIR_OBJ_TEST = bin/obj/test
DIR_EXP = bin
DIR_SRC = src

LIBS_BUILD = -lole32 -lpthread -lwinmm -shared
LIBS_TEST = -L./bin/ -lcrest

BUILD_DEF = -DCREST_EXPORT

OBJ_BUILD = $(addprefix $(DIR_OBJ)/, crest.o wavutils.o conv.o flacutils.o effects.o vorbisutils.o)

OBJ_TEST_01 = $(addprefix $(DIR_OBJ_TEST)/, 01_simple_demo.o)
OBJ_TEST_02 = $(addprefix $(DIR_OBJ_TEST)/, 02_wav_files.o)

OBJ_TEST = bin/test.o

build: $(OBJ_BUILD)
	$(GCC) $(OBJ_BUILD) $(LIBS_BUILD) -o $(DIR_EXP)/crest.dll
	rm -rf $(DIR_OBJ)

test_01: $(OBJ_TEST_01)
	$(GCC) $(OBJ_TEST_01) $(LIBS_TEST) -o $(DIR_EXP)/test_01.exe
	rm -rf $(DIR_OBJ)

test_02: $(OBJ_TEST_02)
	$(GCC) $(OBJ_TEST_02) $(LIBS_TEST) -o $(DIR_EXP)/test_02.exe
	rm -rf $(DIR_OBJ)

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp $(DIR_SRC)/%.h | $(DIR_OBJ)
	$(GCC) $(CPP_FLAGS) $(BUILD_DEF) -c $< -o $@

$(DIR_OBJ_TEST)/%.o: ./tests/%.cpp | $(DIR_OBJ_TEST)
	$(GCC) $(CPP_FLAGS) -c $< -o $@

clean:
	rm -rf $(DIR_OBJ)
	rm $(DIR_EXP)/crest.dll

$(DIR_OBJ):
	mkdir -p $@

$(DIR_OBJ_TEST):
	mkdir -p $@
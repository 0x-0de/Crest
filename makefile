GCC = g++
CPP_FLAGS = -std=c++11 -Wall
CPP_FLAGS_STATIC = -std=c++11 -Wall -g

DIR_OBJ = bin/obj
DIR_OBJ_TEST = bin/obj/test
DIR_OBJ_TEST_STATIC = bin/obj/test/static
DIR_EXP = bin
DIR_SRC = src

LIBS_BUILD = -lole32 -lpthread -lwinmm -shared
LIBS_STATIC = -lole32 -lpthread -lwinmm
LIBS_TEST = -L./bin/ -lcrest

BUILD_DEF = -DCREST_EXPORT
STATIC_DEF = -DCREST_STATIC

OBJ_BUILD = $(addprefix $(DIR_OBJ)/, crest.o wavutils.o conv.o flacutils.o effects.o vorbisutils.o)

OBJ_TEST_01 = $(addprefix $(DIR_OBJ_TEST)/, 01_simple_demo.o)
OBJ_TEST_02 = $(addprefix $(DIR_OBJ_TEST)/, 02_wav_files.o)
OBJ_TEST_03 = $(addprefix $(DIR_OBJ_TEST)/, 03_flac_files.o)
OBJ_TEST_04 = $(addprefix $(DIR_OBJ_TEST)/, 04_bit_reader.o)
OBJ_TEST_05 = $(addprefix $(DIR_OBJ_TEST)/, 05_stressing_vorbis.o)

OBJ_TEST_05_STATIC = $(addprefix $(DIR_OBJ_TEST_STATIC)/, 05_stressing_vorbis.o)

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

test_03: $(OBJ_TEST_03)
	$(GCC) $(OBJ_TEST_03) $(LIBS_TEST) -o $(DIR_EXP)/test_03.exe
	rm -rf $(DIR_OBJ)

test_04: $(OBJ_TEST_04)
	$(GCC) $(OBJ_TEST_04) $(LIBS_TEST) -o $(DIR_EXP)/test_04.exe
	rm -rf $(DIR_OBJ)

test_05: $(OBJ_TEST_05)
	$(GCC) $(OBJ_TEST_05) $(LIBS_TEST) -o $(DIR_EXP)/test_05.exe
	rm -rf $(DIR_OBJ)

test_05_static:
	$(GCC) $(CPP_FLAGS_STATIC) $(STATIC_DEF) ./tests/05_stressing_vorbis.cpp $(DIR_SRC)/conv.cpp $(DIR_SRC)/crest.cpp $(DIR_SRC)/effects.cpp $(DIR_SRC)/flacutils.cpp $(DIR_SRC)/vorbisutils.cpp $(DIR_SRC)/wavutils.cpp $(LIBS_STATIC) -o $(DIR_EXP)/test_05.exe

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
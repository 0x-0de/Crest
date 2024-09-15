GCC = g++
CPP_FLAGS = -std=c++11 -Wall

DIR_OBJ = bin/obj
DIR_EXP = bin
DIR_SRC = src

LIBS_BUILD = -lole32 -lpthread -lwinmm -shared
LIBS_TEST = -L./ -lcrest

BUILD_DEF = -DCREST_EXPORT

OBJ_BUILD = $(addprefix $(DIR_OBJ)/, crest.o wavutils.o conv.o flacutils.o effects.o vorbisutils.o)
OBJ_TEST = bin/test.o

build: $(OBJ_BUILD)
	$(GCC) $(OBJ_BUILD) $(LIBS_BUILD) -o $(DIR_EXP)/crest.dll
	rm -rf $(DIR_OBJ)

test: $(OBJ_TEST)
	$(GCC) $(OBJ_TEST) $(LIBS_TEST) -o test.exe

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp $(DIR_SRC)/%.h | $(DIR_OBJ)
	$(GCC) $(CPP_FLAGS) $(BUILD_DEF) -c $< -o $@

test.o: ../src/test/test.cpp
	$(GCC) $(CPP_FLAGS) ../src/test/test.cpp

clean:
	rm -rf $(DIR_OBJ)
	rm $(DIR_EXP)/crest.dll

$(DIR_OBJ):
	mkdir -p $@
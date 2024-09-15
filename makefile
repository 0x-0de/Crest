GCC = g++
CPP_FLAGS = -std=c++11 -Wall -c

LIBS_BUILD = -lole32 -lpthread -lwinmm -shared
LIBS_DEBUG = -lole32 -lpthread -lwinmm
LIBS_TEST = -L./ -lcrest

BUILD_DEF = -DCREST_EXPORT

OBJ_BUILD = crest.o wavutils.o conv.o flacutils.o effects.o vorbisutils.o
OBJ_TEST = test.o
OBJ_DEBUG = crest_db.o wavutils_db.o conv_db.o flacutils_db.o effects_db.o vorbisutils_db.o test.o

VORBIS_TEST = stb_vorbis_lab.o
FLAC_TEST = flac_lab.o

build: $(OBJ_BUILD)
	$(GCC) $(OBJ_BUILD) $(LIBS_BUILD) -o crest.dll

debug: $(OBJ_DEBUG)
	$(GCC) $(OBJ_DEBUG) $(LIBS_DEBUG) -o crest_db.exe

test: $(OBJ_TEST)
	$(GCC) $(OBJ_TEST) $(LIBS_TEST) -o test.exe

vorbis-test: $(VORBIS_TEST)
	$(GCC) $(VORBIS_TEST) $(LIBS) -o vorbis.exe

flac-test: $(FLAC_TEST)
	$(GCC) $(FLAC_TEST) $(LIBS) -o flac.exe

crest.o: ../src/crest/crest.h ../src/crest/crest.cpp
	$(GCC) $(CPP_FLAGS) $(BUILD_DEF) ../src/crest/crest.cpp

conv.o: ../src/crest/utils/conv.h ../src/crest/utils/conv.cpp
	$(GCC) $(CPP_FLAGS) $(BUILD_DEF) ../src/crest/utils/conv.cpp	

wavutils.o: ../src/crest/utils/wavutils.h ../src/crest/utils/wavutils.cpp
	$(GCC) $(CPP_FLAGS) $(BUILD_DEF) ../src/crest/utils/wavutils.cpp

flacutils.o: ../src/crest/utils/flacutils.h ../src/crest/utils/flacutils.cpp
	$(GCC) $(CPP_FLAGS) $(BUILD_DEF) ../src/crest/utils/flacutils.cpp

effects.o: ../src/crest/effects.h ../src/crest/effects.cpp
	$(GCC) $(CPP_FLAGS) $(BUILD_DEF) ../src/crest/effects.cpp

vorbisutils.o: ../src/crest/utils/vorbisutils.h ../src/crest/utils/vorbisutils.cpp
	$(GCC) $(CPP_FLAGS) $(BUILD_DEF) ../src/crest/utils/vorbisutils.cpp

stb_vorbis.o: ../src/xtra/stb_vorbis.c
	$(GCC) $(CPP_FLAGS) ../src/xtra/stb_vorbis.c

stb_vorbis_lab.o: ../src/xtra/stb_vorbis_lab.cpp
	$(GCC) $(CPP_FLAGS) ../src/xtra/stb_vorbis_lab.cpp

crest_db.o: ../src/crest/crest.h ../src/crest/crest.cpp
	$(GCC) $(CPP_FLAGS) ../src/crest/crest.cpp -o crest_db.o

conv_db.o: ../src/crest/utils/conv.h ../src/crest/utils/conv.cpp
	$(GCC) $(CPP_FLAGS) ../src/crest/utils/conv.cpp	-o conv_db.o

wavutils_db.o: ../src/crest/utils/wavutils.h ../src/crest/utils/wavutils.cpp
	$(GCC) $(CPP_FLAGS) ../src/crest/utils/wavutils.cpp -o wavutils_db.o

flacutils_db.o: ../src/crest/utils/flacutils.h ../src/crest/utils/flacutils.cpp
	$(GCC) $(CPP_FLAGS) ../src/crest/utils/flacutils.cpp -o flacutils_db.o

effects_db.o: ../src/crest/effects.h ../src/crest/effects.cpp
	$(GCC) $(CPP_FLAGS) ../src/crest/effects.cpp -o effects_db.o

vorbisutils_db.o: ../src/crest/utils/vorbisutils.h ../src/crest/utils/vorbisutils.cpp
	$(GCC) $(CPP_FLAGS) ../src/crest/utils/vorbisutils.cpp -o vorbisutils_db.o

test.o: ../src/test/test.cpp
	$(GCC) $(CPP_FLAGS) ../src/test/test.cpp

vorbis_lab.o: ../src/xtra/vorbis_lab.cpp
	$(GCC) $(CPP_FLAGS) ../src/xtra/vorbis_lab.cpp

flac_lab.o: ../src/xtra/flac_lab.cpp
	$(GCC) $(CPP_FLAGS) ../src/xtra/flac_lab.cpp

clean:
	-rm *.o crest.dll crest_db.exe
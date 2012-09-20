OSTYPE = $(shell uname)
ARCH   = $(shell uname -m)
##################################### Defaults ################################
CC             := gcc
INCLUDE        := -I.
CFLAGS         := -Wall -std=c99 -pedantic -O3
CFLAGS_32      := -m32
SHLIB_CFLAGS   := -fPIC -shared
SHLIB_EXT      := .so
LIBS           := -lm
LDFLAGS        :=
############################# Library add-ons #################################
INCLUDE += -I/opt/local/include
LIBS    += -L/opt/local/lib -lpthread -lhdf5
CFLAGS  += -DH5_NO_DEPRECATED_SYMBOLS
GSLLIBS  = $(shell gsl-config --libs)
LIBS    += $(GSLLIBS)
LIBS    += -lfftw3
############################# OS & ARCH specifics #############################
ifneq ($(if $(filter Linux %BSD,$(OSTYPE)),OK), OK)
  ifeq ($(OSTYPE), Darwin)
    SHLIB_CFLAGS   := -dynamiclib
    SHLIB_EXT      := .dylib
    ifeq ($(shell sysctl -n hw.optional.x86_64), 1)
      ARCH           := x86_64
    endif
  else
    ifeq ($(OSTYPE), SunOS)
      CFLAGS         := -c -Wall -std=c99 -pedantic
    else
      # Let's assume this is win32
      SHLIB_EXT      := .dll
    endif
  endif
endif

ifneq ($(ARCH), x86_64)
  CFLAGS_32 += -m32
endif

# Are all G5s ppc970s?
ifeq ($(ARCH), ppc970)
  CFLAGS += -m64
endif
############################ Define targets ###################################
EXE_TARGETS = analyzeWaveform
DEBUG_EXE_TARGETS = hdf5rawWaveformIo filters
# SHLIB_TARGETS = XXX$(SHLIB_EXT)

ifeq ($(ARCH), x86_64) # compile a 32bit version on 64bit platforms
  # SHLIB_TARGETS += XXX_m32$(SHLIB_EXT)
endif

.PHONY: exe_targets shlib_targets debug_exe_targets clean
exe_targets: $(EXE_TARGETS)
shlib_targets: $(SHLIB_TARGETS)
debug_exe_targets: $(DEBUG_EXE_TARGETS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

analyzeWaveform: main.o filters.o runScriptNGetConfig.o hdf5rawWaveformIo.o
	$(CC) $(CFLAGS) $(INCLUDE) $^ $(LIBS) $(LDFLAGS) -o $@
main.o: main.c hdf5rawWaveformIo.h common.h
filters.o: filters.c filters.h common.h
filters: filters.c filters.h common.h
	$(CC) $(CFLAGS) $(INCLUDE) -DFILTERS_DEBUG_ENABLEMAIN $< $(LIBS) $(LDFLAGS) -o $@
runScriptNGetConfig.o: runScriptNGetConfig.c runScriptNGetConfig.h common.h
hdf5rawWaveformIo.o: hdf5rawWaveformIo.c hdf5rawWaveformIo.h common.h
hdf5rawWaveformIo: hdf5rawWaveformIo.c hdf5rawWaveformIo.h
	$(CC) $(CFLAGS) $(INCLUDE) -DHDF5RAWWAVEFORMIO_DEBUG_ENABLEMAIN $< $(LIBS) $(LDFLAGS) -o $@

# libmreadarray$(SHLIB_EXT): mreadarray.o
# 	$(CC) $(SHLIB_CFLAGS) $(CFLAGS) $(LIBS) -o $@ $<
# mreadarray.o: mreadarray.c
# 	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<
# mreadarray: mreadarray.c
# 	$(CC) $(CFLAGS) -DENABLEMAIN $(INCLUDE) $(LIBS) -o $@ $<
# libmreadarray_m32$(SHLIB_EXT): mreadarray.c
# 	$(CC) -m32 $(SHLIB_CFLAGS) $(CFLAGS) $(CFLAGS_32) -o $@ $<

clean:
	rm -f *.o *.so *.dylib *.dll *.bundle
	rm -f $(SHLIB_TARGETS) $(EXE_TARGETS) $(DEBUG_EXE_TARGETS)

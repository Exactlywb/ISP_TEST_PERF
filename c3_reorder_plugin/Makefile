HOST_GCC=g++
TARGET_GCC=/home/dmitry/gcc_patched/gcc-install/bin/gcc
PLUGIN_SOURCE_FILE=source/c3-ipa.cpp
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -fno-rtti -O2

.PHONY: all clean

all: c3-ipa.so

c3-ipa.so: $(PLUGIN_SOURCE_FILE)
	$(HOST_GCC) -shared $(CXXFLAGS) $^ -o $@

clean:
	rm c3-ipa.so


LDFLAGS+= -fPIC -fno-rtti -L$(HOME)/v8-lib/lib-osx-32 -stdlib=libstdc++ -lv8_base -lv8_libbase -lv8_libplatform -lv8_nosnapshot -dead_strip
CXXFLAGS+= -Wall -m32 -stdlib=libstdc++ -std=c++11 -I$(HOME)/v8 -Os

FILE=V8Simple
OBJ_DIR=obj
LIB_DIR=lib
LIB_FILE=lib$(FILE).dylib

all: $(LIB_DIR)/$(LIB_FILE) $(LIB_DIR)/$(FILE).dll

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.cxx
	@mkdir -p $(OBJ_DIR)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(LIB_DIR)/$(LIB_FILE): $(OBJ_DIR)/$(FILE).o $(OBJ_DIR)/$(FILE)_wrap.o
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared $(CXXFLAGS) $(LDFLAGS) $^ -o $@

$(FILE).cs $(FILE)_wrap.cxx: $(FILE).i $(FILE).h
	swig -csharp -dllimport $(FILE) -namespace Fuse.Scripting.V8.Simple -c++ -outfile $(FILE).cs $<

$(LIB_DIR)/$(FILE).dll: $(FILE).cs
	mcs -t:library $^ -out:$@

.PHONY: clean

clean:
	rm $(FILE)_wrap.cxx
	rm -r $(LIB_DIR)
	rm -r $(OBJ_DIR)

LDFLAGS= -fPIC -fno-rtti -L$(HOME)/v8-lib/lib-osx-32 -stdlib=libstdc++ -lv8_base -lv8_libbase -lv8_libplatform -lv8_nosnapshot -dead_strip
CPPFLAGS= -Wall -m32 -stdlib=libstdc++ -std=c++11 -I$(HOME)/v8 -Os

FILE=V8Simple
OBJ_DIR=obj
LIB_DIR=lib
LIB_FILE=lib$(FILE).dylib

all: $(LIB_DIR)/$(LIB_FILE)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	clang++ -c $(CPPFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.cxx
	@mkdir -p $(OBJ_DIR)
	clang++ -c $(CPPFLAGS) $< -o $@

$(LIB_DIR)/$(LIB_FILE): $(OBJ_DIR)/$(FILE).o $(OBJ_DIR)/$(FILE)_wrap.o
	@mkdir -p $(LIB_DIR)
	clang++ -shared $(CPPFLAGS) $(LDFLAGS) $^ -o $@

$(FILE)_wrap.cxx: $(FILE).i $(FILE).h
	swig -csharp -dllimport $(FILE) -namespace Fuse.Scripting.V8.Simple -c++ -outfile $(FILE).cs $<

.PHONY: clean

clean:
	$(RM) -r $(LIB_DIR)
	$(RM) -r $(OBJ_DIR)

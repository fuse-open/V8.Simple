LDFLAGS+= -fPIC -fno-rtti -Wl,--whole-archive -lv8_base -lv8_libbase -lv8_libplatform -lv8_nosnapshot -Wl,--no-whole-archive -dead_strip
CXXFLAGS+= -Wall -std=c++11

FILE=V8Simple
OBJ_DIR=obj
LIB_DIR=lib
LIB_FILE=lib$(FILE).dylib
ANDROID_LIB_FILE=lib$(FILE).so

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

$(LIB_DIR)/$(ANDROID_LIB_FILE): $(OBJ_DIR)/$(FILE).o
	@mkdir -p $(LIB_DIR)
	$(CXX) -shared $(CXXFLAGS) $(LDFLAGS) $^ -o $@

$(FILE).cs $(FILE)_wrap.cxx: $(FILE).i $(FILE).h
	swig -csharp -dllimport $(FILE) -namespace Fuse.Scripting.V8.Simple -c++ -outfile $(FILE).cs $<

$(LIB_DIR)/$(FILE).dll: $(FILE).cs
	mcs -t:library $^ -out:$@

.PHONY: clean

clean:
	rm $(FILE)_wrap.cxx
	rm  $(LIB_DIR)/*.so $(LIB_DIR)/*.dll
	rm -r $(OBJ_DIR)

LDFLAGS+= -fPIC -fno-rtti -dead_strip
CXXFLAGS+= -Wall -std=c++11

FILE=V8Simple
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
	$(RM) $(FILE)_wrap.cxx
	$(RM)  -r lib
	$(RM) -r obj

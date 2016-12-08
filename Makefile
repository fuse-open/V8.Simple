LDFLAGS+= -flto -fPIC -dead_strip
CXXFLAGS+= -fexceptions -Wall -std=c++11

FILE=V8Simple
LIB_DIR=lib
LIB_FILE=lib$(FILE).dylib
ANDROID_LIB_FILE=lib$(FILE).so

all: $(LIB_DIR)/$(LIB_FILE) $(LIB_DIR)/$(FILE).net.dll

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(LIB_DIR)/$(LIB_FILE): $(OBJ_DIR)/$(FILE).o
	@mkdir -p $(LIB_DIR)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(LIB_DIR)/$(ANDROID_LIB_FILE): $(OBJ_DIR)/$(FILE).o
	@mkdir -p $(LIB_DIR)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(FILE).cs: $(FILE).h
	./extract_pinvoke.sh $^ $@

$(LIB_DIR)/$(FILE).net.dll: $(FILE).cs DllDirectory.cs
	@mkdir -p $(LIB_DIR)
	mcs -t:library $^ -out:$@

.PHONY: clean check

check: $(LIB_DIR)/$(LIB_FILE) $(LIB_DIR)/$(FILE).net.dll
	cp $(LIB_DIR)/$(LIB_FILE) test
	cp $(LIB_DIR)/$(FILE).net.dll test
	mcs -t:library -lib:lib -r:V8Simple.net.dll,nunit.framework test/Test.cs
	nunit-console -labels test/Test.dll

clean:
	$(RM)  -r lib
	$(RM) -r obj

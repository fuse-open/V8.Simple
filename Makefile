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

$(LIB_DIR)/$(FILE).net.dll: $(FILE).net.csproj $(FILE).cs Metadata.cs
	@mkdir -p $(LIB_DIR)
	dotnet build $< -c Release -p OutputPath=$(LIB_DIR)

.PHONY: clean check

check: $(LIB_DIR)/$(LIB_FILE) $(LIB_DIR)/$(FILE).net.dll
	cp $(LIB_DIR)/$(LIB_FILE) test
	cp $(LIB_DIR)/$(FILE).net.dll test
	dotnet build test/Test.csproj -p OutputPath=.
	nunit-console -labels test/Test.dll

clean:
	$(RM) -r lib
	$(RM) -r obj

#pragma once
#include <include/v8.h>
#include <string>
#include <vector>

// Note: Every public function that returns a Value* (or derived class)
// requires that the caller takes ownership of and deletes the pointer when
// appropriate. The Values are allocated with `new`, so should be deallocated
// with `delete`.

namespace V8Simple
{

class Value;
class Object;
class Array;
class Function;
class Object;

class Context
{
public:
	Context();
	Value* Evaluate(const std::string& fileName, const std::string& code);
	Object* GlobalObject();
	~Context();
private:
	v8::Platform* _platform;
	static v8::Isolate* _isolate;
	v8::Persistent<v8::Context>* _context;
	static Function* _instanceOf;

	friend class Value;
	friend class Array;
	friend class Function;
	friend class Object;
};

enum class Type
{
	Int,
	Double,
	String,
	Bool,
	Object,
	Array,
	Function,
	Callback,
};

class Value
{
public:
	virtual Type GetType() = 0;
};

template<class T, Type type>
class Primitive: public Value
{
public:
	Primitive(const T& value) : _value(value) { }
	virtual Type GetType() { return type; }
	T GetValue() { return _value; }
private:
	T _value;
};

typedef Primitive<int, Type::Int> Int;
typedef Primitive<double, Type::Double> Double;
typedef Primitive<std::string, Type::String> String;
typedef Primitive<bool, Type::Bool> Bool;

class Object: public Value
{
public:
	virtual Type GetType() { return Type::Object; }
	Value* Get(const std::string& key);
	void Set(const std::string& key, const Value& value);
	std::vector<std::string> Keys();
	bool InstanceOf(Function& type);
	Value* CallMethod(const std::string& name, const std::vector<Value*>& args);
	bool ContainsKey(const std::string& key);
	bool Equals(const Object& object);
private:
	Object(v8::Local<v8::Object> object);
	v8::Persistent<v8::Object> _object;
	friend class Context;
	friend class Function;
};

class Function: public Value
{
public:
	virtual Type GetType() { return Type::Function; }
	Value* Call(const std::vector<Value*>& args);
	Object* Construct(const std::vector<Value*>& args);
	bool Equals(const Function& f);
private:
	Function(v8::Local<v8::Function> function);
	v8::Persistent<v8::Function> _function;
};

class Array: public Value
{
public:
	virtual Type GetType() { return Type::Array; }
	Value* Get(int index);
	void Set(int index, const Value& value);
	int Length();
	bool Equals(const Array& array);
private:
	Array(v8::Local<v8::Array> array);
	v8::Persistent<v8::Array> _array;
};

class Callback: public Value
{
public:
	virtual Type GetType() { return Type::Callback; }
	virtual Value* Call(const std::vector<Value*>& args) = 0;
};

} // namespace V8Simple

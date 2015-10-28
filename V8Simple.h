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

struct ScriptException
{
	std::string Name, ErrorMessage, FileName, StackTrace;
	int LineNumber;

	ScriptException(
		const std::string& name,
		const std::string& errorMessage,
		const std::string& fileName,
		int lineNumber,
		const std::string& stackTrace)
		: Name(name)
		, ErrorMessage(errorMessage)
		, FileName(fileName)
		, StackTrace(stackTrace)
		, LineNumber(lineNumber)
	{ }
};

struct Exception
{
	std::string Message;
	Exception(const std::string message)
		: Message(message)
	{ }
};

class Context
{
public:
	Context();
	Value* Evaluate(const std::string& fileName, const std::string& code)
		throw(ScriptException, Exception);
	Object* GlobalObject();
	~Context();
private:
	v8::Platform* _platform;
	static v8::Isolate* _isolate;
	v8::Persistent<v8::Context>* _context;
	static Function* _instanceOf;
	static Value* Wrap(
		const v8::TryCatch& tryCatch,
		v8::Local<v8::Value> value)
		throw(ScriptException, Exception);
	static Value* Wrap(
		const v8::TryCatch& tryCatch,
		v8::MaybeLocal<v8::Value> mvalue)
		throw(ScriptException, Exception);
	static v8::Local<v8::Value> Unwrap(
		const v8::TryCatch& tryCatch,
		const Value* value)
		throw(ScriptException);
	static std::vector<v8::Local<v8::Value>> UnwrapVector(
		const v8::TryCatch& tryCatch,
		const std::vector<Value*>& values)
		throw(ScriptException);
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
	virtual Type GetType() const = 0;
	virtual ~Value() { }
};

template<class T, Type type>
class Primitive: public Value
{
public:
	Primitive(const T& value) : _value(value) { }
	virtual Type GetType() const override { return type; }
	T GetValue() const { return _value; }
private:
	const T _value;
};

typedef Primitive<int, Type::Int> Int;
typedef Primitive<double, Type::Double> Double;
typedef Primitive<std::string, Type::String> String;
typedef Primitive<bool, Type::Bool> Bool;

class Object: public Value
{
public:
	virtual Type GetType() const override;
	Value* Get(const std::string& key)
		throw(ScriptException, Exception);
	void Set(const std::string& key, const Value& value)
		throw(ScriptException, Exception);
	std::vector<std::string> Keys()
		throw(ScriptException, Exception);
	bool InstanceOf(Function& type)
		throw(ScriptException, Exception);
	Value* CallMethod(const std::string& name, const std::vector<Value*>& args)
		throw(ScriptException, Exception);
	bool ContainsKey(const std::string& key)
		throw(ScriptException, Exception);
	bool Equals(const Object& object)
		throw(ScriptException, Exception);
private:
	Object(v8::Local<v8::Object> object);
	v8::Persistent<v8::Object> _object;
	friend class Context;
	friend class Function;
};

class Function: public Value
{
public:
	virtual Type GetType() const override;
	Value* Call(const std::vector<Value*>& args)
		throw(ScriptException, Exception);
	Object* Construct(const std::vector<Value*>& args)
		throw(ScriptException, Exception);
	bool Equals(const Function& f)
		throw(ScriptException, Exception);
private:
	friend class Context;
	Function(v8::Local<v8::Function> function);
	v8::Persistent<v8::Function> _function;
};

class Array: public Value
{
public:
	virtual Type GetType() const override;
	Value* Get(int index)
		throw(ScriptException, Exception);
	void Set(int index, const Value& value)
		throw(ScriptException, Exception);
	int Length()
		throw(ScriptException, Exception);
	bool Equals(const Array& array)
		throw(ScriptException, Exception);
private:
	friend class Context;
	Array(v8::Local<v8::Array> array);
	v8::Persistent<v8::Array> _array;
};

class Callback: public Value
{
public:
	virtual Type GetType() const override;
	virtual Value* Call(const std::vector<Value*>& args)
		throw(ScriptException, Exception) = 0;
	virtual Callback* Copy() const = 0;
private:
	friend class Context;
};

} // namespace V8Simple

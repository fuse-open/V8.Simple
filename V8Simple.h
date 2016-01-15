#pragma once
#include <include/v8.h>
#include <stdexcept>
#include <vector>

// Note: All public functions that return a Value* (or derived class), require
// that the caller takes ownership of and deletes the pointer when appropriate.
// The Values should be deallocated with the delete operator.

#ifdef _MSC_VER
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

namespace V8Simple
{

struct V8Scope;
class Object;

v8::Isolate* CurrentIsolate();
v8::Local<v8::Context> CurrentContext();
void Throw(const V8Scope& scope);

enum class DllExport Type
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

template<class T> struct TypeTag { };

class DllExport Value
{
public:
	virtual Type GetValueType() const = 0;
	virtual ~Value()
	{
	}
protected:
	static Value* Wrap(
		const V8Scope& scope,
		v8::Local<v8::Value> value);
	static Value* Wrap(
		const V8Scope& scope,
		v8::MaybeLocal<v8::Value> mvalue);
	static v8::Local<v8::Value> Unwrap(
		const V8Scope& scope,
		Value* value);
	std::vector<v8::Local<v8::Value>> UnwrapVector(
		const V8Scope& scope,
		const std::vector<Value*>& values);
	friend class Context;
};

class DllExport String: public Value
{
public:
	String(const char* value);
	String(const char* value, int length);
	String(const String& str);
	String& operator=(const String& str);
	virtual Type GetValueType() const override final;
	const char* GetValue() const;
	virtual ~String();
private:
	String(const v8::String::Utf8Value& v);
	char* _value;
	int _length;
	friend class Object;
	friend struct ScriptException;
};

class DllExport Function: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Call(const std::vector<Value*>& args);
	Object* Construct(const std::vector<Value*>& args);
	bool Equals(const Function* f);

	Function(const Function&) = delete;
	Function& operator=(const Function&) = delete;
private:
	Function(v8::Local<v8::Function> function);
	v8::Persistent<v8::Function> _function;
	friend class Value;
};

class DllExport Object: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Get(const char* key);
	void Set(const char* key, Value* value);
	std::vector<String> Keys();
	bool InstanceOf(Function* type);
	Value* CallMethod(const char* name, const std::vector<Value*>& args);
	bool ContainsKey(const char* key);
	bool Equals(const Object* object);

	Object(const Object&) = delete;
	Object& operator=(const Object&) = delete;
private:
	Object(v8::Local<v8::Object> object);
	v8::Persistent<v8::Object> _object;
	friend class Context;
	friend class Value;
	friend class Function;
};

class DllExport Array: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Get(int index);
	void Set(int index, Value* value);
	int Length();
	bool Equals(const Array* array);

	Array(const Array&) = delete;
	Array& operator=(const Array&) = delete;
private:
	Array(v8::Local<v8::Array> array);
	v8::Persistent<v8::Array> _array;
	friend class Value;
};

class DllExport UniqueValueVector
{
public:
	int Length();
	Value* Get(int index);
private:
	UniqueValueVector(std::vector<Value*>& values);
	std::vector<Value*>& _values;
	friend class Value;
};

class DllExport Callback: public Value
{
public:
	Callback();
	virtual Type GetValueType() const override /* final */;
	virtual Value* Call(UniqueValueVector args);
	virtual void Retain() { }
	virtual void Release() { }
};

template<class T>
class DllExport Primitive: public Value
{
public:
	Primitive(const T& value) : _value(value) { }
	virtual Type GetValueType() const override final { return TypeTag<Primitive<T>>::Tag; }
	T GetValue() const { return _value; }
private:
	const T _value;
};

typedef Primitive<int> Int;
typedef Primitive<double> Double;
typedef Primitive<bool> Bool;

template<> struct TypeTag<Object> { static const Type Tag = Type::Object; };
template<> struct TypeTag<Array> { static const Type Tag = Type::Array; };
template<> struct TypeTag<Function> { static const Type Tag = Type::Function; };
template<> struct TypeTag<Callback> { static const Type Tag = Type::Callback; };
template<> struct TypeTag<Int> { static const Type Tag = Type::Int; };
template<> struct TypeTag<Double> { static const Type Tag = Type::Double; };
template<> struct TypeTag<String> { static const Type Tag = Type::String; };
template<> struct TypeTag<Bool> { static const Type Tag = Type::Bool; };

struct DllExport ScriptException
{
	String* GetName() { return new String(Name); }
	String* GetErrorMessage() { return new String(ErrorMessage); }
	String* GetFileName() { return new String(FileName); }
	int GetLineNumber() { return LineNumber; }
	String* GetStackTrace() { return new String(StackTrace); }
	String* GetSourceLine() { return new String(SourceLine); }
private:
	const String Name, ErrorMessage, FileName, StackTrace, SourceLine;
	int LineNumber;

	ScriptException(
		const ::v8::String::Utf8Value& name,
		const ::v8::String::Utf8Value& errorMessage,
		const ::v8::String::Utf8Value& fileName,
		int lineNumber,
		const ::v8::String::Utf8Value& stackTrace,
		const ::v8::String::Utf8Value& sourceLine);
	friend void Throw(const V8Scope& scope);
};

struct DllExport MessageHandler
{
	virtual void Handle(const String& message) { }
	virtual ~MessageHandler() { }
	virtual void Retain() { }
	virtual void Release() { }
};

struct DllExport ScriptExceptionHandler
{
	virtual void Handle(const ScriptException& e) { }
	virtual ~ScriptExceptionHandler() { }
	virtual void Retain() { }
	virtual void Release() { }
};

class DllExport Context
{
public:
	Context(ScriptExceptionHandler* scriptExceptionHandler, MessageHandler* runtimeExceptionHandler);
	Value* Evaluate(const char* fileName, const char* code);
	Object* GlobalObject();
	bool IdleNotificationDeadline(double deadline_in_seconds);
	~Context();

	static void SetDebugMessageHandler(MessageHandler* debugMessageHandler);
	static void SendDebugCommand(const char* command);
	static void ProcessDebugMessages();
private:
	static Context* _globalContext;
	static v8::Platform* _platform;

	MessageHandler* _debugMessageHandler;
	v8::Isolate* _isolate;
	v8::Isolate* _conversionIsolate;
	v8::Persistent<v8::Context>* _context;
	Function* _instanceOf;
	ScriptExceptionHandler* _scriptExceptionHandler;
	MessageHandler* _runtimeExceptionHandler;

	static void HandleScriptException(
		const ScriptException& e);
	static void HandleRuntimeException(
		const char* e);

	friend v8::Isolate* CurrentIsolate();
	friend v8::Local<v8::Context> CurrentContext();
	friend struct V8Scope;
	friend class Value;
	friend class Array;
	friend class Function;
	friend class Object;
};

} // namespace V8Simple

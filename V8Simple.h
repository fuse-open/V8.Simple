#pragma once
#include <include/v8.h>
#include <stdexcept>
#include <vector>

// Note: All public functions that return a Value* (or derived class), require
// that the caller takes ownership of and deletes the pointer when appropriate.
// The Values should be deallocated with the Delete functions (for maximum
// portability).

#ifdef _MSC_VER
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif

namespace V8Simple
{

typedef unsigned char byte;
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
	virtual void Delete();
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
	String(const byte* buffer, int bufferLength);
	static String* New(const byte* buffer, int bufferLength);
	String(const String& str);
	String& operator=(const String& str);
	virtual Type GetValueType() const override final;
	const byte* GetValue() const;
	void GetBuffer(byte* outBuffer) const;
	int GetBufferLength() const;
	virtual ~String();
	String* Copy() const;
private:
	String(const v8::String::Utf8Value& v);
	byte* _value;
	int _length;
	friend class Object;
	friend class Value;
	friend struct ScriptException;
};

class DllExport Function: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Call(const std::vector<Value*>& args);
	Value* Call(Value** args, int numArgs);
	Object* Construct(const std::vector<Value*>& args);
	Object* Construct(Value** args, int numArgs);
	bool StrictEquals(const Function* f);

	virtual ~Function();
private:
	Function(v8::Local<v8::Function> function);
	v8::Persistent<v8::Function>* _function;
	friend class Value;
};

class DllExport UniqueValueVector
{
public:
	int Length();
	Value* Get(int index);
	~UniqueValueVector();
	void Delete();
private:
	UniqueValueVector(const std::vector<Value*>&& values);
	std::vector<Value*>* _values;
	friend class Value;
	friend class Object;
};

class DllExport Object: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Get(const String* key);
	void Set(const String* key, Value* value);
	UniqueValueVector* Keys();
	bool InstanceOf(Function* type);
	Value* CallMethod(const String* name, const std::vector<Value*>& args);
	Value* CallMethod(const String* name, Value** args, int numArgs);
	bool ContainsKey(const String* key);
	bool StrictEquals(const Object* object);

	virtual ~Object();
private:
	Object(v8::Local<v8::Object> object);
	v8::Persistent<v8::Object>* _object;
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
	bool StrictEquals(const Array* array);

	virtual ~Array();
private:
	Array(v8::Local<v8::Array> array);
	v8::Persistent<v8::Array>* _array;
	friend class Value;
};

class DllExport Callback: public Value
{
public:
	Callback();
	virtual Type GetValueType() const override /* final */;
	virtual Value* Call(UniqueValueVector* args);
	virtual void Retain() { }
	virtual void Release() { }
};

template<class T>
class DllExport Primitive: public Value
{
public:
	Primitive(const T& value) : _value(value) { }
	static Primitive<T>* New(const T& value)
	{
		return new Primitive<T>(value);
	}
	virtual Type GetValueType() const override final { return TypeTag<Primitive<T>>::Tag; }
	T GetValue() const { return _value; }
	virtual void Delete() override
	{
		delete this;
	}
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
	String* GetName() { return Name.Copy(); }
	String* GetErrorMessage() { return ErrorMessage.Copy(); }
	String* GetFileName() { return FileName.Copy(); }
	int GetLineNumber() { return LineNumber; }
	String* GetStackTrace() { return StackTrace.Copy(); }
	String* GetSourceLine() { return SourceLine.Copy(); }
	ScriptException* Copy() const;
	void Delete();

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
	virtual void Handle(const String* message) { }
	virtual ~MessageHandler() { }
};

struct DllExport ScriptExceptionHandler
{
	virtual void Handle(const ScriptException& e) { }
	virtual ~ScriptExceptionHandler() { }
};

class DllExport Context
{
public:
	Context(ScriptExceptionHandler* scriptExceptionHandler, MessageHandler* runtimeExceptionHandler);
	static Context* New(ScriptExceptionHandler* scriptExceptionHandler, MessageHandler* runtimeExceptionHandler);
	Value* Evaluate(const String* fileName, const String* code);
	Object* GlobalObject();
	bool IdleNotificationDeadline(double deadlineInSeconds);
	~Context();
	void Delete();

	static const char* GetVersion();

	static void SetDebugMessageHandler(MessageHandler* debugMessageHandler);
	static void SendDebugCommand(const String* command);
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
		const char* messageBuffer);

	friend v8::Isolate* CurrentIsolate();
	friend v8::Local<v8::Context> CurrentContext();
	friend struct V8Scope;
	friend class Value;
	friend class Array;
	friend class Function;
	friend class Object;
};

} // namespace V8Simple

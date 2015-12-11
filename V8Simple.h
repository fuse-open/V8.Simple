#pragma once
#include <include/v8.h>
// TODO remove
// #include <iostream>
#include <stdexcept>
#include <vector>

// Note: All public functions that return a Value* (or derived class), require
// that the caller takes ownership of and deletes the pointer when appropriate.
// The Values should be deallocated with the delete operator.

namespace V8Simple
{

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

// TODO remove
// const std::string TypeNames[] = { "Int", "Double", "String", "Bool", "Object", "Array", "Function", "Callback" };

template<class T> struct TypeTag { };

class Value
{
public:
	virtual Type GetValueType() const = 0;
	virtual ~Value()
	{
		// std::cout << "- " << this << std::endl;
	}
protected:
	// TODO remove
	Value(Type t)
	{
		// std::cout << "+ " << TypeNames[static_cast<int>(t)] << " " << this << std::endl;
	}
};

class String: public Value
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
	friend class Object;
	friend struct ScriptException;
	char* _value;
	int _length;
};

class Object;

class Function: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Call(const std::vector<Value*>& args);
	Object* Construct(const std::vector<Value*>& args);
	bool Equals(const Function& f);
	virtual ~Function()
	{
		// TODO remove
		// std::cout << "Function destructor " << this << " " << &_function << std::endl;
		// delete _function;
	}
protected:
	Function(v8::Local<v8::Function> function);
private:
	friend class Context;
	v8::Persistent<v8::Function> _function;
};

class Object: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Get(const char* key);
	void Set(const char* key, Value* value);
	std::vector<String> Keys();
	bool InstanceOf(Function& type);
	Value* CallMethod(const char* name, const std::vector<Value*>& args);
	bool ContainsKey(const char* key);
	bool Equals(const Object& object);
protected:
	Object(v8::Local<v8::Object> object);
private:
	v8::Persistent<v8::Object> _object;
	friend class Context;
	friend class Function;
};

class Array: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Get(int index);
	void Set(int index, Value* value);
	int Length();
	bool Equals(const Array& array);
protected:
	Array(v8::Local<v8::Array> array);
private:
	friend class Context;
	v8::Persistent<v8::Array> _array;
};

class UniqueValueVector
{
public:
	UniqueValueVector(const std::vector<Value*>&& values);
	~UniqueValueVector();
	int Length();
	Value* Get(int index);
private:
	friend class Context;
	std::vector<Value*> _values;
};

class Callback: public Value
{
public:
	Callback();
	virtual Type GetValueType() const override /* final */;
	virtual Value* Call(const UniqueValueVector& args);
	virtual void Retain() { }
	virtual void Release() { }
private:
	friend class Context;
};

template<class T>
class Primitive: public Value
{
public:
	Primitive(const T& value) : Value(TypeTag<Primitive<T>>::Tag), _value(value) { }
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

struct ScriptException
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
	friend class Context;
};

struct MessageHandler
{
	virtual void Handle(const char* jsonMessage) { }
	virtual ~MessageHandler() { }
	virtual void Retain() { }
	virtual void Release() { }
};

struct ScriptExceptionHandler
{
	virtual void Handle(const ScriptException& e) { }
	virtual ~ScriptExceptionHandler() { }
	virtual void Retain() { }
	virtual void Release() { }
};

struct V8Scope;

class Context
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
	static MessageHandler* _debugMessageHandler;
	static v8::Platform* _platform;
	static v8::Isolate* _isolate;
	static v8::Persistent<v8::Context>* _context;
	static Function* _instanceOf;
	static ScriptExceptionHandler* _scriptExceptionHandler;
	static MessageHandler* _runtimeExceptionHandler;
	static Value* Wrap(
		const V8Scope& scope,
		v8::Local<v8::Value> value)
		throw(std::runtime_error);
	static Value* Wrap(
		const V8Scope& scope,
		v8::MaybeLocal<v8::Value> mvalue)
		throw(std::runtime_error);
	static v8::Local<v8::Value> Unwrap(
		const V8Scope& scope,
		Value* value);
	static std::vector<v8::Local<v8::Value>> UnwrapVector(
		const V8Scope& scope,
		const std::vector<Value*>& values);
	static void Throw(
		const V8Scope& scope);
	template<class A>
	static v8::Local<A> FromJust(
		const V8Scope& scope,
		v8::MaybeLocal<A> a);
	template<class A>
	static A FromJust(
		const V8Scope& scope,
		v8::Maybe<A> a);
	static void HandleScriptException(
		const ScriptException& e);
	static void HandleRuntimeException(
		const char* e);

	friend class Value;
	friend class Array;
	friend class Function;
	friend class Object;
	friend struct V8Scope;
};

} // namespace V8Simple

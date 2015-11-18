#pragma once
#include <include/v8.h>
// TODO remove
#include <iostream>
#include <stdexcept>
#include <vector>

// Note: All public functions that return a Value* (or derived class), require
// that the caller takes ownership of and deletes the pointer when appropriate.
// The Values should be deallocated with Value::Deallocate.

namespace V8Simple
{

class Value;
class Object;
class Array;
class Function;
class Object;
class Callback;

struct ScriptException
{
	const char* GetName() { return Name; }
	const char* GetErrorMessage() { return ErrorMessage; }
	const char* GetFileName() { return FileName; }
	int GetLineNumber() { return LineNumber; }
	const char* GetStackTrace() { return StackTrace; }
	const char* GetSourceLine() { return SourceLine; }

	~ScriptException();
private:
	const char *Name, *ErrorMessage, *FileName, *StackTrace, *SourceLine;
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

struct DebugMessageHandler;
struct ScriptExceptionHandler;

class Context
{
public:
	Context(ScriptExceptionHandler* scriptExceptionHandler)
		throw(std::runtime_error);
	Value* Evaluate(const char* fileName, const char* code)
		throw(std::runtime_error);
	Object* GlobalObject();
	bool IdleNotificationDeadline(double deadline_in_seconds);
	~Context();

	static void SetDebugMessageHandler(DebugMessageHandler* debugMessageHandler);
	static void SendDebugCommand(const char* command);
	static void ProcessDebugMessages();
private:
	static DebugMessageHandler* _debugMessageHandler;
	static v8::Platform* _platform;
	static v8::Isolate* _isolate;
	v8::Persistent<v8::Context>* _context;
	static Function* _instanceOf;
	static ScriptExceptionHandler* _scriptExceptionHandler;
	static Value* Wrap(
		const v8::TryCatch& tryCatch,
		v8::Local<v8::Value> value)
		throw(std::runtime_error);
	static Value* Wrap(
		const v8::TryCatch& tryCatch,
		v8::MaybeLocal<v8::Value> mvalue)
		throw(std::runtime_error);
	static v8::Local<v8::Value> Unwrap(
		const v8::TryCatch& tryCatch,
		Value* value);
	static std::vector<v8::Local<v8::Value>> UnwrapVector(
		const v8::TryCatch& tryCatch,
		const std::vector<Value*>& values);
	static void Throw(
		v8::Local<v8::Context> context,
		const v8::TryCatch& tryCatch);
	template<class A>
	static v8::Local<A> FromJust(
		v8::Local<v8::Context> context,
		const v8::TryCatch& tryCatch,
		v8::MaybeLocal<A> a);
	template<class A>
	static A FromJust(
		v8::Local<v8::Context> context,
		const v8::TryCatch& tryCatch,
		v8::Maybe<A> a);
	static void HandleScriptException(
		const ScriptException& e);

	friend class Value;
	friend class Array;
	friend class Function;
	friend class Object;
};

struct DebugMessageHandler
{
	virtual void Handle(const char* jsonMessage) = 0;
	virtual ~DebugMessageHandler() { }
	virtual void Retain() const { }
	virtual void Release() const { }
};

struct ScriptExceptionHandler
{
	virtual void Handle(const ScriptException& e) = 0;
	virtual ~ScriptExceptionHandler() { }
	virtual void Retain() const { }
	virtual void Release() const { }
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

// TODO remove
const std::string TypeNames[] = { "Int", "Double", "String", "Bool", "Object", "Array", "Function", "Callback" };

template<class T> struct TypeTag { };

class Value
{
public:
	virtual Type GetValueType() const = 0;
	/*
	template<typename T>
	T* As()
	{
		std::cout << "cast " << TypeNames[static_cast<int>(GetValueType())] << " " << this << std::endl;
		if (GetValueType() == TypeTag<T>::Tag)
		{
			return new T(*static_cast<T*>(this));
		}
		return nullptr;
	}
	*/
	virtual ~Value()
	{
		std::cout << "- " << this << std::endl;
	}
protected:
	// TODO remove
	Value(Type t)
	{
		std::cout << "+ " << TypeNames[static_cast<int>(t)] << " " << this << std::endl;
	}
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

class String: public Value
{
public:
	String(const char* value);
	String(const char* value, int length);
	String(const String& str);
	virtual Type GetValueType() const override final;
	const char* GetValue() const;
	virtual ~String();
private:
	char* _value;
	const int _length;
};

template<> struct TypeTag<Object> { static const Type Tag = Type::Object; };
template<> struct TypeTag<Array> { static const Type Tag = Type::Array; };
template<> struct TypeTag<Function> { static const Type Tag = Type::Function; };
template<> struct TypeTag<Callback> { static const Type Tag = Type::Callback; };
template<> struct TypeTag<Int> { static const Type Tag = Type::Int; };
template<> struct TypeTag<Double> { static const Type Tag = Type::Double; };
template<> struct TypeTag<String> { static const Type Tag = Type::String; };
template<> struct TypeTag<Bool> { static const Type Tag = Type::Bool; };


class Object: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Get(const char* key)
		throw(std::runtime_error);
	void Set(const char* key, Value& value);
	std::vector<const char*> Keys();
	bool InstanceOf(Function& type)
		throw(std::runtime_error);
	Value* CallMethod(const char* name, const std::vector<Value*>& args)
		throw(std::runtime_error);
	bool ContainsKey(const char* key);
	bool Equals(const Object& object);
protected:
	Object(v8::Local<v8::Object> object);
private:
	v8::Persistent<v8::Object> _object;
	friend class Context;
	friend class Function;
};

class Function: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Call(const std::vector<Value*>& args)
		throw(std::runtime_error);
	Object* Construct(const std::vector<Value*>& args);
	bool Equals(const Function& f);
	virtual ~Function()
	{
		// TODO remove
		std::cout << "Function destructor " << this << " " << &_function << std::endl;
		// delete _function;
	}
protected:
	Function(v8::Local<v8::Function> function);
private:
	friend class Context;
	v8::Persistent<v8::Function> _function;
};

class Array: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Get(int index)
		throw(std::runtime_error);
	void Set(int index, Value& value);
	int Length();
	bool Equals(const Array& array);
protected:
	Array(v8::Local<v8::Array> array);
private:
	friend class Context;
	v8::Persistent<v8::Array> _array;
};

class Callback: public Value
{
public:
	Callback();
	virtual Type GetValueType() const override;
	virtual Value* Call(const std::vector<Value*>& args)
		throw(std::runtime_error);
	virtual void Retain() const { }
	virtual void Release() const { }
private:
	friend class Context;
};

} // namespace V8Simple

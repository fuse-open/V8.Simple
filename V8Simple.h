#pragma once
#include <include/v8.h>
#include <string>
#include <vector>
#include <iostream>

// Note: All public functions that return a Value* (or derived class), save for
// Value::As<T>(), require that the caller takes ownership of and delete the
// pointer when appropriate. The Values are allocated with `new`, so should be
// deallocated with `delete`.

namespace V8Simple
{

class Value;
class Object;
class Array;
class Function;
class Object;
class Callback;

struct ScriptException: std::exception
{
	std::string Name, ErrorMessage, FileName, StackTrace, SourceLine;
	int LineNumber;

	ScriptException(
		const std::string& name,
		const std::string& errorMessage,
		const std::string& fileName,
		int lineNumber,
		const std::string& stackTrace,
		const std::string& sourceLine)
		: Name(name)
		, ErrorMessage(errorMessage)
		, FileName(fileName)
		, StackTrace(stackTrace)
		, SourceLine(sourceLine)
		, LineNumber(lineNumber)
	{ }

	virtual const char* what() const throw()
	{
		// TODO
		return Name.c_str();
	}
};

struct Exception: std::exception
{
	std::string Message;
	Exception(const std::string message)
		: Message(message)
	{ }

	virtual const char* what() const throw()
	{
		return Message.c_str();
	}
};

struct MessageHandler;

class Context
{
public:
	Context()
		throw(Exception);
	Value* Evaluate(const std::string& fileName, const std::string& code)
		throw(ScriptException, Exception);
	Object* GlobalObject();
	~Context();

	struct Debug
	{
		static void SetMessageHandler(MessageHandler* messageHandler);
		static void SendCommand(std::string command);
		static void ProcessDebugMessages();
	private:
		static MessageHandler* _messageHandler;
	};
private:
	static v8::Platform* _platform;
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

struct MessageHandler
{
	virtual void Handle(std::string jsonMessage) = 0;
	virtual ~MessageHandler() { };
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

template<class T> struct TypeTag { };

class Value
{
public:
	virtual Type GetValueType() const = 0;
	template<typename T>
	T* As()
	{
		if (GetValueType() == TypeTag<T>::Tag)
		{
			return static_cast<T*>(this);
		}
		return nullptr;
	}
	virtual ~Value()
	{
		std::cout << "Destructing value " << this << std::endl;
	}
};

template<class T>
class Primitive: public Value
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
typedef Primitive<std::string> String;
typedef Primitive<bool> Bool;

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
	Value* Get(const std::string& key)
		throw(ScriptException, Exception);
	void Set(const std::string& key, const Value& value)
		throw(ScriptException);
	std::vector<std::string> Keys()
		throw(ScriptException);
	bool InstanceOf(Function& type)
		throw(ScriptException, Exception);
	Value* CallMethod(const std::string& name, const std::vector<Value*>& args)
		throw(ScriptException, Exception);
	bool ContainsKey(const std::string& key)
		throw(ScriptException);
	bool Equals(const Object& object)
		throw(ScriptException);
private:
	Object(v8::Local<v8::Object> object);
	v8::Persistent<v8::Object> _object;
	friend class Context;
	friend class Function;
};

class Function: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Call(const std::vector<Value*>& args)
		throw(ScriptException, Exception);
	Object* Construct(const std::vector<Value*>& args)
		throw(ScriptException);
	bool Equals(const Function& f)
		throw(ScriptException);
private:
	friend class Context;
	Function(v8::Local<v8::Function> function);
	v8::Persistent<v8::Function> _function;
};

class Array: public Value
{
public:
	virtual Type GetValueType() const override final;
	Value* Get(int index)
		throw(ScriptException, Exception);
	void Set(int index, const Value& value)
		throw(ScriptException);
	int Length();
	bool Equals(const Array& array)
		throw(ScriptException);
private:
	friend class Context;
	Array(v8::Local<v8::Array> array);
	v8::Persistent<v8::Array> _array;
};

class Callback: public Value
{
public:
	Callback();
	virtual Type GetValueType() const override;
	virtual Value* Call(const std::vector<Value*>& args)
		throw(ScriptException, Exception);
	virtual Callback* Copy() const
		throw(Exception);
private:
	friend class Context;
};

} // namespace V8Simple

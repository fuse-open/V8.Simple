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
class Context;
class String;

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
	External,
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
		v8::Local<v8::Value> value);
	static Value* Wrap(
		const v8::TryCatch& tryCatch,
		v8::Local<v8::Value> value);
	static Value* WrapMaybe(
		const v8::TryCatch& tryCatch,
		v8::MaybeLocal<v8::Value> value);
	static v8::Local<v8::Value> Unwrap(Value* value);
	std::vector<v8::Local<v8::Value>> UnwrapVector(const std::vector<Value*>& values);
	static v8::Local<v8::String> ToV8String(const v8::TryCatch& tryCatch, const String& str);
	static v8::Local<v8::String> ToV8String(const String& str);
	friend class Context;
	friend void Throw(const v8::TryCatch& tryCatch);
};

class DllExport String: public Value
{
public:
	static String* New(const byte* buffer, int bufferLength);
	String(const byte* buffer, int bufferLength);
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
	friend class Context;
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
	v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>* _function;
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
	void* GetArrayBufferData();
	bool StrictEquals(const Object* object);

	virtual ~Object();
private:
	Object(v8::Local<v8::Object> object);
	v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object> >* _object;
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
	v8::Persistent<v8::Array, v8::CopyablePersistentTraits<v8::Array> >* _array;
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

class DllExport External: public Value
{
public:
	External(void* value);
	static External* New(void* value);
	virtual Type GetValueType() const override;
	void* GetValue();

	virtual ~External();
private:
	External(v8::Local<v8::External> external);
	v8::Persistent<v8::External, v8::CopyablePersistentTraits<v8::External> >* _external;
	friend class Value;
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
template<> struct TypeTag<External> { static const Type Tag = Type::External; };
template<> struct TypeTag<Int> { static const Type Tag = Type::Int; };
template<> struct TypeTag<Double> { static const Type Tag = Type::Double; };
template<> struct TypeTag<String> { static const Type Tag = Type::String; };
template<> struct TypeTag<Bool> { static const Type Tag = Type::Bool; };

struct DllExport ScriptException
{
	Value* GetException() { auto result = _exception; _exception = nullptr; return result; }
	String* GetErrorMessage() { return _errorMessage.Copy(); }
	String* GetFileName() { return _fileName.Copy(); }
	int GetLineNumber() { return _lineNumber; }
	String* GetStackTrace() { return _stackTrace.Copy(); }
	String* GetSourceLine() { return _sourceLine.Copy(); }
	ScriptException* Copy();
	~ScriptException();
	void Delete();
private:
	Value* _exception;
	const String _errorMessage, _fileName, _stackTrace, _sourceLine;
	int _lineNumber;

	ScriptException(
		Value* exception,
		const ::v8::String::Utf8Value& errorMessage,
		const ::v8::String::Utf8Value& fileName,
		int lineNumber,
		const ::v8::String::Utf8Value& stackTrace,
		const ::v8::String::Utf8Value& sourceLine);
	friend void Throw(const v8::TryCatch& tryCatch);
	friend class Value;
};

struct DllExport MessageHandler
{
	virtual void Handle(const String* message) { }
	virtual ~MessageHandler() { }
};

struct DllExport ScriptExceptionHandler
{
	virtual void Handle(ScriptException& e) { }
	virtual ~ScriptExceptionHandler() { }
};

struct DllExport ExternalFreer
{
	virtual void Free(void* external) { }
	virtual ~ExternalFreer() { }
};

class DllExport Context
{
public:
	Context(ScriptExceptionHandler* scriptExceptionHandler, MessageHandler* runtimeExceptionHandler, ExternalFreer* externalFreer);
	static Context* New(ScriptExceptionHandler* scriptExceptionHandler, MessageHandler* runtimeExceptionHandler, ExternalFreer* externalFreer);
	Value* Evaluate(const String* fileName, const String* code);
	Object* GlobalObject();
	bool IdleNotificationDeadline(double deadlineInSeconds);
	~Context();
	void Delete();

	static Object* NewExternalArrayBuffer(void* data, int byteLength);

	static const char* GetVersion();
	static Value* ThrowException(Value* exception);

	static void SetDebugMessageHandler(MessageHandler* debugMessageHandler);
	static void SendDebugCommand(const String* command);
	static void ProcessDebugMessages();
private:
	static v8::Platform* _platform;
	static v8::Isolate* _isolate;
	static v8::Isolate* _conversionIsolate;
	static Context* _globalContext;

	static MessageHandler* _debugMessageHandler;
	v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context> >* _context;
	Function* _instanceOf;
	ScriptExceptionHandler* _scriptExceptionHandler;
	MessageHandler* _runtimeExceptionHandler;
	ExternalFreer* _externalFreer;

	void HandleScriptException(ScriptException& e) const;
	void HandleRuntimeException(const char* messageBuffer) const;

	v8::Local<v8::Context> V8Context() const;

	static Context* Global();
	static v8::Isolate* Isolate();
	friend struct V8Scope;
	friend class Value;
	friend class Array;
	friend class Function;
	friend class Object;
	friend class External;
	friend void Throw(const v8::TryCatch& tryCatch);
};

} // namespace V8Simple

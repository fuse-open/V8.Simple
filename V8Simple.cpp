#include "V8Simple.h"
#include <include/v8-debug.h>
#include <include/libplatform/libplatform.h>
#include <stdlib.h>
#include <string>

// TODO remove
#include <iostream>

namespace V8Simple
{

String::String(const char* value)
	: String(value, value == nullptr ? 0 : std::strlen(value))
{
}

String::String(const char* value, int length)
	: Value(Type::String)
	, _value(new char[length + 1])
	, _length(length)
{
	if (length > 0 && value != nullptr)
	{
		std::memcpy(_value, value, _length);
	}
	_value[_length] = 0;
}

String& String::operator=(const String& str)
{
	delete[] _value;
	_length = str._length;
	_value = new char[_length + 1];
	std::memcpy(_value, str._value, _length + 1);
	return *this;
}

String::String(const String& str)
	: String(str._value, str._length)
{
}

String::String(const v8::String::Utf8Value& v)
	: String(v.length() > 0 ? *v : nullptr, v.length())
{
}

Type String::GetValueType() const { return Type::String; }
const char* String::GetValue() const { return _value; }

String::~String()
{
	delete[] _value;
}

static v8::Local<v8::String> ToV8String(
	v8::Isolate* isolate,
	const char* str)
{
	return v8::String::NewFromUtf8(isolate, str);
}

template<class T>
static const char* ToString(const T& t)
{
	return *v8::String::Utf8Value(t);
}

Object::Object(v8::Local<v8::Object> object)
	: Value(Type::Object)
	, _object(Context::_isolate, object)
{ }

Type Object::GetValueType() const { return Type::Object; }

Value* Object::Get(const char* key)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		return Context::Wrap(
			tryCatch,
			_object.Get(Context::_isolate)->Get(
				Context::_isolate->GetCurrentContext(),
				ToV8String(Context::_isolate, key)));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
		return nullptr;
	}
	catch (const std::runtime_error& e)
	{
		Context::HandleRuntimeException(e.what());
		return nullptr;
	}
}

void Object::Set(const char* key, Value* value)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);
		auto context = Context::_isolate->GetCurrentContext();

		auto ret = _object.Get(Context::_isolate)->Set(
			context,
			ToV8String(Context::_isolate, key),
			Context::Unwrap(tryCatch, value));
		Context::FromJust(context, tryCatch, ret);
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
	}
}

std::vector<String> Object::Keys()
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);
		auto context = Context::_isolate->GetCurrentContext();

		auto propArr = Context::FromJust(
			context,
			tryCatch,
			_object.Get(Context::_isolate)->GetPropertyNames(context));

		auto length = propArr->Length();
		std::vector<String> result;
		result.reserve(length);
		for (int i = 0; i < static_cast<int>(length); ++i)
		{
			result.push_back(String(v8::String::Utf8Value(Context::FromJust(
				context,
				tryCatch,
				propArr->Get(context, static_cast<uint32_t>(i))))));
		}
		return result;
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
		return std::vector<String>();
	}
}

bool Object::InstanceOf(Function& type)
{
	Value* callResult = nullptr;
	try
	{
		std::vector<Value*> args;
		args.reserve(2);
		args.push_back(this);
		args.push_back(&type);
		callResult = Context::_instanceOf->Call(args);
		bool result = (callResult == nullptr || callResult->GetValueType() != Type::Bool)
			? false
			: static_cast<Bool*>(callResult)->GetValue();
		return result;
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
	}
	catch (const std::runtime_error& e)
	{
		Context::HandleRuntimeException(e.what());
	}
	finally
	{
		delete callResult;
	}
	return false;
}

// Workaround for pre-C++11
template<class T>
static T* DataPointer(std::vector<T>& v)
{
	if (v.size() == 0)
	{
		return nullptr;
	}
	return &v[0];
}

Value* Object::CallMethod(
	const char* name,
	const std::vector<Value*>& args)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		auto context = Context::_isolate->GetCurrentContext();
		auto localObject = _object.Get(Context::_isolate);
		auto fun = Context::FromJust(
			context,
			tryCatch,
			localObject->Get(
				context,
				ToV8String(Context::_isolate, name).As<v8::Value>()))
			.As<v8::Function>();
		auto unwrappedArgs = Context::UnwrapVector(tryCatch, args);
		return Context::Wrap(
			tryCatch,
			fun->Call(
				localObject,
				static_cast<int>(unwrappedArgs.size()),
				DataPointer(unwrappedArgs)));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
	}
	catch (const std::runtime_error& e)
	{
		Context::HandleRuntimeException(e.what());
	}
	return nullptr;
}

bool Object::ContainsKey(const char* key)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		auto context = Context::_isolate->GetCurrentContext();
		return Context::FromJust(
			context,
			tryCatch,
			_object.Get(Context::_isolate)->Has(
				context,
				ToV8String(Context::_isolate, key)));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
		return false;
	}
}

bool Object::Equals(const Object& o)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		auto context = Context::_isolate->GetCurrentContext();

		return Context::FromJust(
			context,
			tryCatch,
			_object.Get(Context::_isolate)->Equals(
				context,
				o._object.Get(Context::_isolate)));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
		return false;
	}
}

Function::Function(v8::Local<v8::Function> function)
	: Value(Type::Function)
	, _function(Context::_isolate, function)
{
}

Type Function::GetValueType() const { return Type::Function; }

Value* Function::Call(const std::vector<Value*>& args)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		auto context = Context::_isolate->GetCurrentContext();
		auto unwrappedArgs = Context::UnwrapVector(tryCatch, args);
		return Context::Wrap(
			tryCatch,
			_function.Get(Context::_isolate)->Call(
				context,
				context->Global(),
				static_cast<int>(unwrappedArgs.size()),
				DataPointer(unwrappedArgs)));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
	}
	catch (const std::runtime_error& e)
	{
		Context::HandleRuntimeException(e.what());
	}
	return nullptr;
}

Object* Function::Construct(const std::vector<Value*>& args)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		auto context = Context::_isolate->GetCurrentContext();
		auto unwrappedArgs = Context::UnwrapVector(tryCatch, args);

		return new Object(
			Context::FromJust(
				context,
				tryCatch,
				_function.Get(Context::_isolate)->NewInstance(
					context,
					unwrappedArgs.size(),
					DataPointer(unwrappedArgs))));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
	}
	return nullptr;
}

bool Function::Equals(const Function& function)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		auto context = Context::_isolate->GetCurrentContext();

		return Context::FromJust(
			context,
			tryCatch,
			_function.Get(Context::_isolate)->Equals(
				context,
				function._function.Get(Context::_isolate)));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
	}
	return false;
}

Array::Array(v8::Local<v8::Array> array)
	: Value(Type::Array)
	, _array(Context::_isolate, array)
{ }

Type Array::GetValueType() const { return Type::Array; }

Value* Array::Get(int index)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		auto context = Context::_isolate->GetCurrentContext();
		return Context::Wrap(
			tryCatch,
			_array.Get(Context::_isolate)->Get(
				context,
				static_cast<uint32_t>(index)));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
		return nullptr;
	}
	catch (const std::runtime_error& e)
	{
		Context::HandleRuntimeException(e.what());
		return nullptr;
	}
}

void Array::Set(int index, Value* value)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		auto context = Context::_isolate->GetCurrentContext();
		Context::FromJust(
			context,
			tryCatch,
			_array.Get(Context::_isolate)->Set(
				context,
				static_cast<uint32_t>(index),
				Context::Unwrap(tryCatch, value)));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
	}
}

int Array::Length()
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);

	return static_cast<int>(_array.Get(Context::_isolate)->Length());
}

bool Array::Equals(const Array& array)
{
	try
	{
		v8::Isolate::Scope isolateScope(Context::_isolate);
		v8::HandleScope handleScope(Context::_isolate);
		v8::TryCatch tryCatch(Context::_isolate);

		auto context = Context::_isolate->GetCurrentContext();
		return Context::FromJust(
			context,
			tryCatch,
			_array.Get(Context::_isolate)->Equals(
				context,
				array._array.Get(Context::_isolate)));
	}
	catch (const ScriptException& e)
	{
		Context::HandleScriptException(e);
		return false;
	}

}

UniqueValueVector::UniqueValueVector(const std::vector<Value*>&& values)
	: _values(values)
{
}

UniqueValueVector::~UniqueValueVector()
{
	for (Value* v: _values)
	{
		delete v;
	}
}

int UniqueValueVector::Length()
{
	return _values.size();
}

Value* UniqueValueVector::Get(int index)
{
	Value* result = nullptr;
	std::swap(_values.at(index), result);
	return result;
}

Callback::Callback()
	: Value(Type::Callback)
{
}

Value* Callback::Call(const UniqueValueVector& args)
{
	return nullptr;
}

Type Callback::GetValueType() const { return Type::Callback; }

Function* Context::_instanceOf = nullptr;
v8::Isolate* Context::_isolate = nullptr;
v8::Platform* Context::_platform = nullptr;
MessageHandler* Context::_debugMessageHandler = nullptr;
ScriptExceptionHandler* Context::_scriptExceptionHandler = nullptr;
MessageHandler* Context::_runtimeExceptionHandler = nullptr;

struct ArrayBufferAllocator: ::v8::ArrayBuffer::Allocator
{
	virtual void* Allocate(size_t length)
	{
		return calloc(length, 1);
	}

	virtual void* AllocateUninitialized(size_t length)
	{
		return malloc(length);
	}

	virtual void Free(void* data, size_t)
	{
		free(data);
	}
};

ScriptException::ScriptException(
	const ::v8::String::Utf8Value& name,
	const ::v8::String::Utf8Value& errorMessage,
	const ::v8::String::Utf8Value& fileName,
	int lineNumber,
	const ::v8::String::Utf8Value& stackTrace,
	const ::v8::String::Utf8Value& sourceLine)
	: Name(name)
	, ErrorMessage(errorMessage)
	, FileName(fileName)
	, StackTrace(stackTrace)
	, SourceLine(sourceLine)
	, LineNumber(lineNumber)
{
}

void Context::Throw(
	v8::Local<v8::Context> context,
	const v8::TryCatch& tryCatch)
{
	auto isolate = context->GetIsolate();
	v8::Local<v8::Value> emptyString = v8::String::Empty(isolate);

	auto message = tryCatch.Message();
	auto sourceLine(emptyString);
	auto messageStr(emptyString);
	auto fileName(emptyString);
	int lineNumber = -1;
	if (!message.IsEmpty())
	{
		sourceLine = message->GetSourceLine(context).FromMaybe(emptyString);
		auto messageStrLocal = message->Get();
		if (!messageStrLocal.IsEmpty())
		{
			messageStr = messageStrLocal;
		}
		fileName = message->GetScriptResourceName();
		lineNumber = message->GetLineNumber(context).FromMaybe(-1);
	}

	v8::Local<v8::Value> exception = tryCatch.Exception().IsEmpty()
		? emptyString
		: tryCatch.Exception();

	auto stackTrace(
		tryCatch
		.StackTrace(context)
		.FromMaybe(emptyString.As<v8::Value>()));

	throw ScriptException(
		v8::String::Utf8Value(exception),
		v8::String::Utf8Value(messageStr),
		v8::String::Utf8Value(fileName),
		lineNumber,
		v8::String::Utf8Value(stackTrace),
		v8::String::Utf8Value(sourceLine));
}

void Context::HandleScriptException(const ScriptException& e)
{
	if (_scriptExceptionHandler != nullptr)
	{
		_scriptExceptionHandler->Handle(e);
	}
}

void Context::HandleRuntimeException(const char* e)
{
	if (_runtimeExceptionHandler != nullptr)
	{
		_runtimeExceptionHandler->Handle(e);
	}
}

template<class A>
v8::Local<A> Context::FromJust(
	v8::Local<v8::Context> context,
	const v8::TryCatch& tryCatch,
	v8::MaybeLocal<A> a)
{
	if (a.IsEmpty())
	{
		Throw(context, tryCatch);
	}
	return a.ToLocalChecked();
}

template<class A>
A Context::FromJust(
	v8::Local<v8::Context> context,
	const v8::TryCatch& tryCatch,
	v8::Maybe<A> a)
{
	if (a.IsNothing())
	{
		Throw(context, tryCatch);
	}
	return a.FromJust();
}

Value* Context::Wrap(
	const v8::TryCatch& tryCatch,
	v8::Local<v8::Value> value) throw(std::runtime_error)
{
	auto context = _isolate->GetCurrentContext();
	if (value->IsInt32())
	{
		return new Int(FromJust(
			context,
			tryCatch,
			value->Int32Value(context)));
	}
	if (value->IsNumber() || value->IsNumberObject())
	{
		return new Double(FromJust(
			context,
			tryCatch,
			value->NumberValue(context)));
	}
	if (value->IsBoolean() || value->IsBooleanObject())
	{
		return new Bool(FromJust(
			context,
			tryCatch,
			value->BooleanValue(context)));
	}
	if (value->IsString() || value->IsStringObject())
	{
		v8::String::Utf8Value str(FromJust(
			context,
			tryCatch,
			value->ToString(context)));
		return new String(*str, str.length());
	}
	if (value->IsArray())
	{
		return new Array(FromJust(
			context,
			tryCatch,
			value->ToObject(context)).As<v8::Array>());
	}
	if (value->IsFunction())
	{
		return new Function(FromJust(
			context,
			tryCatch,
			value->ToObject(context)).As<v8::Function>());
	}
	if (value->IsObject())
	{
		return new Object(FromJust(
			context,
			tryCatch,
			value->ToObject(context)));
	}
	if (value->IsUndefined() || value->IsNull())
	{
		return nullptr;
	}
	throw std::runtime_error("Unhandled type in V8Simple");
}

Value* Context::Wrap(
	const v8::TryCatch& tryCatch,
	v8::MaybeLocal<v8::Value> mvalue) throw(std::runtime_error)
{
	auto context = _isolate->GetCurrentContext();
	return Wrap(tryCatch, FromJust(context, tryCatch, mvalue));
}

v8::Local<v8::Value> Context::Unwrap(
	const v8::TryCatch& tryCatch,
	Value* value)
{
	if (value == nullptr)
	{
		return v8::Null(_isolate).As<v8::Value>();
	}

	switch (value->GetValueType())
	{
		case Type::Int:
			return v8::Int32::New(
				_isolate,
				static_cast<Int*>(value)->GetValue());
		case Type::Double:
			return v8::Number::New(
				_isolate,
				static_cast<Double*>(value)->GetValue());
		case Type::String:
			return ToV8String(
				_isolate,
				static_cast<String*>(value)->GetValue());
		case Type::Bool:
			return v8::Boolean::New(
				_isolate,
				static_cast<Bool*>(value)->GetValue());
		case Type::Object:
			return static_cast<Object*>(value)
				->_object.Get(_isolate);
		case Type::Array:
			return static_cast<Array*>(value)
				->_array.Get(_isolate);
		case Type::Function:
			return static_cast<Function*>(value)
				->_function.Get(_isolate);
		case Type::Callback:
			Callback* callback = static_cast<Callback*>(value);
			callback->Retain();
			auto localCallback = v8::External::New(_isolate, callback);
			v8::Persistent<v8::External> persistentCallback(_isolate, localCallback);

			persistentCallback.SetWeak(
				callback,
				[] (const v8::WeakCallbackInfo<Callback>& data)
				{
					auto cb = data.GetParameter();
					cb->Release();
				},
				v8::WeakCallbackType::kParameter);

			auto context = _isolate->GetCurrentContext();
			return FromJust(context, tryCatch, v8::Function::New(
				context,
				[] (const v8::FunctionCallbackInfo<v8::Value>& info)
				{
					v8::HandleScope handleScope(info.GetIsolate());
					v8::TryCatch tryCatch(info.GetIsolate());

					std::vector<Value*> wrappedArgs;
					wrappedArgs.reserve(info.Length());
					try
					{
						for (int i = 0; i < info.Length(); ++i)
						{
							wrappedArgs.push_back(Wrap(
								tryCatch,
								info[i]));
						}
					}
					catch (const std::runtime_error& e)
					{
						Context::HandleRuntimeException(e.what());
					}

					Callback* callback =
						static_cast<Callback*>(info.Data()
							.As<v8::External>()
							->Value());
					Value* result = callback->Call(UniqueValueVector((std::vector<Value*>&&)wrappedArgs));

					info.GetReturnValue().Set(Unwrap(
						tryCatch,
						result));
				},
				localCallback.As<v8::Value>()));
		}
	return v8::Null(_isolate).As<v8::Value>();
}

std::vector<v8::Local<v8::Value>> Context::UnwrapVector(
	const v8::TryCatch& tryCatch,
	const std::vector<Value*>& values)
{
	std::vector<v8::Local<v8::Value>> result;
	result.reserve(values.size());
	for (Value* value: values)
	{
		result.push_back(Unwrap(tryCatch, value));
	}
	return result;
}

void Context::SetDebugMessageHandler(MessageHandler* debugMessageHandler)
{
	v8::Isolate::Scope isolateScope(_isolate);
	if (_debugMessageHandler != nullptr)
	{
		_debugMessageHandler->Release();
	}
	if (debugMessageHandler == nullptr)
	{
		v8::Debug::SetMessageHandler(nullptr);
	}
	else
	{
		v8::Debug::SetMessageHandler([] (const v8::Debug::Message& message)
		{
			_debugMessageHandler->Handle(ToString(message.GetJSON()));
		});
		debugMessageHandler->Retain();
	}
	_debugMessageHandler = debugMessageHandler;
}

void Context::SendDebugCommand(const char* command)
{
	v8::Isolate::Scope isolateScope(_isolate);
	v8::HandleScope handleScope(_isolate);

	auto str = ToV8String(_isolate, command);
	auto len = str->Length();
	uint16_t* buffer = new uint16_t[len];
	str->Write(buffer);
	v8::Debug::SendCommand(_isolate, buffer, len);
	delete[] buffer;
}

void Context::ProcessDebugMessages()
{
	v8::Isolate::Scope isolateScope(_isolate);
	v8::Debug::ProcessDebugMessages();
}

Context::Context(ScriptExceptionHandler* scriptExceptionHandler, MessageHandler* runtimeExceptionHandler)
{
	_scriptExceptionHandler = scriptExceptionHandler;
	if (_scriptExceptionHandler != nullptr)
	{
		_scriptExceptionHandler->Retain();
	}

	_runtimeExceptionHandler = runtimeExceptionHandler;
	if (_runtimeExceptionHandler != nullptr)
	{
		_runtimeExceptionHandler->Retain();
	}

	// TODO remove
	v8::V8::SetFlagsFromString("--expose-gc", 11);
	if (_platform == nullptr)
	{
		v8::V8::InitializeICU();
		_platform = v8::platform::CreateDefaultPlatform();
		v8::V8::InitializePlatform(_platform);
		v8::V8::Initialize();
	}

	if (_isolate == nullptr)
	{
		v8::Isolate::CreateParams createParams;
		static ArrayBufferAllocator arrayBufferAllocator;
		createParams.array_buffer_allocator = &arrayBufferAllocator;
		_isolate = v8::Isolate::New(createParams);
	}
	else
	{
		Context::HandleRuntimeException("V8Simple Contexts are not re-entrant");
	}

	v8::Isolate::Scope isolateScope(_isolate);
	v8::HandleScope handleScope(_isolate);

	auto localContext = v8::Context::New(_isolate);
	localContext->Enter();

	_context = new v8::Persistent<v8::Context>(_isolate, localContext);
	
	Value* instanceOf = Evaluate(
		"instanceof",
		"(function(x, y) { return (x instanceof y); })");
	if (!instanceOf || instanceOf->GetValueType() != Type::Function)
	{
		Context::HandleRuntimeException("V8Simple could not create an instanceof function");
	}
	else
	{
		_instanceOf = static_cast<Function*>(instanceOf);
	}
}

Context::~Context()
{
	delete _instanceOf;
	_instanceOf = nullptr;

	if (_runtimeExceptionHandler != nullptr)
	{
		_runtimeExceptionHandler->Release();
	}
	_runtimeExceptionHandler = nullptr;

	if (_runtimeExceptionHandler != nullptr)
	{
		_scriptExceptionHandler->Release();
	}
	_scriptExceptionHandler = nullptr;

	{
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		_context->Get(_isolate)->Exit();
	}
	delete _context;
	_context = nullptr;

	_isolate->Dispose();
	_isolate = nullptr;

	// If we do this we can't create a new context afterwards.
	//
	// v8::V8::Dispose();
	// v8::V8::ShutdownPlatform();
	// delete _platform;
}

Value* Context::Evaluate(const char* fileName, const char* code)
{
	try
	{
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		v8::TryCatch tryCatch(_isolate);
		auto context = _context->Get(_isolate);

		v8::ScriptOrigin origin(ToV8String(_isolate, fileName));
		auto script = FromJust(
			context,
			tryCatch,
			v8::Script::Compile(
				context,
				ToV8String(_isolate, code),
				&origin));

		return Wrap(tryCatch, script->Run(context));
	}
	catch (const ScriptException& e)
	{
		HandleScriptException(e);
		return nullptr;
	}
	catch (const std::runtime_error& e)
	{
		HandleRuntimeException(e.what());
		return nullptr;
	}
}

Object* Context::GlobalObject()
{
	v8::Isolate::Scope isolateScope(_isolate);
	v8::HandleScope handleScope(_isolate);

	return new Object(_context->Get(_isolate)->Global());
}

bool Context::IdleNotificationDeadline(double deadline_in_seconds)
{
	// TODO remove
	_isolate->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);
	return _isolate->IdleNotificationDeadline(deadline_in_seconds);
}


} // namespace V8Simple

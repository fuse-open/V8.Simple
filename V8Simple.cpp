#include "V8Simple.h"
#include <include/v8-debug.h>
#include <include/libplatform/libplatform.h>
#include <stdlib.h>
#include <string>

namespace V8Simple
{

String::String(const byte* buffer, int bufferLength)
	: _value(new byte[bufferLength + 1])
	, _length(bufferLength)
{
	if (_length > 0 && buffer != nullptr)
	{
		std::memcpy(_value, buffer, _length);
	}
	_value[_length] = 0;
}

String* String::New(const byte* buffer, int bufferLength)
{
	return buffer == nullptr ? nullptr : new String(buffer, bufferLength);
}

String& String::operator=(const String& str)
{
	delete[] _value;
	_length = str._length;
	_value = new byte[_length + 1];
	std::memcpy(_value, str._value, _length + 1);
	return *this;
}

String::String(const String& str)
	: String(str._value, str._length)
{
}

String::String(const v8::String::Utf8Value& v)
	: String(v.length() > 0 ? reinterpret_cast<const byte*>(*v) : nullptr, v.length())
{
}

Type String::GetValueType() const { return Type::String; }
const byte* String::GetValue() const { return _value; }
int String::GetBufferLength() const { return _length; }
void String::GetBuffer(byte* outBuffer) const
{
	std::memcpy(outBuffer, _value, _length);
}

String::~String()
{
	delete[] _value;
}

String* String::Copy() const
{
	return new String(*this);
}

v8::Isolate* Context::Isolate()
{
	return Context::_isolate;
}

Context* Context::Global()
{
	return Context::_globalContext;
}

struct V8Scope
{
	V8Scope(v8::Isolate* isolate)
		: Locker(isolate)
		, IsolateScope(isolate)
		, HandleScope(isolate)
		, ContextScope(Context::Global()->V8Context())
	{
	}
	V8Scope()
		: V8Scope(Context::Isolate())
	{
	}
	v8::Locker Locker;
	v8::Isolate::Scope IsolateScope;
	v8::HandleScope HandleScope;
	v8::Context::Scope ContextScope;
};

void Throw(const v8::TryCatch& tryCatch)
{
	auto context = Context::Global()->V8Context();
	v8::Local<v8::Value> emptyString = v8::String::Empty(Context::Isolate());

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

template<class A>
static v8::Local<A> FromJust(
	const v8::TryCatch& tryCatch,
	v8::MaybeLocal<A> a)
{
	if (tryCatch.HasCaught() || a.IsEmpty())
	{
		Throw(tryCatch);
	}
	return a.ToLocalChecked();
}

template<class A>
static A FromJust(
	const v8::TryCatch& tryCatch,
	v8::Maybe<A> a)
{
	if (tryCatch.HasCaught() || a.IsNothing())
	{
		Throw(tryCatch);
	}
	return a.FromJust();
}

v8::Local<v8::String> Value::ToV8String(const v8::TryCatch& tryCatch, const String& str)
{
	return FromJust(tryCatch, v8::String::NewFromUtf8(Context::Isolate(), reinterpret_cast<const char*>(str.GetValue()), v8::NewStringType::kNormal));
}

v8::Local<v8::String> Value::ToV8String(const String& str)
{
	v8::TryCatch tryCatch;
	return ToV8String(tryCatch, str);
}

Value* Value::Wrap(const v8::TryCatch& tryCatch, v8::Local<v8::Value> value)
{
	auto context = Context::Global()->V8Context();
	if (value->IsInt32())
	{
		return new Int(FromJust(
			tryCatch,
			value->Int32Value(context)));
	}
	if (value->IsNumber() || value->IsNumberObject())
	{
		return new Double(FromJust(
			tryCatch,
			value->NumberValue(context)));
	}
	if (value->IsBoolean() || value->IsBooleanObject())
	{
		return new Bool(FromJust(
			tryCatch,
			value->BooleanValue(context)));
	}
	if (value->IsString() || value->IsStringObject())
	{
		v8::String::Utf8Value str(FromJust(
			tryCatch,
			value->ToString(context)));
		return new String(reinterpret_cast<const byte*>(*str), str.length());
	}
	if (value->IsArray())
	{
		return new Array(FromJust(
			tryCatch,
			value->ToObject(context)).As<v8::Array>());
	}
	if (value->IsFunction())
	{
		return new Function(FromJust(
			tryCatch,
			value->ToObject(context)).As<v8::Function>());
	}
	if (value->IsExternal())
	{
		return new External(value.As<v8::External>()->Value());
	}
	if (value->IsObject())
	{
		return new Object(FromJust(
			tryCatch,
			value->ToObject(context)));
	}
	if (value->IsUndefined() || value->IsNull())
	{
		return nullptr;
	}
	throw std::runtime_error("Unhandled type in V8Simple");
}

Value* Value::Wrap(const v8::TryCatch& tryCatch, v8::MaybeLocal<v8::Value> value)
{
	return Wrap(tryCatch, FromJust(tryCatch, value));
}

Value* Value::Wrap(v8::Local<v8::Value> value)
{
	v8::TryCatch tryCatch;
	return Wrap(tryCatch, value);

}

void Value::Delete()
{
	delete this;
}

v8::Local<v8::Value> Value::Unwrap(Value* value)
{
	auto isolate = Context::Isolate();

	if (value == nullptr)
	{
		return v8::Null(isolate).As<v8::Value>();
	}

	switch (value->GetValueType())
	{
		case Type::Int:
			return v8::Int32::New(
				isolate,
				static_cast<Int*>(value)->GetValue());
		case Type::Double:
			return v8::Number::New(
				isolate,
				static_cast<Double*>(value)->GetValue());
		case Type::String:
			return ToV8String(*static_cast<String*>(value));
		case Type::Bool:
			return v8::Boolean::New(
				isolate,
				static_cast<Bool*>(value)->GetValue());
		case Type::Object:
			return static_cast<Object*>(value)
				->_object->Get(isolate);
		case Type::Array:
			return static_cast<Array*>(value)
				->_array->Get(isolate);
		case Type::Function:
			return static_cast<Function*>(value)
				->_function->Get(isolate);
		case Type::External:
			return static_cast<External*>(value)
				->_external->Get(isolate);
		case Type::Callback:
		{
			Callback* callback = static_cast<Callback*>(value);
			callback->Retain();
			auto localCallback = v8::External::New(isolate, callback);
			auto persistentCallback = new v8::Persistent<v8::External, v8::CopyablePersistentTraits<v8::External> >(isolate, localCallback);
			struct Closure
			{
				v8::Persistent<v8::External, v8::CopyablePersistentTraits<v8::External> >* persistent;
				Callback* callback;
			};

			auto closure = new Closure{persistentCallback, callback};

			persistentCallback->SetWeak(
				closure,
				[] (const v8::WeakCallbackInfo<Closure>& data)
				{
					auto closure = data.GetParameter();
					auto pcb = closure->persistent;
					auto cb = closure->callback;
					cb->Release();
					pcb->ClearWeak();
					delete pcb;
					delete closure;
				},
				v8::WeakCallbackType::kParameter);

			v8::TryCatch tryCatch;
			return FromJust(tryCatch, v8::Function::New(
				Context::Global()->V8Context(),
				[] (const v8::FunctionCallbackInfo<v8::Value>& info)
				{
					v8::HandleScope handleScope(info.GetIsolate());
					try
					{
						std::vector<Value*> wrappedArgs;
						wrappedArgs.reserve(info.Length());
						for (int i = 0; i < info.Length(); ++i)
						{
							wrappedArgs.push_back(Wrap(info[i]));
						}

						Callback* callback =
							static_cast<Callback*>(info.Data()
								.As<v8::External>()
								->Value());
						UniqueValueVector args((const std::vector<Value*>&&)wrappedArgs);
						Value* result = callback->Call(&args);

						info.GetReturnValue().Set(Unwrap(result));
					}
					catch (const std::runtime_error& e)
					{
						info.GetIsolate()->ThrowException(
							v8::String::NewFromUtf8(
								info.GetIsolate(),
								e.what(),
								v8::NewStringType::kNormal)
							.FromMaybe(v8::String::Empty(info.GetIsolate())));
					}
					catch (const ScriptException& e)
					{
						info.GetIsolate()->ThrowException(
							v8::String::NewFromUtf8(
								info.GetIsolate(),
								reinterpret_cast<const char*>(e.ErrorMessage.GetValue()),
								v8::NewStringType::kNormal)
							.FromMaybe(v8::String::Empty(info.GetIsolate())));
					}
				},
				localCallback.As<v8::Value>()));
		}
	}
	return v8::Null(isolate).As<v8::Value>();
}

std::vector<v8::Local<v8::Value>> Value::UnwrapVector(const std::vector<Value*>& values)
{
	std::vector<v8::Local<v8::Value>> result;
	result.reserve(values.size());
	for (Value* value: values)
	{
		result.push_back(Unwrap(value));
	}
	return result;
}

Object::Object(v8::Local<v8::Object> object)
	: _object(new v8::Persistent<v8::Object, v8::CopyablePersistentTraits<v8::Object> >(Context::Isolate(), object))
{ }

Object::~Object()
{
	v8::Locker locker(Context::Isolate());
	v8::Isolate::Scope isolateScope(Context::Isolate());
	delete _object;
}

Type Object::GetValueType() const { return Type::Object; }

Value* Object::Get(const String* key)
{
	if (key == nullptr)
	{
		Context::Global()->HandleRuntimeException("V8Simple::Object::Get is not defined for nullptr argument");
		return nullptr;
	}
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;

		return Wrap(
			tryCatch,
			_object->Get(Context::Isolate())->Get(
				Context::Global()->V8Context(),
				ToV8String(tryCatch, *key)));
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
		return nullptr;
	}
	catch (const std::runtime_error& e)
	{
		Context::Global()->HandleRuntimeException(e.what());
		return nullptr;
	}
}

void Object::Set(const String* key, Value* value)
{
	if (key == nullptr)
	{
		Context::Global()->HandleRuntimeException("V8Simple::Object::Set is not defined for nullptr `key` argument");
		return;
	}
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;

		FromJust(tryCatch, _object->Get(Context::Isolate())->Set(
			Context::Global()->V8Context(),
			ToV8String(tryCatch, *key),
			Unwrap(value)));
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
	}
}

UniqueValueVector* Object::Keys()
{
	std::vector<Value*> result;
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;
		auto context = Context::Global()->V8Context();

		auto propArr = FromJust(
			tryCatch,
			_object->Get(Context::Isolate())->GetOwnPropertyNames(context));

		auto length = propArr->Length();
		result.reserve(length);
		for (int i = 0; i < static_cast<int>(length); ++i)
		{
			result.push_back(new String(v8::String::Utf8Value(FromJust(
				tryCatch,
				propArr->Get(context, static_cast<uint32_t>(i))))));
		}
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
	}
	return new UniqueValueVector((const std::vector<Value*>&&)result);
}

bool Object::InstanceOf(Function* type)
{
	if (type == nullptr)
	{
		return false;
	}
	Value* callResult = nullptr;
	try
	{
		std::vector<Value*> args;
		args.reserve(2);
		args.push_back(this);
		args.push_back(type);
		callResult = Context::Global()->_instanceOf->Call(args);
		bool result = (callResult == nullptr || callResult->GetValueType() != Type::Bool)
			? false
			: static_cast<Bool*>(callResult)->GetValue();
		delete callResult;
		return result;
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
	}
	catch (const std::runtime_error& e)
	{
		Context::Global()->HandleRuntimeException(e.what());
	}
	delete callResult;
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
	const String* name,
	const std::vector<Value*>& args)
{
	if (name == nullptr)
	{
		Context::Global()->HandleRuntimeException("V8Simple::Object::CallMethod is not defined for nullptr `name` argument");
		return nullptr;
	}
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;

		auto localObject = _object->Get(Context::Isolate());
		auto prop = FromJust(
			tryCatch,
			localObject->Get(
				Context::Global()->V8Context(),
				ToV8String(tryCatch, *name).As<v8::Value>()));

		if (!prop->IsFunction())
		{
			Throw(tryCatch);
		}
		auto fun = prop.As<v8::Function>();

		auto unwrappedArgs = UnwrapVector(args);
		return Wrap(
			tryCatch,
			fun->Call(
				localObject,
				static_cast<int>(unwrappedArgs.size()),
				DataPointer(unwrappedArgs)));
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
	}
	catch (const std::runtime_error& e)
	{
		Context::Global()->HandleRuntimeException(e.what());
	}
	return nullptr;
}

Value* Object::CallMethod(
	const String* name,
	Value** args,
	int numArgs)
{
	return CallMethod(name, std::vector<Value*>(args, args + numArgs));
}

bool Object::ContainsKey(const String* key)
{
	if (key == nullptr)
	{
		Context::Global()->HandleRuntimeException("V8Simple::Object::ContainsKey is not defined for nullptr");
		return false;
	}
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;

		return FromJust(
			tryCatch,
			_object->Get(Context::Isolate())->Has(
				Context::Global()->V8Context(),
				ToV8String(tryCatch, *key)));
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
		return false;
	}
}

void* Object::GetArrayBufferData()
{
	V8Scope scope;

	auto localObject = _object->Get(Context::Isolate());
	if (!localObject->IsArrayBuffer())
	{
		return nullptr;
	}
	auto arrayBuffer = localObject.As<v8::ArrayBuffer>();
	return arrayBuffer->GetContents().Data();
}

bool Object::StrictEquals(const Object* object)
{
	if (object == nullptr)
	{
		return false;
	}
	V8Scope scope;
	auto isolate = Context::Isolate();

	return _object->Get(isolate)->StrictEquals(
		object->_object->Get(isolate));
}

Function::Function(v8::Local<v8::Function> function)
	: _function(new v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>>(Context::Isolate(), function))
{ }

Function::~Function()
{
	v8::Locker locker(Context::Isolate());
	v8::Isolate::Scope isolateScope(Context::Isolate());
	delete _function;
}

Type Function::GetValueType() const { return Type::Function; }

Value* Function::Call(const std::vector<Value*>& args)
{
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;
		auto context = Context::Global()->V8Context();

		auto unwrappedArgs = UnwrapVector(args);
		return Wrap(
			tryCatch,
			_function->Get(Context::Isolate())->Call(
				context,
				context->Global(),
				static_cast<int>(unwrappedArgs.size()),
				DataPointer(unwrappedArgs)));
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
	}
	catch (const std::runtime_error& e)
	{
		Context::Global()->HandleRuntimeException(e.what());
	}
	return nullptr;
}

Value* Function::Call(
	Value** args,
	int numArgs)
{
	return Call(std::vector<Value*>(args, args + numArgs));
}

Object* Function::Construct(const std::vector<Value*>& args)
{
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;

		auto unwrappedArgs = UnwrapVector(args);

		return new Object(
			FromJust(
				tryCatch,
				_function->Get(Context::Isolate())->NewInstance(
					Context::Global()->V8Context(),
					static_cast<int>(unwrappedArgs.size()),
					DataPointer(unwrappedArgs))));
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
	}
	return nullptr;
}

Object* Function::Construct(
	Value** args,
	int numArgs)
{
	return Construct(std::vector<Value*>(args, args + numArgs));
}

bool Function::StrictEquals(const Function* function)
{
	if (function == nullptr)
	{
		return false;
	}
	V8Scope scope;
	auto isolate = Context::Isolate();

	return _function->Get(isolate)->StrictEquals(
		function->_function->Get(isolate));
}

Array::Array(v8::Local<v8::Array> array)
	: _array(new v8::Persistent<v8::Array, v8::CopyablePersistentTraits<v8::Array> >(Context::Isolate(), array))
{ }

Array::~Array()
{
	v8::Locker locker(Context::Isolate());
	v8::Isolate::Scope isolateScope(Context::Isolate());
	delete _array;
}

Type Array::GetValueType() const { return Type::Array; }

Value* Array::Get(int index)
{
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;

		return Wrap(
			tryCatch,
			_array->Get(Context::Isolate())->Get(
				Context::Global()->V8Context(),
				static_cast<uint32_t>(index)));
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
		return nullptr;
	}
	catch (const std::runtime_error& e)
	{
		Context::Global()->HandleRuntimeException(e.what());
		return nullptr;
	}
}

void Array::Set(int index, Value* value)
{
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;

		FromJust(
			tryCatch,
			_array->Get(Context::Isolate())->Set(
				Context::Global()->V8Context(),
				static_cast<uint32_t>(index),
				Unwrap(value)));
	}
	catch (const ScriptException& e)
	{
		Context::Global()->HandleScriptException(e);
	}
}

int Array::Length()
{
	V8Scope scope;

	return static_cast<int>(_array->Get(Context::Isolate())->Length());
}

bool Array::StrictEquals(const Array* array)
{
	if (array == nullptr)
	{
		return false;
	}
	V8Scope scope;
	auto isolate = Context::Isolate();

	return _array->Get(isolate)->StrictEquals(
		array->_array->Get(isolate));

}

UniqueValueVector::UniqueValueVector(const std::vector<Value*>&& values)
	: _values(new std::vector<Value*>(values))
{
}

int UniqueValueVector::Length()
{
	return static_cast<int>(_values->size());
}

Value* UniqueValueVector::Get(int index)
{
	Value* result = nullptr;
	std::swap(_values->at(index), result);
	return result;
}

UniqueValueVector::~UniqueValueVector()
{
	for (Value* v: *_values)
	{
		delete v;
	}
	delete _values;
}

void UniqueValueVector::Delete()
{
	delete this;
}

Callback::Callback()
{
}

Value* Callback::Call(UniqueValueVector* args)
{
	return nullptr;
}

Type Callback::GetValueType() const { return Type::Callback; }

External::External(void* value)
{
	V8Scope scope;

	auto isolate = Context::Isolate();
	auto localExternal = v8::External::New(isolate, value);
	_external = new v8::Persistent<v8::External, v8::CopyablePersistentTraits<v8::External> >(Context::Isolate(), localExternal);

	auto finalizer
		= new v8::Persistent<v8::External, v8::CopyablePersistentTraits<v8::External> >(isolate, localExternal);

	struct Closure
	{
		v8::Persistent<v8::External, v8::CopyablePersistentTraits<v8::External> >* persistent;
		void* value;
	};

	auto closure = new Closure{finalizer, value};

	finalizer->SetWeak(
		closure,
		[] (const v8::WeakCallbackInfo<Closure>& data)
		{
			auto closure = data.GetParameter();
			auto pext = closure->persistent;
			auto value = closure->value;
			if (Context::Global()->_externalFreer != nullptr)
			{
				Context::Global()->_externalFreer->Free(value);
			}
			pext->ClearWeak();
			delete pext;
			delete closure;
		},
		v8::WeakCallbackType::kParameter);
}

External* External::New(void* value)
{
	return new External(value);
}

External::External(v8::Local<v8::External> external)
	: _external(new v8::Persistent<v8::External, v8::CopyablePersistentTraits<v8::External> >(Context::Isolate(), external))
{ }

Type External::GetValueType() const { return Type::External; }

void* External::GetValue()
{
	V8Scope scope;
	auto isolate = Context::Isolate();
	return _external->Get(isolate)->Value();
}

External::~External()
{
	v8::Locker locker(Context::Isolate());
	v8::Isolate::Scope isolateScope(Context::Isolate());
	delete _external;
}

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

ScriptException* ScriptException::Copy() const
{
	return new ScriptException(*this);
}

void ScriptException::Delete()
{
	delete this;
}

void Context::HandleScriptException(const ScriptException& e) const
{
	if (_scriptExceptionHandler != nullptr)
	{
		_scriptExceptionHandler->Handle(e);
	}
}

void Context::HandleRuntimeException(const char* e) const
{
	if (_runtimeExceptionHandler != nullptr)
	{
		String str(reinterpret_cast<const byte*>(e), static_cast<int>(std::strlen(e)));
		_runtimeExceptionHandler->Handle(&str);
	}
}

void Context::SetDebugMessageHandler(MessageHandler* debugMessageHandler)
{
	if (Context::Global() != nullptr)
	{
		V8Scope scope;

		_debugMessageHandler = debugMessageHandler;

		if (debugMessageHandler == nullptr)
		{
			v8::Debug::SetMessageHandler(nullptr);
		}
		else
		{
			v8::Debug::SetMessageHandler([] (const v8::Debug::Message& message)
			{
				v8::String::Utf8Value utf8(message.GetJSON());
				const String str(reinterpret_cast<const byte*>(*utf8), utf8.length());
				_debugMessageHandler->Handle(&str);
			});
		}
	}
}

void Context::SendDebugCommand(const String* command)
{
	if (Context::Global() != nullptr)
	{
		if (command == nullptr)
		{
			Context::Global()->HandleRuntimeException("V8Simple::Context::SendDebugCommand is not defined for nullptr argument");
			return;
		}
		v8::Locker locker(_conversionIsolate);
		v8::Isolate::Scope isolateScope(_conversionIsolate);
		v8::HandleScope handleScope(_conversionIsolate);

		auto str = v8::String::NewFromUtf8(
			_conversionIsolate,
			reinterpret_cast<const char*>(command->GetValue()),
			v8::NewStringType::kNormal).FromMaybe(v8::String::Empty(_conversionIsolate));
		auto len = str->Length();
		auto buffer = new uint16_t[len + 1];
		str->Write(buffer);
		v8::Debug::SendCommand(_isolate, buffer, len);
		delete[] buffer;
	}
}

void Context::ProcessDebugMessages()
{
	if (Context::Global() != nullptr)
	{
		V8Scope scope;
		v8::Debug::ProcessDebugMessages();
	}
}

v8::Platform* Context::_platform = nullptr;
v8::Isolate* Context::_isolate = nullptr;
v8::Isolate* Context::_conversionIsolate = nullptr;
MessageHandler* Context::_debugMessageHandler = nullptr;
Context* Context::_globalContext = nullptr;

Context::Context(ScriptExceptionHandler* scriptExceptionHandler, MessageHandler* runtimeExceptionHandler, ExternalFreer* externalFreer)
	: _scriptExceptionHandler(scriptExceptionHandler)
	, _runtimeExceptionHandler(runtimeExceptionHandler)
	, _externalFreer(externalFreer)
{

	if (_globalContext != nullptr)
	{
		HandleRuntimeException("V8Simple::Context is not re-entrant");
		return;
	}

	_globalContext = this;

	// v8::V8::SetFlagsFromString("--expose-gc", 11);
	{
		// For compatibility with node.js
		const char flags[] = "--expose_debug_as=v8debug";
		v8::V8::SetFlagsFromString(flags, sizeof(flags));
	}

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
		_conversionIsolate = v8::Isolate::New(createParams);
	}

	{
		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);

		auto localContext = v8::Context::New(_isolate);
		v8::Context::Scope contextScope(localContext);

		_context = new v8::Persistent<v8::Context, v8::CopyablePersistentTraits<v8::Context> >(_isolate, localContext);
	}
	
	{
		const char instanceOfStrBuf[] = "instanceof";
		const char instanceOfCodeBuf[] = "(function(x, y) { return (x instanceof y); })";
		const String instanceOfString(reinterpret_cast<const byte*>(instanceOfStrBuf), sizeof (instanceOfStrBuf));
		const String instanceOfCode(reinterpret_cast<const byte*>(instanceOfCodeBuf), sizeof (instanceOfCodeBuf));
		Value* instanceOf = Evaluate(&instanceOfString, &instanceOfCode);
		
		if (!instanceOf || instanceOf->GetValueType() != Type::Function)
		{
			Context::HandleRuntimeException("V8Simple could not create an instanceof function");
		}
		else
		{
			_instanceOf = static_cast<Function*>(instanceOf);
		}
	}
}

Context* Context::New(
	ScriptExceptionHandler* scriptExceptionHandler,
	MessageHandler* runtimeExceptionHandler,
	ExternalFreer* externalFreer)
{
	return new Context(scriptExceptionHandler, runtimeExceptionHandler, externalFreer);
}

Context::~Context()
{
	SetDebugMessageHandler(nullptr);
	_globalContext = nullptr;

	{
		v8::Locker locker(_isolate);
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);

		{
			v8::Context::Scope contextScope(_context->Get(_isolate));
			delete _instanceOf;
		}

		delete _context;
	}

	// _conversionIsolate->Dispose();
	// _isolate->Dispose();

	// If we do this we can't create a new context afterwards.
	//
	// v8::V8::Dispose();
	// v8::V8::ShutdownPlatform();
	// delete _platform;
}

Value* Context::Evaluate(const String* fileName, const String* code)
{
	if (fileName == nullptr)
	{
		Context::HandleRuntimeException("V8Simple::Context::Evaluate is not defined for nullptr `fileName` argument");
		return nullptr;
	}
	if (code == nullptr)
	{
		Context::HandleRuntimeException("V8Simple::Context::Evaluate is not defined for nullptr `code` argument");
		return nullptr;
	}
	try
	{
		V8Scope scope;
		v8::TryCatch tryCatch;
		auto context = V8Context();

		v8::ScriptOrigin origin(Value::ToV8String(tryCatch, *fileName));
		auto script = FromJust(
			tryCatch,
			v8::Script::Compile(
				context,
				Value::ToV8String(tryCatch, *code),
				&origin));

		return Value::Wrap(tryCatch, script->Run(context));
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
	V8Scope scope;

	return new Object(V8Context()->Global());
}

Object* Context::NewExternalArrayBuffer(void* data, int byteLength)
{
	V8Scope scope;

	return new Object(v8::ArrayBuffer::New(Context::Isolate(), data, (size_t)byteLength));
}

bool Context::IdleNotificationDeadline(double deadline_in_seconds)
{
	// _isolate->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);
	return _isolate->IdleNotificationDeadline(deadline_in_seconds);
}

Value* Context::ThrowException(Value* exception)
{
	V8Scope scope;

	v8::Local<v8::Value> unwrappedException = Value::Unwrap(exception);

	auto result = Isolate()->ThrowException(unwrappedException);

	return Value::Wrap(result);
}

void Context::Delete()
{
	delete this;
}

v8::Local<v8::Context> Context::V8Context() const
{
	return _context->Get(Isolate());
}

const char* Context::GetVersion()
{
	return v8::V8::GetVersion();
}

} // namespace V8Simple

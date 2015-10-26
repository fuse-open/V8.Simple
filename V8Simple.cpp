#include "V8Simple.h"
#include <include/libplatform/libplatform.h>
#include <stdlib.h>

namespace V8Simple
{

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

static v8::Local<v8::String> ToV8String(v8::Isolate* isolate, const std::string& str)
{
	return v8::String::NewFromUtf8(isolate, str.c_str());
}

template<class T>
static std::string ToString(const T& t)
{
	return *v8::String::Utf8Value(t);
}

static void Throw(v8::Local<v8::Context> context, const v8::TryCatch& tryCatch) throw(ScriptException)
{
	std::string exception(ToString(tryCatch.Exception()));
	auto message = tryCatch.Message();
	auto stackTrace = ToString(
		tryCatch
		.StackTrace(context)
		.FromMaybe(ToV8String(context->GetIsolate(), "").As<v8::Value>()));
	throw ScriptException(
		exception,
		ToString(message->Get()),
		ToString(message->GetScriptOrigin().ResourceName()),
		message->GetLineNumber(context).FromMaybe(-1),
		stackTrace);
}

template<class A>
static v8::Local<A> FromJust(
	v8::Local<v8::Context> context,
	const v8::TryCatch& tryCatch,
	v8::MaybeLocal<A> a) throw(ScriptException)
{
	if (a.IsEmpty())
	{
		Throw(context, tryCatch);
	}
	return a.ToLocalChecked();
}

template<class A>
static A FromJust(
	v8::Local<v8::Context> context,
	const v8::TryCatch& tryCatch,
	v8::Maybe<A> a) throw(ScriptException)
{
	if (a.IsNothing())
	{
		Throw(context, tryCatch);
	}
	return a.FromJust();
}

Value* Context::Wrap(
	const v8::TryCatch& tryCatch,
	v8::Local<v8::Value> value) throw(ScriptException, Exception)
{
	auto context = _isolate->GetCurrentContext();
	if (value->IsInt32())
	{
		return new Int(FromJust(context, tryCatch, value->Int32Value(context)));
	}
	if (value->IsNumber() || value->IsNumberObject())
	{
		return new Double(FromJust(context, tryCatch, value->NumberValue(context)));
	}
	if (value->IsBoolean() || value->IsBooleanObject())
	{
		return new Bool(FromJust(context, tryCatch, value->BooleanValue(context)));
	}
	if (value->IsString() || value->IsStringObject())
	{
		return new String(ToString(FromJust(context, tryCatch, value->ToString(context))));
	}
	if (value->IsArray())
	{
		return new Array(FromJust(context, tryCatch, value->ToObject(context)).As<v8::Array>());
	}
	if (value->IsFunction())
	{
		return new Function(FromJust(context, tryCatch, value->ToObject(context)).As<v8::Function>());
	}
	if (value->IsObject())
	{
		return new Object(FromJust(context, tryCatch, value->ToObject(context)));
	}
	if (value->IsUndefined() || value->IsNull())
	{
		return nullptr;
	}
	throw Exception("Unhandled type in V8Simple");
}

Value* Context::Wrap(
	const v8::TryCatch& tryCatch,
	v8::MaybeLocal<v8::Value> mvalue) throw(ScriptException, Exception)
{
	auto context = _isolate->GetCurrentContext();
	return Wrap(FromJust(context, tryCatch, mvalue), tryCatch);
}

v8::Local<v8::Value> Context::Unwrap(
	const v8::TryCatch& tryCatch,
	const Value* value) throw(ScriptException)
{
	if (value == nullptr)
	{
		return v8::Null(_isolate).As<v8::Value>();
	}

	switch (value->GetType())
	{
		case Type::Int:
			return v8::Int32::New(_isolate, static_cast<const Int*>(value)->GetValue());
		case Type::Double:
			return v8::Number::New(_isolate, static_cast<const Double*>(value)->GetValue());
		case Type::String:
			return ToV8String(_isolate, static_cast<const String*>(value)->GetValue());
		case Type::Bool:
			return v8::Boolean::New(_isolate, static_cast<const Bool*>(value)->GetValue());
		case Type::Object:
			return static_cast<const Object*>(value)->_object.Get(_isolate);
		case Type::Array:
			return static_cast<const Array*>(value)->_array.Get(_isolate);
		case Type::Function:
			return static_cast<const Function*>(value)->_function.Get(_isolate);
		case Type::Callback:
			Callback* callback = static_cast<const Callback*>(value)->Copy();
			auto localCallback = v8::External::New(_isolate, callback);
			v8::Persistent<v8::External> persistentCallback(_isolate, localCallbcak);

			persistentCallback.SetWeak(
				callback,
				[] (const v8::WeakCallbackInfo<Callback>& data)
				{
					delete callback;
				},
				v8::WeakCallbackType::kParameter);

			auto context = _isolate->GetCurrentContext();
			auto function = FromJust(context, v8::Function::New(
				context,
				[] (const v8::FunctionCallbackInfo<v8::Value>& info)
				{
					v8::HandleScope handleScope(info.GetIsolate());
					v8::TryCatch tryCatch(info.GetIsolate());

					std::vector<Value*> wrappedArgs(info.Length());
					for (int i = 0; i < info.Length(); ++i)
					{
						wrappedArgs.push_back(Wrap(info[i], tryCatch));
					}

					Callback* callback = static_cast<Callback*>(info.Data().As<v8::External>()->Value());
					Value* result = callback->Call(wrappedArgs);

					for (Value* value: wrappedArgs)
					{
						delete value;
					}
					info.GetReturnValue().Set(Unwrap(result, tryCatch));
				},
				localCallback.As<v8::Value>()), tryCatch);
	}
}

std::vector<v8::Local<v8::Value>> Context::UnwrapVector(
	const std::vector<Value*>& values,
	const v8::TryCatch& tryCatch) throw(ScriptException)
{
	std::vector<v8::Local<v8::Value>> result(values.size());
	for (Value* value: values)
	{
		result.push_back(Unwrap(*value, tryCatch));
	}
	return result;
}

Context::Context()
{
	v8::V8::InitializeICU();
	_platform = v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(_platform);
	v8::V8::Initialize();

	v8::Isolate::CreateParams createParams;
	static ArrayBufferAllocator arrayBufferAllocator;
	createParams.array_buffer_allocator = &arrayBufferAllocator;
	_isolate = v8::Isolate::New(createParams);

	v8::Isolate::Scope isolateScope(_isolate);
	v8::HandleScope handleScope(_isolate);

	auto localContext = v8::Context::New(_isolate);
	localContext->Enter();

	_context = new v8::Persistent<v8::Context>(_isolate, localContext);
	_instanceOf = static_cast<Function*>(
		Evaluate("instanceof", "(function(x, y) { return (x instanceof y); })"));
}

Context::~Context()
{
	delete _instanceOf;
	_instanceOf = nullptr;

	{
		v8::Isolate::Scope isolateScope(_isolate);
		v8::HandleScope handleScope(_isolate);
		_context->Get(_isolate)->Exit();
	}
	delete _context;
	_context = nullptr;

	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	delete _platform;
}

Value* Context::Evaluate(const std::string& fileName, const std::string& code)
{
	v8::Isolate::Scope isolateScope(_isolate);
	v8::HandleScope handleScope(_isolate);
	v8::TryCatch tryCatch(_isolate);

	auto script = v8::Script::Compile(ToV8String(_isolate, code), ToV8String(_isolate, fileName));
	return Wrap(script->Run(), tryCatch);
}

Object* Context::GlobalObject()
{
	v8::Isolate::Scope isolateScope(_isolate);
	v8::HandleScope handleScope(_isolate);

	return new Object(_context->Get(_isolate)->Global());
}

Object::Object(v8::Local<v8::Object> object)
	: _object(Context::_isolate, object)
{ }

Type Object::GetType() { return Type::Object; }

Value* Object::Get(const std::string& key)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	return Context::Wrap(
		_object.Get(Context::_isolate)->Get(
			Context::_isolate->GetCurrentContext(),
			ToV8String(Context::_isolate, key)),
		tryCatch);
}

void Object::Set(const std::string& key, const Value& value)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);
	auto context = Context::_isolate->GetCurrentContext();

	auto ret = _object.Get(Context::_isolate)->Set(
		context,
		ToV8String(Context::_isolate, key),
		Unwrap(value, tryCatch));
	FromJust(context, ret, tryCatch);
}

bool Object::InstanceOf(Function& type)
{
	Value* thisValue = static_cast<Value*>(this);
	std::vector<Value*> args{thisValue, static_cast<Value*>(&type)};
	Bool* callResult = static_cast<Bool*>(Context::_instanceOf->Call(args));
	bool result = callResult->GetValue();
	delete callResult;
	return result;
}

Value* Object::CallMethod(const std::string& name, const std::vector<Value*>& args)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	auto localObject = _object.Get(Context::_isolate);
	auto fun = FromJust(
		context,
		localObject->Get(context, ToV8String(Context::_isolate, name).As<v8::Value>()),
		tryCatch).As<v8::Function>();
	auto unwrappedArgs = UnwrapVector(args, tryCatch);
	return Context::Wrap(
		fun->Call(
			localObject,
			static_cast<int>(unwrappedArgs.size()),
			unwrappedArgs.data()),
		tryCatch);
}

bool Object::ContainsKey(const std::string& key)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	return FromJust(
		context,
		_object.Get(Context::_isolate)->Has(
			context,
			ToV8String(Context::_isolate, key)),
		tryCatch);
}

bool Object::Equals(const Object& o)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	return FromJust(
		context,
		_object.Get(Context::_isolate)->Equals(
			context,
			o._object.Get(Context::_isolate)),
		tryCatch);
}

Function::Function(v8::Local<v8::Function> function)
	: _function(Context::_isolate, function)
{ }

Type Function::GetType() { return Type::Function; }

Value* Function::Call(const std::vector<Value*>& args)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	auto unwrappedArgs = UnwrapVector(args, tryCatch);
	return Context::Wrap(
		_function.Get(Context::_isolate)->Call(
			context,
			context->Global(),
			static_cast<int>(unwrappedArgs.size()),
			unwrappedArgs.data()),
		tryCatch);
}

Object* Function::Construct(const std::vector<Value*>& args)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	auto unwrappedArgs = UnwrapVector(args, tryCatch);
	return new Object(
		FromJust(
			context,
			_function.Get(Context::_isolate)->NewInstance(
				context,
				unwrappedArgs.size(),
				unwrappedArgs.data()),
			tryCatch));
}

bool Function::Equals(const Function& function)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	return FromJust(
		context,
		_function.Get(Context::_isolate)->Equals(
			context,
			function._function.Get(Context::_isolate)),
		tryCatch);
}

Array::Array(v8::Local<v8::Array> array)
	: _array(Context::_isolate, array)
{ }

Type Array::GetType() { return Type::Array; }

Value* Array::Get(int index)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	return Context::Wrap(
		_array.Get(Context::_isolate)->Get(
			context,
			static_cast<uint32_t>(index)),
		tryCatch);
}

void Array::Set(int index, const Value& value)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	auto result = _array.Get(Context::_isolate)->Set(
		context,
		static_cast<uint32_t>(index),
		Unwrap(value, tryCatch));
	FromJust(context, result, tryCatch);
}

int Array::Length()
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	return static_cast<int>(_array.Get(Context::_isolate)->Length());
}

bool Array::Equals(const Array& array)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	return FromJust(
		context,
		_array.Get(Context::_isolate)->Equals(
			context,
			array._array.Get(Context::_isolate)),
		tryCatch);

}

Type Callback::GetType() { return Type::Callback; }

} // namespace V8Simple

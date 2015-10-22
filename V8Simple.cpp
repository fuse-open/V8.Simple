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

static ArrayBufferAllocator arrayBufferAllocator{};

template<class A>
static v8::Local<A> FromJust(v8::MaybeLocal<A> a, v8::TryCatch& tryCatch)
{
	// TODO
	return a.ToLocalChecked();
}

template<class A>
static A FromJust(v8::Maybe<A> a, v8::TryCatch& tryCatch)
{
	// TODO
	return a.FromJust();
}

static Value* Wrap(v8::Local<v8::Value> value, const v8::TryCatch& tryCatch)
{
	// TODO
}

static Value* Wrap(v8::MaybeLocal<v8::Value> mvalue, const v8::TryCatch& tryCatch)
{
	// TODO
}

static v8::Local<v8::Value> Unwrap(const Value& value, const v8::TryCatch& tryCatch)
{
	// TODO
}

static std::vector<v8::Local<v8::Value>> UnwrapVector(const std::vector<Value*>& values, const v8::TryCatch& tryCatch)
{
	// TODO
}

Context::Context()
{
	v8::V8::InitializeICU();
	_platform = v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(_platform);
	v8::V8::Initialize();

	v8::Isolate::CreateParams createParams;
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

static v8::Local<v8::String> V8String(v8::Isolate* isolate, const std::string& str)
{
	return v8::String::NewFromUtf8(isolate, str.c_str());
}

Value* Context::Evaluate(const std::string& fileName, const std::string& code)
{
	v8::Isolate::Scope isolateScope(_isolate);
	v8::HandleScope handleScope(_isolate);
	v8::TryCatch tryCatch(_isolate);

	auto script = v8::Script::Compile(V8String(_isolate, code), V8String(_isolate, fileName));
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

Value* Object::Get(const std::string& key)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	return Wrap(
		_object.Get(Context::_isolate)->Get(
			Context::_isolate->GetCurrentContext(),
			V8String(Context::_isolate, key)),
		tryCatch);
}

void Object::Set(const std::string& key, const Value& value)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto ret = _object.Get(Context::_isolate)->Set(
		Context::_isolate->GetCurrentContext(),
		V8String(Context::_isolate, key),
		Unwrap(value, tryCatch));
	FromJust(ret, tryCatch);
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
		localObject->Get(context, V8String(Context::_isolate, name).As<v8::Value>()),
		tryCatch).As<v8::Function>();
	auto unwrappedArgs = UnwrapVector(args, tryCatch);
	return Wrap(
		fun->Call(
			localObject,
			unwrappedArgs.size(),
			unwrappedArgs.data()),
		tryCatch);
}

bool Object::ContainsKey(const std::string& key)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	return FromJust(
		_object.Get(Context::_isolate)->Has(
			Context::_isolate->GetCurrentContext(),
			V8String(Context::_isolate, key)),
		tryCatch);
}

bool Object::Equals(const Object& o)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	return FromJust(
		_object.Get(Context::_isolate)->Equals(
			Context::_isolate->GetCurrentContext(),
			o._object.Get(Context::_isolate)),
		tryCatch);
}

Function::Function(v8::Local<v8::Function> function)
	: _function(Context::_isolate, function)
{ }

Value* Function::Call(const std::vector<Value*>& args)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto context = Context::_isolate->GetCurrentContext();
	auto unwrappedArgs = UnwrapVector(args, tryCatch);
	return Wrap(
		_function.Get(Context::_isolate)->Call(
			context,
			context->Global(),
			unwrappedArgs.size(),
			unwrappedArgs.data()),
		tryCatch);
}

Object* Function::Construct(const std::vector<Value*>& args)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto unwrappedArgs = UnwrapVector(args, tryCatch);
	return new Object(
		FromJust(
			_function.Get(Context::_isolate)->NewInstance(
				Context::_isolate->GetCurrentContext(),
				unwrappedArgs.size(),
				unwrappedArgs.data()),
			tryCatch));
}

bool Function::Equals(const Function& function)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	return FromJust(
		_function.Get(Context::_isolate)->Equals(
			Context::_isolate->GetCurrentContext(),
			function._function.Get(Context::_isolate)),
		tryCatch);
}

Array::Array(v8::Local<v8::Array> array)
	: _array(Context::_isolate, array)
{ }

Value* Array::Get(int index)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	return Wrap(
		_array.Get(Context::_isolate)->Get(
			Context::_isolate->GetCurrentContext(),
			index),
		tryCatch);
}

void Array::Set(int index, const Value& value)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	auto result = _array.Get(Context::_isolate)->Set(
		Context::_isolate->GetCurrentContext(),
		index,
		Unwrap(value, tryCatch));
	FromJust(result, tryCatch);
}

int Array::Length()
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	return _array.Get(Context::_isolate)->Length();
}

bool Array::Equals(const Array& array)
{
	v8::Isolate::Scope isolateScope(Context::_isolate);
	v8::HandleScope handleScope(Context::_isolate);
	v8::TryCatch tryCatch(Context::_isolate);

	return FromJust(
		_array.Get(Context::_isolate)->Equals(
			Context::_isolate->GetCurrentContext(),
			array._array.Get(Context::_isolate)),
		tryCatch);

}

} // namespace V8Simple

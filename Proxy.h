#include <V8Simple.h>
#include <Uno.h>

class DebugMessageHandlerProxy: public DebugMessageHandler
{
public:
	DebugMessageHandlerProxy(@{DebugMessageHandler} unoObject)
		: _unoObject(unoObject)
	{
	}

	virtual void Handle(const char* jsonMessage) override
	{
		@{DebugMessageHandler:Of(_unoObject).Handle(string):Call(::uString::Utf8(jsonMessage))};
	}

	virtual void Retain() override
	{
		::uRetain(_unoObject);
		@{DebugMessageHandler:Of(_unoObject).Retain():Call()};
	}

	virtual void Release() override
	{
		@{DebugMessageHandler:Of(_unoObject).Release():Call()};
		::uRelease(_unoObject);
	}
private:
	@{DebugMessageHandler} _unoObject;
};

class ScriptExceptionHandlerProxy: public ScriptExceptionHandler
{
public:
	ScriptExceptionHandlerProxy(@{ScriptExceptionHandler} unoObject)
		: _unoObject(unoObject)
	{
	}

	virtual void Handle(const char* jsonMessage) override
	{
		@{ScriptExceptionHandler:Of(_unoObject).Handle(string):Call(::uString::Utf8(jsonMessage))};
	}

	virtual void Retain() override
	{
		::uRetain(_unoObject);
		@{ScriptExceptionHandler:Of(_unoObject).Retain():Call()};
	}

	virtual void Release() override
	{
		@{ScriptExceptionHandler:Of(_unoObject).Release():Call()};
		::uRelease(_unoObject);
	}
private:
	@{ScriptExceptionHandler} _unoObject;
};

class CallbackProxy: public Callback
{
public:
	CallbackProxy(@{Callback} unoObject)
		: _unoObject(unoObject)
	{
	}

	virtual Value* Call(const ::std::vector< ::Value*> args) override
	{
		@{ValueVector} vargs = @{ValueVector():New()};
		for (auto val: args)
		{
			@{ValueVector:Of(args):Add(Value):Call(@{Value(ValueImpl).New(val)})};
		}
		@{Value} result = @{Callback:Of(_unoObject).Call(ValueVector):Call(vargs)};
		return @{Value:Of(result)._valueImpl:Get()};
	}

	virtual void Retain() override
	{
		::uRetain(_unoObject);
		@{Callback:Of(_unoObject).Retain():Call()};
	}

	virtual void Release() override
	{
		@{Callback:Of(_unoObject).Release():Call()};
		::uRelease(_unoObject);
	}
private:
	@{Callback} _unoObject;
};

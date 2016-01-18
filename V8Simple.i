%module(directors="1") v8
%{
#include "V8Simple.h"
%}

%typemap(csdestruct, methodname="Dispose", methodmodifiers="public") SWIGTYPE {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          $imcall;
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
      global::System.GC.SuppressFinalize(this);
  }

%typemap(csdestruct_derived, methodname="Dispose", methodmodifiers="public") SWIGTYPE {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          $imcall;
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
      global::System.GC.SuppressFinalize(this);
      base.Dispose();
  }


// Make sure we return the correct objects in C#:
// See: http://johnnado.com/swig-csharp-java-downcast/
%pragma(csharp) imclasscode=%{
	public static Value InstantiateConcreteValue(global::System.IntPtr cPtr, bool owner)
	{
		if (cPtr == global::System.IntPtr.Zero)
		{
			return null;
		}
		Type type = (Type)$modulePINVOKE.Value_GetValueType(new global::System.Runtime.InteropServices.HandleRef(null, cPtr));
		switch (type)
		{
			case Type.Int: return new Int(cPtr, owner);
			case Type.Double: return new Double(cPtr, owner);
			case Type.String: return new String(cPtr, owner);
			case Type.Bool: return new Bool(cPtr, owner);
			case Type.Object: return new Object(cPtr, owner);
			case Type.Function: return new Function(cPtr, owner);
			case Type.Array: return new Array(cPtr, owner);
		}
		throw new global::System.Exception("V8Simple: Unhandled value type");
	}
%}
%typemap(csout, excode=SWIGEXCODE)
V8Simple::Value*
{
	global::System.IntPtr cPtr = $imcall;
	$csclassname ret = ($csclassname) $modulePINVOKE.InstantiateConcreteValue(cPtr, $owner);$excode
	return ret;
}
%include <std_vector.i>
%newobject V8Simple::Context::Evaluate(const char*, const char*);
%newobject V8Simple::Context::GlobalObject();
%newobject V8Simple::Object::Get(const char*);
%newobject V8Simple::Object::Keys();
%newobject V8Simple::Object::CallMethod(const char*, const std::vector<Value*>&);
%ignore V8Simple::Object::CallMethod(const char*, Value** args, int numArgs);
%newobject V8Simple::Function::Call(const std::vector<Value*>&);
%ignore V8Simple::Function::Call(Value** args, int numArgs);
%newobject V8Simple::Function::Construct(const std::vector<Value*>&);
%ignore V8Simple::Function::Construct(Value** args, int numArgs);
%ignore V8Simple::Value::Delete();
%ignore V8Simple::String::New(const char* value, int length);
%ignore V8Simple::Primitive<int>::New(const int& value);
%ignore V8Simple::Primitive<double>::New(const double& value);
%ignore V8Simple::Primitive<bool>::New(const bool& value);
%ignore V8Simple::Context::New(ScriptExceptionHandler* scriptExceptionHandler, MessageHandler* runtimeExceptionHandler);
%newobject V8Simple::Array::Get(int);
%newobject V8Simple::ScriptException::GetName();
%newobject V8Simple::ScriptException::GetErrorMessage();
%newobject V8Simple::ScriptException::GetFileName();
%newobject V8Simple::ScriptException::GetStackTrace();
%newobject V8Simple::ScriptException::GetSourceLine();
%newobject V8Simple::UniqueValueVector::Get(int);
%newobject V8Simple::String::Copy();
%newobject V8Simple::ScriptException::Copy();
%feature("director") V8Simple::MessageHandler;
%feature("director") V8Simple::ScriptExceptionHandler;
%feature("director") V8Simple::Callback;
%typemap(throws, canthrow=1) std::runtime_error {
  SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, $1.what());
  return $null;
}
%ignore V8Simple::CurrentContext();
%ignore V8Simple::CurrentIsolate();
%ignore V8Simple::Throw(const V8Scope& scope);
%include "V8Simple.h"
%template(Int) V8Simple::Primitive<int>;
%template(Double) V8Simple::Primitive<double>;
%template(Bool) V8Simple::Primitive<bool>;
%template(StringVector) std::vector<V8Simple::String>;
%template(ValueVector) std::vector<V8Simple::Value*>;

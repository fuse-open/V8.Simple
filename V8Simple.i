%module(directors="1") v8
%{
#include "V8Simple.h"
%}

%include "arrays_csharp.i"
%apply unsigned char INPUT[] { const V8Simple::byte* buffer }
%apply unsigned char OUTPUT[] { V8Simple::byte* outBuffer }

// void* to IntPtr
%typemap(csdirectorin) void *VOID_INT_PTR "$1"
%apply void *VOID_INT_PTR { void * }

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

%typemap(cscode) SWIGTYPE %{
	public override bool Equals(object o) {
		if (o != null && o is $csclassname)
		{
			return Equals(($csclassname)o);
		}
		return false;
	}
	public bool Equals($csclassname o) {
		return o.swigCPtr.Handle.Equals(this.swigCPtr.Handle);
	}
	public override int GetHashCode() {
		return swigCPtr.Handle.GetHashCode();
	}
%}

%typemap(cscode) V8Simple::String %{
	public string GetValue()
	{
		var buffer = new byte[GetBufferLength()];
		GetBuffer(buffer);
		return System.Text.Encoding.UTF8.GetString(buffer);
	}
	private String(byte[] buffer) : this(buffer, buffer.Length) { }
	private String(string str) : this(System.Text.Encoding.UTF8.GetBytes(str)) { }
	public static String New(string str)
	{
		return str == null ? null : new String(str);
	}
%}

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
			case Type.External: return new External(cPtr, owner);
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
%newobject V8Simple::Context::Evaluate(const String*, const String*);
%newobject V8Simple::Context::GlobalObject();
%newobject V8Simple::Context::NewExternalArrayBuffer(void* data, int byteLength);
%newobject V8Simple::Context::ThrowException(Value* exception);
%newobject V8Simple::Object::Get(const String*);
%newobject V8Simple::Object::Keys();
%newobject V8Simple::Object::CallMethod(const String*, const std::vector<Value*>&);
%ignore V8Simple::Object::CallMethod(const String*, Value** args, int numArgs);
%newobject V8Simple::Function::Call(const std::vector<Value*>&);
%ignore V8Simple::Function::Call(Value** args, int numArgs);
%newobject V8Simple::Function::Construct(const std::vector<Value*>&);
%ignore V8Simple::Function::Construct(Value** args, int numArgs);
%ignore V8Simple::Value::Delete();
%newobject V8Simple::String::New(const byte* buffer, int bufferLength);
%ignore V8Simple::String::GetValue() const;
%ignore V8Simple::Primitive<int>::New(const int& value);
%ignore V8Simple::Primitive<double>::New(const double& value);
%ignore V8Simple::Primitive<bool>::New(const bool& value);
%ignore V8Simple::External::New(void* value);
%ignore V8Simple::Context::New(ScriptExceptionHandler* scriptExceptionHandler, MessageHandler* runtimeExceptionHandler);
%newobject V8Simple::Array::Get(int);
%newobject V8Simple::ScriptException::GetException();
%newobject V8Simple::ScriptException::GetErrorMessage();
%newobject V8Simple::ScriptException::GetFileName();
%newobject V8Simple::ScriptException::GetStackTrace();
%newobject V8Simple::ScriptException::GetSourceLine();
%newobject V8Simple::UniqueValueVector::Get(int);
%newobject V8Simple::String::Copy();
%newobject V8Simple::ScriptException::Copy();
%feature("director") V8Simple::MessageHandler;
%feature("director") V8Simple::ScriptExceptionHandler;
%feature("director") V8Simple::ExternalFreer;
%feature("director") V8Simple::Callback;
%typemap(throws, canthrow=1) std::runtime_error {
  SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, $1.what());
  return $null;
}
%ignore V8Simple::Throw(const V8Scope& scope);
%include "V8Simple.h"
%template(Int) V8Simple::Primitive<int>;
%template(Double) V8Simple::Primitive<double>;
%template(Bool) V8Simple::Primitive<bool>;
%template(StringVector) std::vector<V8Simple::String>;
%template(ValueVector) std::vector<V8Simple::Value*>;

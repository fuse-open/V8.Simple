%module(directors="1") v8
%{
#include "V8Simple.h"
%}
// Make sure we return the correct objects in C#:
// See: http://johnnado.com/swig-csharp-java-downcast/
%pragma(csharp) imclasscode=%{
	static $imclassname()
	{
		if (System.IO.Path.DirectorySeparatorChar == '\\') // Super-awesome and reliable Windows detection
		{
			var asmDir = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);

			switch (System.IntPtr.Size)
			{
				case 4: SetDllDirectory(System.IO.Path.Combine(asmDir, "x86")); break;
				case 8: SetDllDirectory(System.IO.Path.Combine(asmDir, "x64")); break;
				default: throw new System.Exception("Invalid IntPtr.Size: " + System.IntPtr.Size);
			}
		}
	}
	[System.Runtime.InteropServices.DllImport("kernel32.dll", CharSet = System.Runtime.InteropServices.CharSet.Auto, SetLastError = true)]
	private static extern bool SetDllDirectory(string path);
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
%newobject V8Simple::Object::CallMethod(const char*, const std::vector<Value*>&);
%newobject V8Simple::Function::Call(const std::vector<Value*>&);
%newobject V8Simple::Function::Construct(const std::vector<Value*>&);
%newobject V8Simple::Array::Get(int);
%newobject V8Simple::ScriptException::GetName();
%newobject V8Simple::ScriptException::GetErrorMessage();
%newobject V8Simple::ScriptException::GetFileName();
%newobject V8Simple::ScriptException::GetStackTrace();
%newobject V8Simple::ScriptException::GetSourceLine();
%newobject V8Simple::UniqueValueVector::Get(int);
%feature("director") V8Simple::MessageHandler;
%feature("director") V8Simple::ScriptExceptionHandler;
%feature("director") V8Simple::Callback;
%typemap(throws, canthrow=1) std::runtime_error {
  SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, $1.what());
  return $null;
}
%include "V8Simple.h"
%template(Int) V8Simple::Primitive<int>;
%template(Double) V8Simple::Primitive<double>;
%template(Bool) V8Simple::Primitive<bool>;
%template(StringVector) std::vector<V8Simple::String>;
%template(ValueVector) std::vector<V8Simple::Value*>;

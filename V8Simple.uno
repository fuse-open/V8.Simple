using Uno.Collections;
using Uno.Compiler.ExportTargetInterop;

namespace Fuse.Scripting.V8.Simple
{

	[DotNetType("Fuse.Scripting.V8.Simple.Context")]
	[TargetSpecificImplementation]
	public class Context
	{
		if defined(CPlusPlus)
			ContextImpl _contextImpl;

		extern (CPlusPlus) public ~Context()
		{
			extern (_contextImpl) "delete $0";
		}

		public Context(ScriptExceptionHandler scriptExceptionHandler)
		{
			if defined(CPlusPlus)
			{
				extern (_contextImpl, scriptExceptionHandler) "$0 = new Context($1)";
			}
		}

		public Value Evaluate(string fileName, string code)
		{
			if defined(CPlusPlus)
			{
				extern "const char* cfileName = ::uAllocCStr($0)";
				extern "const char* ccode = ::uAllocCStr($1)";
				Value result = Value.New(extern <ValueImpl> (_contextImpl) "$0->Evaluate(cfileName, ccode)");
				extern "::uFreeCStr($0)";
				extern "::uFreeCStr($1)";
				return result;
			}
		}

		public Object GlobalObject()
		{
			if defined(CPlusPlus)
			{
				return new Object(extern<ObjectImpl> (_contextImpl) "$0->GlobalObject());
			}
		}

		public bool IdleNotificationDeadline(double deadline_in_seconds)
		{
			if defined(CPlusPlus)
			{
				return extern<bool> (_contextImpl, deadline_in_seconds) "$0->IdleNotificationDeadline($1)";
			}
		}

		// TODO
		public static void SetDebugMessageHandler(DebugMessageHandler debugMessageHandler);
		public static void SendDebugCommand(string command);
		public static void ProcessDebugMessages();
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct ContextImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.ScriptException")]
	[TargetSpecificImplementation]
	public class ScriptException
	{
		
		if defined(CPlusPlus)
			ScriptExceptionImpl _scriptExceptionImpl;

		extern (CPlusPlus) ~ScriptException()
		{
			extern (_scriptExceptionImpl) "delete $0";
		}
		public string GetName()
		{
			if defined(CPlusPlus)
			{
				return extern<string> (_scriptExceptionImpl) "::uString::Utf8($0->GetName())";
			}
		}
		public string GetErrorMessage()
		{
			if defined(CPlusPlus)
			{
				return extern<string> (_scriptExceptionImpl) "::uString::Utf8($0->GetErrorMessage())";
			}
		}
		public string GetFileName();
		{
			if defined(CPlusPlus)
			{
				return extern<string> (_scriptExceptionImpl) "::uString::Utf8($0->GetFileName())";
			}
		}
		public int GetLineNumber();
		{
			if defined(CPlusPlus)
			{
				return extern<int> (_scriptExceptionImpl) "$0->GetLineNumber()";
			}
		}
		public string GetStackTrace();
		{
			if defined(CPlusPlus)
			{
				return extern<string> (_scriptExceptionImpl) "::uString::Utf8($0->GetStackTrace())";
			}
		}
		public string GetSourceLine();
		{
			if defined(CPlusPlus)
			{
				return extern<string> (_scriptExceptionImpl) "::uString::Utf8($0->GetSourceLine())";
			}
		}
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct ScriptExceptionImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.DebugMessageHandler")]
	[TargetSpecificImplementation]
	public class DebugMessageHandler
	{
		// TODO
		public virtual void Handle(string jsonMessage);
		public virtual void Retain();
		public virtual void Release();
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct DebugMessageHandlerImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.ScriptExceptionHandler")]
	[TargetSpecificImplementation]
	public class ScriptExceptionHandler
	{
		// TODO
		public virtual void Handle(ScriptException e);
		public virtual void Retain();
		public virtual void Release();
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct ScriptExceptionHandlerImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.Type")]
	public enum Type {
	  Int,
	  Double,
	  String,
	  Bool,
	  Object,
	  Array,
	  Function,
	  Callback
	}

	[DotNetType("Fuse.Scripting.V8.Simple.Value")]
	[TargetSpecificImplementation]
	public class Value
	{
		if defined(CPlusPlus)
			protected ValueImpl _valueImpl;

		internal extern(CPlusPlus) Value(ValueImpl impl)
		{
			_valueImpl = impl;
		}
		internal static extern(CPlusPlus) Value New(ValueImpl impl)
		{
			if (impl == default(ValueImpl))
			{
				return null;
			}
			var type = extern<Type> (impl) "$0->GetValueType()";
			switch (type)
			{
				case Type.Int: return new Int(impl);
				case Type.Double: return new Double(impl);
				case Type.String: return new String(impl);
				case Type.Bool: return new Bool(impl);
				case Type.Object: return new Object(impl);
				case Type.Array: return new Array(impl);
				case Type.Function: return new Function(impl);
			}
			throw new Exception("V8Simple: Unhandled type on Uno side: " + type);
		}
		extern(CPlusPlus) ~Value()
		{
			extern (_valueImpl) "delete $0";
		}
		public virtual Type GetValueType()
		{
			return extern<Type> (_valueImpl) "$0->GetValueType()";
		}
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct ValueImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.Object")]
	[TargetSpecificImplementation]
	public class Object: Value
	{
		if defined(CPlusPlus)
			protected ObjectImpl _objectImpl
			{
				get
				{
					return extern<ObjectImpl> (_valueImpl) "static_cast< @{ObjectImpl}>($0)";
				}
			}

		internal extern(CPlusPlus) Object(ValueImpl impl)
			: base(impl)
		{
		}
		public override Type GetValueType()
		{
			return extern<Type> (_objectImpl) "$0->GetValueType()";
		}
		public Value Get(string key)
		{
			extern "const char* ckey = ::uAllocCStr($0)";
			var result = Value.New(extern<ValueImpl> (_objectImpl) "$0->Get(ckey)");
			extern "::uFreeCStr(ckey)";
			return result;
		}
		public void Set(string key, Value value)
		{
			extern "const char* ckey = ::uAllocCStr($0)";
			var result = Value.New(extern<ValueImpl> (_objectImpl, value._valueImpl) "$0->Set(ckey, $1)");
			extern "::uFreeCStr(ckey)";
			return result;
		}
		// TODO
		public StringVector Keys();
		public bool InstanceOf(Function type)
		{
			return extern<bool> (_objectImpl, type._functionImpl) "$0->InstanceOf($1)";
		}
		// TODO
		public Value CallMethod(string name, ValueVector args);
		public bool ContainsKey(string key)
		{
			extern "const char* ckey = ::uAllocCStr($0)";
			var result = extern<bool> (_objectImpl) "$0->ContainsKey(ckey)";
			extern "::uFreeCStr(ckey)";
			return result
		}
		public bool Equals(Object arg)
		{
			return extern<bool> (_objectImpl, arg._objectImpl) "$0->Equals(*$1)";
		}
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct ObjectImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.Function")]
	[TargetSpecificImplementation]
	public class Function: Value
	{
		if defined(CPlusPlus)
			protected FunctionImpl _functionImpl
			{
				get
				{
					return extern<FunctionImpl> (_valueImpl) "static_cast< @{FunctionImpl}>($0)";
				}
			}

		internal extern(CPlusPlus) Function(ValueImpl impl)
			base(impl)
		{
		}
		public override Type GetValueType()
		{
			return extern<Type> (_functionImpl) "$0->GetValueType()";
		}
		// TODO
		public Value Call(ValueVector args);
		// TODO
		public Object Construct(ValueVector args);
		public bool Equals(Function f)
		{
			return extern<bool> (_functionImpl, f._functionImpl) "$0->Equals(*$1)";
		}
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct FunctionImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.Array")]
	[TargetSpecificImplementation]
	public class Array : Value
	{
		if defined(CPlusPlus)
			protected ArrayImpl _arrayImpl
			{
				get
				{
					return extern<ArrayImpl> (_valueImpl) "static_cast< @{ArrayImpl}>($0)";
				}
			}

		internal extern(CPlusPlus) Array(ValueImpl impl)
			base(impl)
		{
		}
		public override Type GetValueType()
		{
			return extern<Type> (_arrayImpl) "$0->GetValueType()";
		}
		public Value Get(int index)
		{
			return Value.New(extern<ValueImpl> (_arrayImpl, index) "$0->Get($1)");
		}
		public void Set(int index, Value value)
		{
			extern (_arrayImpl, index, value._valueImpl) "$0->Set($1, $2)";
		}
		public int Length()
		{
			return extern<int> (_arrayImpl) "$0->Length()";
		}
		public bool Equals(Array array)
		{
			return extern<bool> (_arrayImpl, array._arrayImpl) "$0->Equals(*$1)";
		}
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct ArrayImpl { }

	// TODO
	[DotNetType("Fuse.Scripting.V8.Simple.Callback")]
	[TargetSpecificImplementation]
	public class Callback: Value
	{
		public override Type GetValueType();
		public virtual Value Call(ValueVector args);
		public virtual void Retain();
		public virtual void Release();
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct CallbackImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.Int")]
	[TargetSpecificImplementation]
	public class Int: Value
	{
		if defined(CPlusPlus)
			protected IntImpl _intImpl
			{
				get
				{
					return extern<IntImpl> (_valueImpl) "static_cast< @{IntImpl}>($0)";
				}
			}

		internal extern(CPlusPlus) Int(ValueImpl impl)
			base(impl)
		{
		}
		public override Type GetValueType()
		{
			return extern<Type> (_intImpl) "$0->GetValueType()";
		}
		public Int(int value)
			base(extern<ValueImpl> "new ::V8Simple::Int($0)")
		{
		}
		public int GetValue()
		{
			return extern<int> (_intImpl) "$0->GetValue()";
		}
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct IntImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.Double")]
	[TargetSpecificImplementation]
	public class Double: Value
	{
		if defined(CPlusPlus)
			protected DoubleImpl _doubleImpl
			{
				get
				{
					return extern<DoubleImpl> (_valueImpl) "static_cast< @{DoubleImpl}>($0)";
				}
			}

		internal extern(CPlusPlus) Double(ValueImpl impl)
			base(impl)
		{
		}
		public override Type GetValueType()
		{
			return extern<Type> (_doubleImpl) "$0->GetValueType()";
		}
		public Double(double value)
			base(extern<ValueImpl> "new ::V8Simple::Double($0)")
		{
		}
		public double GetValue()
		{
			return extern<double> (_doubleImpl) "$0->GetValue()";
		}
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct DoubleImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.String")]
	[TargetSpecificImplementation]
	public class String: Value
	{
		if defined(CPlusPlus)
			protected StringImpl _stringImpl
			{
				get
				{
					return extern<StringImpl> (_valueImpl) "static_cast< @{StringImpl}>($0)";
				}
			}

		internal extern(CPlusPlus) String(ValueImpl impl)
			base(impl)
		{
		}
		public override Type GetValueType()
		{
			return extern<Type> (_stringImpl) "$0->GetValueType()";
		}
		public String(string value)
		{
			extern "const char* cvalue = ::uAllocCStr($0)";
			_valueImpl = extern<ValueImpl> "new ::V8Simple::String(cvalue)")
			extern "::uFreeCStr(cvalue)";
		}
		public string GetValue()
		{
			return extern<string> (_stringImpl) "::uString::Utf8($0->GetValue())";
		}
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct StringImpl { }

	[DotNetType("Fuse.Scripting.V8.Simple.Bool")]
	[TargetSpecificImplementation]
	public class Bool: Value
	{
		if defined(CPlusPlus)
			protected BoolImpl _boolImpl
			{
				get
				{
					return extern<BoolImpl> (_valueImpl) "static_cast< @{BoolImpl}>($0)";
				}
			}

		internal extern(CPlusPlus) Bool(ValueImpl impl)
			base(impl)
		{
		}
		public override Type GetValueType()
		{
			return extern<Type> (_boolImpl) "$0->GetValueType()";
		}
		public Bool(bool value)
			base(extern<ValueImpl> "new ::V8Simple::Bool($0)")
		{
		}
		public bool GetValue()
		{
			return extern<bool> (_boolImpl) "$0->GetValue()";
		}
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct BoolImpl { }

	// TODO
	[DotNetType("Fuse.Scripting.V8.Simple.StringVector")]
	[TargetSpecificImplementation]
	public class StringVector: IList<string>
	{
		public StringVector();
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct StringVectorImpl { }

	// TODO
	[DotNetType("Fuse.Scripting.V8.Simple.ValueVector")]
	[TargetSpecificImplementation]
	public class ValueVector: IList<Value>
	{
		public ValueVector();
	}

	[TargetSpecificType]
	internal extern(CPlusPlus) struct ValueVectorImpl { }
}

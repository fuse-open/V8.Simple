using Uno.Collections;
using Uno.Compiler.ExportTargetInterop;

namespace Fuse.Scripting.V8.Simple
{
	[DotNetType("Fuse.Scripting.V8.Simple.ScriptException")]
	public class ScriptException
	{
		public string GetName();
		public string GetErrorMessage();
		public string GetFileName();
		public int GetLineNumber();
		public string GetStackTrace();
		public string GetSourceLine();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.Context")]
	public class Context
	{
		public Context(ScriptExceptionHandler scriptExceptionHandler);
		public Value Evaluate(string fileName, string code);
		public Object GlobalObject();
		public bool IdleNotificationDeadline(double deadline_in_seconds);
		public static void SetDebugMessageHandler(DebugMessageHandler debugMessageHandler);
		public static void SendDebugCommand(string command);
		public static void ProcessDebugMessages();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.DebugMessageHandler")]
	public class DebugMessageHandler
	{
		public virtual void Handle(string jsonMessage)
		public virtual void Retain();
		public virtual void Release();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.ScriptExceptionHandler")]
	public class ScriptExceptionHandler
	{
		public virtual void Handle(ScriptException e);
		public virtual void Retain();
		public virtual void Release();
	}

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
	public class Value
	{
		public virtual Type GetValueType();
		public Int AsInt();
		public Double AsDouble();
		public String AsString();
		public Bool AsBool();
		public Object AsObject();
		public Function AsFunction();
		public Array AsArray();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.Object")]
	public class Object: Value
	{
		public override Type GetValueType();
		public Value Get(string key);
		public void Set(string key, Value value);
		public StringVector Keys();
		public bool InstanceOf(Function type);
		public Value CallMethod(string name, ValueVector args);
		public bool ContainsKey(string key);
		public bool Equals(Object arg0);
	}

	[DotNetType("Fuse.Scripting.V8.Simple.Function")]
	public class Function: Value
	{
		public override Type GetValueType();
		public Value Call(ValueVector args);
		public Object Construct(ValueVector args);
		public bool Equals(Function f);
	}

	[DotNetType("Fuse.Scripting.V8.Simple.Array")]
	public class Array : Value
	{
		public override Type GetValueType();
		public Value Get(int index);
		public void Set(int index, Value value);
		public int Length();
		public bool Equals(Array array);
	}

	[DotNetType("Fuse.Scripting.V8.Simple.Callback")]
	public class Callback: Value
	{
		public override Type GetValueType();
		public virtual Value Call(ValueVector args);
		public virtual Callback Clone();
		public virtual void Retain();
		public virtual void Release();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.Int")]
	public class Int: Value
	{
		public Int(int value);
		public override Type GetValueType();
		public int GetValue();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.Double")]
	public class Double: Value
	{
		public Double(double value);
		public override Type GetValueType();
		public double GetValue();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.String")]
	public class String: Value
	{
		public String(string value);
		public override Type GetValueType();
		public string GetValue();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.Bool")]
	public class Bool: Value
	{
		public Bool(bool value);
		public override Type GetValueType();
		public bool GetValue();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.StringVector")]
	public class StringVector: IList<string>
	{
		public StringVector();
	}

	[DotNetType("Fuse.Scripting.V8.Simple.ValueVector")]
	public class ValueVector: IList<Value>
	{
		public ValueVector();
	}
}

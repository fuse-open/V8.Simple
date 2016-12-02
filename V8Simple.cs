using System;
using System.Runtime.InteropServices;
using System.Text;
namespace Fuse.Scripting.V8.Simple
{
// -------------------------------------------------------------------------
// Types
public enum JSType
{
	Null,
	Int,
	Double,
	String,
	Bool,
	Object,
	Array,
	Function,
	External,
}
public enum JSRuntimeError
{
	NoError,
	InvalidCast,
	StringTooLong,
	TypeError,
}
[StructLayout(LayoutKind.Sequential)]
public struct JSContext
{
	readonly IntPtr _handle;
}
[StructLayout(LayoutKind.Sequential)]
public struct JSValue
{
	readonly IntPtr _handle;
}
[StructLayout(LayoutKind.Sequential)]
public struct JSString
{
	readonly IntPtr _handle;
}
[StructLayout(LayoutKind.Sequential)]
public struct JSObject
{
	readonly IntPtr _handle;
}
[StructLayout(LayoutKind.Sequential)]
public struct JSArray
{
	readonly IntPtr _handle;
}
[StructLayout(LayoutKind.Sequential)]
public struct JSFunction
{
	readonly IntPtr _handle;
}
[StructLayout(LayoutKind.Sequential)]
public struct JSExternal
{
	readonly IntPtr _handle;
}
[StructLayout(LayoutKind.Sequential)]
public struct JSScriptException
{
	readonly IntPtr _handle;
	public override bool Equals(object that) { return that is JSScriptException ? this == (JSScriptException)that : false; }
	public override int GetHashCode() { return _handle.GetHashCode(); }
	public static bool operator ==(JSScriptException e1, JSScriptException e2) { return e1._handle == e2._handle; }
	public static bool operator !=(JSScriptException e1, JSScriptException e2) { return e1._handle != e2._handle; }
}
public delegate JSValue JSCallback(JSContext context, IntPtr data, [In, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]JSValue[] args, int numArgs, out JSValue error);
public delegate void JSExternalFinalizer(IntPtr external);
public delegate void JSCallbackFinalizer(IntPtr data);
public delegate void JSDebugMessageHandler(IntPtr data, JSString message);
// -------------------------------------------------------------------------
// Context
public static class Context
{
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="RetainJSContext")]
public static extern void Retain(JSContext context);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="ReleaseJSContext")]
public static extern void Release(JSContext context);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CreateJSContext")]
public static extern JSContext Create([MarshalAs(UnmanagedType.FunctionPtr)]JSCallbackFinalizer callbackFinalizer, [MarshalAs(UnmanagedType.FunctionPtr)]JSExternalFinalizer externalFinalizer);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSContextEvaluateCreate")]
public static extern JSValue EvaluateCreate(JSContext context, JSString fileName, JSString code, out JSScriptException error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSContextCopyGlobalObject")]
public static extern JSObject CopyGlobalObject(JSContext context);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetV8Version")]
public static extern IntPtr GetV8VersionPtr();
public static string GetV8Version() { return Marshal.PtrToStringAnsi(GetV8VersionPtr()); }
}
// -------------------------------------------------------------------------
// Debug
public static class Debug
{
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="SetJSDebugMessageHandler")]
public static extern void SetMessageHandler(JSContext context, IntPtr data, [MarshalAs(UnmanagedType.FunctionPtr)]JSDebugMessageHandler messageHandler);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="SendJSDebugCommand")]
public static extern void SendCommand(JSContext context, [MarshalAs(UnmanagedType.LPWStr, SizeParamIndex = 2)]string command, int length);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="ProcessJSDebugMessages")]
public static extern void ProcessMessages(JSContext context);
}
// -------------------------------------------------------------------------
// Value
public static class Value
{
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetJSValueType")]
public static extern JSType GetType(JSValue value);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="RetainJSValue")]
public static extern void Retain(JSContext context, JSValue value);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="ReleaseJSValue")]
public static extern void Release(JSContext context, JSValue value);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSValueAsInt")]
public static extern int AsInt(JSValue value, out JSRuntimeError error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSValueAsDouble")]
public static extern double AsDouble(JSValue value, out JSRuntimeError error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSValueAsString")]
public static extern JSString AsString(JSValue value, out JSRuntimeError error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSValueAsBool")]
[return: MarshalAs(UnmanagedType.Bool)]
public static extern bool AsBool(JSValue value, out JSRuntimeError error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSValueAsObject")]
public static extern JSObject AsObject(JSValue value, out JSRuntimeError error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSValueAsArray")]
public static extern JSArray AsArray(JSValue value, out JSRuntimeError error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSValueAsFunction")]
public static extern JSFunction AsFunction(JSValue value, out JSRuntimeError error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSValueAsExternal")]
public static extern JSExternal AsExternal(JSValue value, out JSRuntimeError error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSValueStrictEquals")]
[return: MarshalAs(UnmanagedType.Bool)]
public static extern bool StrictEquals(JSContext context, JSValue obj1, JSValue obj2);
// --------------------------------------------------------------------------
// Primitives
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSNull")]
public static extern JSValue JSNull();
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CreateJSInt")]
public static extern JSValue CreateInt(int value);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CreateJSDouble")]
public static extern JSValue CreateDouble(double value);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CreateJSBool")]
public static extern JSValue CreateBool([MarshalAs(UnmanagedType.Bool)]bool value);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CreateExternalJSArrayBuffer")]
public static extern JSObject CreateExternalArrayBuffer(JSContext context, IntPtr data, int byteLength);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CreateJSCallback")]
public static extern JSFunction CreateCallback(JSContext context, IntPtr data, [MarshalAs(UnmanagedType.FunctionPtr)]JSCallback callback, out JSScriptException error);
// --------------------------------------------------------------------------
// String
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CreateJSString")]
public static extern JSString CreateString(JSContext context, [MarshalAs(UnmanagedType.LPWStr, SizeParamIndex = 2)]string buffer, int length, out JSRuntimeError error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSStringLength")]
public static extern int Length(JSContext context, JSString str);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="WriteJSStringBuffer")]
public static extern void Write(JSContext context, JSString str, [Out, MarshalAs(UnmanagedType.LPWStr)]StringBuilder buffer);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSStringAsValue")]
public static extern JSValue AsValue(JSString str);
// -------------------------------------------------------------------------
// Object
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CopyJSObjectProperty")]
public static extern JSValue CopyProperty(JSContext context, JSObject obj, JSString key, out JSScriptException error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="SetJSObjectProperty")]
public static extern void SetProperty(JSContext context, JSObject obj, JSString key, JSValue value, out JSScriptException error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CopyJSObjectOwnPropertyNames")]
public static extern JSArray CopyOwnPropertyNames(JSContext context, JSObject obj, out JSScriptException error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSObjectHasProperty")]
[return: MarshalAs(UnmanagedType.Bool)]
public static extern bool HasProperty(JSContext context, JSObject obj, JSString key, out JSScriptException error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetJSObjectArrayBufferData")]
public static extern IntPtr GetArrayBufferData(JSContext context, JSObject obj, out JSRuntimeError outError);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSObjectAsValue")]
public static extern JSValue AsValue(JSObject obj);
// -------------------------------------------------------------------------
// Array
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CopyJSArrayPropertyAtIndex")]
public static extern JSValue CopyProperty(JSContext context, JSArray arr, int index, out JSScriptException error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="SetJSArrayPropertyAtIndex")]
public static extern void SetProperty(JSContext context, JSArray arr, int index, JSValue value, out JSScriptException error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSArrayLength")]
public static extern int Length(JSContext context, JSArray arr);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSArrayAsValue")]
public static extern JSValue AsValue(JSArray arr);
// -------------------------------------------------------------------------
// Function
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CallJSFunctionCreate")]
public static extern JSValue CallCreate(JSContext context, JSFunction function, JSObject thisObject, [In, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 4)]JSValue[] args, int numArgs, out JSScriptException error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="ConstructJSFunctionCreate")]
public static extern JSObject ConstructCreate(JSContext context, JSFunction function, [In, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)]JSValue[] args, int numArgs, out JSScriptException error);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSFunctionAsValue")]
public static extern JSValue AsValue(JSFunction fun);
// -------------------------------------------------------------------------
// External
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="CreateJSExternal")]
public static extern JSExternal CreateExternal(JSContext context, IntPtr value);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetJSExternalValue")]
public static extern IntPtr GetExternalValue(JSContext context, JSExternal external);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="JSExternalAsValue")]
public static extern JSValue AsValue(JSExternal external);
}
// -------------------------------------------------------------------------
// Exceptions
public static class ScriptException
{
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="RetainJSScriptException")]
public static extern void Retain(JSContext context, JSScriptException e);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="ReleaseJSScriptException")]
public static extern void Release(JSContext context, JSScriptException e);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetJSScriptException")]
public static extern JSValue GetException(JSScriptException e);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetJSScriptExceptionMessage")]
public static extern JSString GetMessage(JSScriptException e);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetJSScriptExceptionFileName")]
public static extern JSString GetFileName(JSScriptException e);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetJSScriptExceptionLineNumber")]
public static extern int GetLineNumber(JSScriptException e);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetJSScriptExceptionStackTrace")]
public static extern JSString GetStackTrace(JSScriptException e);
[DllImport("V8Simple.dll", CallingConvention = CallingConvention.Cdecl, EntryPoint="GetJSScriptExceptionSourceLine")]
public static extern JSString GetSourceLine(JSScriptException e);
}
}

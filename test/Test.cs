using Fuse.Scripting.V8.Simple;
using V8Simple = Fuse.Scripting.V8.Simple;
using System;
using System.Collections.Generic;
using NUnit.Framework;
using System.Runtime.InteropServices;

[TestFixture]
public class V8SimpleTests
{
	V8Simple.String Str(string str)
	{
		return V8Simple.String.New(str);
	}

	[Test]
	public void PrimitiveTests()
	{
		using (var context = new Context(null, null, null))
		{
			{
				var result = context.Evaluate(Str("ValueTests"), Str("12 + 13"));
				Assert.AreEqual(result.GetValueType(), V8Simple.Type.Int);
				Assert.AreEqual(((V8Simple.Int)result).GetValue(), 25);
			}
			{
				var result = context.Evaluate(Str("ValueTests"), Str("1.2 + 1.3"));
				Assert.AreEqual(result.GetValueType(), V8Simple.Type.Double);
				Assert.AreEqual(((V8Simple.Double)result).GetValue(), 2.5);
			}
			{
				var result = context.Evaluate(Str("ValueTests"), Str("\"abc 123\""));
				Assert.AreEqual(result.GetValueType(), V8Simple.Type.String);
				Assert.AreEqual(((V8Simple.String)result).GetValue(), "abc 123");
			}
			{
				var result = context.Evaluate(Str("ValueTests"), Str("true || false"));
				Assert.AreEqual(result.GetValueType(), V8Simple.Type.Bool);
				Assert.AreEqual(((V8Simple.Bool)result).GetValue(), true);
			}
		}
	}

	[Test]
	public void ObjectTests()
	{
		using (var context = new Context(null, null, null))
		{
			{
				var obj = (V8Simple.Object)context.Evaluate(Str("ObjectTests"), Str("({ a: \"abc\", b: 123 })"));
				Assert.IsNotNull(obj);
				Assert.AreEqual(obj.GetValueType(), V8Simple.Type.Object);
				Assert.AreEqual(obj.Get(Str("a")).GetValueType(), V8Simple.Type.String);
				Assert.AreEqual(((V8Simple.String)obj.Get(Str("a"))).GetValue(), "abc");
				Assert.AreEqual(obj.Get(Str("b")).GetValueType(), V8Simple.Type.Int);
				Assert.AreEqual(((V8Simple.Int)obj.Get(Str("b"))).GetValue(), 123);
				obj.Set(Str("a"), new V8Simple.String(Str("xyz")));
				Assert.AreEqual(((V8Simple.String)obj.Get(Str("a"))).GetValue(), "xyz");
				obj.Set(Str("c"), new V8Simple.Double(123.4));
				Assert.AreEqual(((V8Simple.Double)obj.Get(Str("c"))).GetValue(), 123.4);
				var keys1 = obj.Keys();
				List<Value> keys1l = new List<Value>();
				for (int i = 0; i < keys1.Length(); ++i)
				{
					keys1l.Add(keys1.Get(i));
				}
				var keys2 = obj.Keys();
				List<Value> keys2l = new List<Value>();
				for (int i = 0; i < keys2.Length(); ++i)
				{
					keys2l.Add(keys2.Get(i));
				}
				Assert.AreEqual(keys1l.Count, 3);
				Assert.AreEqual(keys2l.Count, 3);
				Assert.IsTrue(obj.ContainsKey(Str("a")));
				Assert.IsTrue(obj.ContainsKey(Str("b")));
				Assert.IsTrue(obj.ContainsKey(Str("c")));
				Assert.IsFalse(obj.ContainsKey(Str("d")));
				Assert.IsTrue(obj.Equals(obj));
				Assert.IsTrue(obj.StrictEquals(obj));
				Assert.IsFalse(obj.Equals((V8Simple.Object)context.Evaluate(Str("ObjectTests"), Str("({ abc: \"abc\" })"))));
				Assert.IsFalse(obj.StrictEquals((V8Simple.Object)context.Evaluate(Str("ObjectTests"), Str("({ abc: \"abc\" })"))));
				obj.Set(Str("f"), context.Evaluate(Str("ObjectTests f"), Str("(function(x, y) { return x + y; })")));
				var callResult = obj.CallMethod(
					Str("f"),
					new ValueVector { new V8Simple.Int(12), new V8Simple.Int(13) });
				Assert.IsNotNull(callResult);
				Assert.AreEqual(callResult.GetValueType(), V8Simple.Type.Int);
				Assert.AreEqual(
					((V8Simple.Int)callResult).GetValue(),
					25);
				Assert.IsTrue(
					((V8Simple.Object)(context.Evaluate(Str("ObjectTests instanceof"), Str("new Map()"))))
					.InstanceOf((V8Simple.Function)context.Evaluate(Str("ObjectTests instanceof 2"), Str("Map"))));
				Assert.IsFalse(
					obj
					.InstanceOf((V8Simple.Function)context.Evaluate(Str("ObjectTests instanceof 2"), Str("Map"))));
			}
		}
	}

	[Test]
	public void ArrayTests()
	{
		using (var context = new Context(null, null, null))
		{
			var arr = (V8Simple.Array)context.Evaluate(Str("ArrayTests"), Str("[\"abc\", 123]"));
			Assert.AreEqual(arr.GetValueType(), V8Simple.Type.Array);
			Assert.AreEqual(arr.Length(), 2);
			Assert.IsTrue(arr.Equals(arr));
			Assert.IsTrue(arr.StrictEquals(arr));
			Assert.IsFalse(arr.Equals((V8Simple.Array)context.Evaluate(Str("ArrayTests"), Str("[1, 2, 3]"))));
			Assert.IsFalse(arr.StrictEquals((V8Simple.Array)context.Evaluate(Str("ArrayTests"), Str("[1, 2, 3]"))));
			Assert.AreEqual(((V8Simple.String)arr.Get(0)).GetValue(), "abc");
			Assert.AreEqual(((V8Simple.Int)arr.Get(1)).GetValue(), 123);
			arr.Set(1, new V8Simple.String(Str("123")));
			Assert.AreEqual(((V8Simple.String)arr.Get(1)).GetValue(), "123");
		}
	}

	[Test]
	public void FunctionTests()
	{
		using (var context = new Context(null, null, null))
		{
			var fun = (V8Simple.Function)context.Evaluate(Str("FunctionTests"), Str("(function(x, y) { return x * y; })"));
			Assert.IsNotNull(fun, "Test0");
			Assert.AreEqual(fun.GetValueType(), V8Simple.Type.Function, "Test1");
			var callResult = (V8Simple.Int)fun.Call(new ValueVector { new V8Simple.Int(11), new V8Simple.Int(12) });
			Assert.IsNotNull(callResult, "Test1.5");
			Assert.AreEqual(callResult.GetValue(), 132, "Test2");
			Assert.IsTrue(fun.Equals(fun), "Test3");
			Assert.IsTrue(fun.StrictEquals(fun), "Test3");
			var str = (V8Simple.Function)context.Evaluate(Str("FunctionTests construct"), Str("String"));
			Assert.IsNotNull(str, "Test3.5");
			Assert.IsFalse(fun.Equals(str), "Test4");
			Assert.IsFalse(fun.StrictEquals(str), "Test4");
			var obj = str.Construct(new ValueVector { new V8Simple.String(Str("abc 123")) });
			Assert.IsNotNull(obj);
			Assert.AreEqual(((V8Simple.Int)obj.CallMethod(Str("indexOf"), new ValueVector { new V8Simple.String(Str("1")) })).GetValue(), 4, "Test5");
		}
	}

	class MyCallback: Callback
	{
		static List<object> _retained = new List<object>();
		public override void Retain()
		{
			_retained.Add(this);
		}
		public override void Release()
		{
			_retained.Remove(this);
		}
		public override Value Call(UniqueValueVector args)
		{
			if (args.Length() == 2)
			{
				var x = ((V8Simple.Int)args.Get(0)).GetValue();
				var y = ((V8Simple.Int)args.Get(1)).GetValue();
				return new V8Simple.Int(x + y + 1000);
			}
			return null;
		}
	}

	class DelegateCallback: Callback
	{
		static List<object> _retained = new List<object>();
		Func<UniqueValueVector, Value> _f;
		public DelegateCallback(Func<UniqueValueVector, Value> f)
		{
			_f = f;
		}
		public override void Retain()
		{
			_retained.Add(this);
		}
		public override void Release()
		{
			_retained.Remove(this);
		}
		public override Value Call(UniqueValueVector args)
		{
			return _f(args);
		}
	}

	[Test]
	public void CallbackTests()
	{
		using (var context = new Context(null, null, null))
		{
			var f = (V8Simple.Function)context.Evaluate(
				Str("CallbackTests"),
				Str("(function(f) { return f(12, 13) + f(10, 20); })"));
			context.IdleNotificationDeadline(1);
			System.GC.Collect();
			System.GC.WaitForPendingFinalizers();
			Assert.AreEqual(
				((V8Simple.Int)f.Call(new ValueVector { new MyCallback() })).GetValue(),
				12 + 13 + 1000 + 10 + 20 + 1000);
			context.IdleNotificationDeadline(1);
			System.GC.Collect();
			System.GC.WaitForPendingFinalizers();
		}
	}

	[Test]
	public void CallbackTests2()
	{
		using (var context = new Context(null, null, null))
		{
			for (int i = 0; i < 10; ++i)
			{
				var f = (V8Simple.Function)context.Evaluate(
					Str("CallbackTests2"),
					Str("(function(f) { return (function() { return f(12, 13) + f(10, 20); }); })"));
				var g = (V8Simple.Function)f.Call(new ValueVector { new MyCallback() });
				context.IdleNotificationDeadline(1);
				System.GC.Collect();
				System.GC.WaitForPendingFinalizers();
				Assert.AreEqual(
					((V8Simple.Int)g.Call(new ValueVector { })).GetValue(),
					12 + 13 + 1000 + 10 + 20 + 1000);
			}
		}
	}

	public class DelegateScriptExceptionHandler: ScriptExceptionHandler
	{
		Action<ScriptException> _handler;

		public DelegateScriptExceptionHandler(Action<ScriptException> handler)
		{
			_handler = handler;
		}

		public override void Handle(ScriptException e)
		{
			_handler(e);
		}
	}

	[Test]
	public void ErrorTests()
	{
		bool handled;
		bool runtimeHandled;
		var scriptExceptionHandler = new DelegateScriptExceptionHandler(x => { handled = true; });
		var runtimeExceptionHandler = new DelegateMessageHandler(x => { runtimeHandled = true; });
		using (var context = new Context(
			scriptExceptionHandler,
			runtimeExceptionHandler,
			null))
		{
			handled = false;
			context.Evaluate(Str("ErrorTests"), Str("new ...."));
			Assert.IsTrue(handled, "Test1");

			handled = false;
			context.Evaluate(Str("ErrorTests"), Str("obj.someMethod()"));
			Assert.IsTrue(handled, "Test2");

			handled = false;
			context.Evaluate(Str("ErrorTests"), Str("throw \"Hello\";"));
			Assert.IsTrue(handled, "Test3");

			var obj = (V8Simple.Object)context.Evaluate(Str("ErrorTests"), Str("({})"));

			runtimeHandled = false;
			obj.ContainsKey(null);
			Assert.IsTrue(runtimeHandled, "Test4");

			runtimeHandled = false;
			obj.Get(null);
			Assert.IsTrue(runtimeHandled, "Test5");

			runtimeHandled = false;
			obj.Set(null, null);
			Assert.IsTrue(runtimeHandled, "Test6");

			runtimeHandled = false;
			obj.CallMethod(null, new ValueVector { });
			Assert.IsTrue(runtimeHandled, "Test7");

			runtimeHandled = false;
			V8Simple.Context.SendDebugCommand(null);
			Assert.IsTrue(runtimeHandled, "Test8");

			runtimeHandled = false;
			context.Evaluate(null, Str("({})"));
			Assert.IsTrue(runtimeHandled, "Test9");

			runtimeHandled = false;
			context.Evaluate(Str("ErrorTests"), null);
			Assert.IsTrue(runtimeHandled, "Test10");
		}
	}

	public class DelegateMessageHandler: MessageHandler
	{
		Action<string> _handler;

		public DelegateMessageHandler(Action<string> handler)
		{
			_handler = handler;
		}

		public override void Handle(V8Simple.String e)
		{
			_handler(e.GetValue());
		}
	}

	[Test]
	public void DebuggerTests()
	{
		V8Simple.Context.SetDebugMessageHandler(null);
		V8Simple.Context.ProcessDebugMessages();
		using (var context = new Context(null, null, null))
		{
			var messageHandler = new DelegateMessageHandler(x => { return; });
			V8Simple.Context.SetDebugMessageHandler(messageHandler);
			V8Simple.Context.SendDebugCommand(Str("{}"));
			V8Simple.Context.ProcessDebugMessages();
		}
		V8Simple.Context.SetDebugMessageHandler(null);
		V8Simple.Context.ProcessDebugMessages();
	}

	[Test]
	public void VersionTests()
	{
		Assert.IsNotNull(Context.GetVersion());
	}

	[Test]
	public void UnicodeTests()
	{
		string str = "ç, é, õ";
		using (var context = new Context(null, null, null))
		{
			var res = ((V8Simple.String)context.Evaluate(Str("UnicodeTests"), Str("\"" + str + "\""))).GetValue();
			Assert.AreEqual(str, res);
		}
	}

	[Test]
	public void CallbackExceptionTests()
	{
		bool handled;
		var scriptExceptionHandler = new DelegateScriptExceptionHandler(x => { handled = true; });
		using (var context = new Context(
			scriptExceptionHandler,
			null,
			null))
		{
			var f = context.Evaluate(Str("CallbackExceptionTests"), Str("(function(f) { f(); })")) as V8Simple.Function;
			handled = false;
			f.Call(new ValueVector {new DelegateCallback(x => Context.ThrowException(Str("My Exception")))});
			Assert.IsTrue(handled, "Test1");
		}
	}

	class SomeObject
	{
		public string SomeField;
		public SomeObject(string someField)
		{
			SomeField = someField;
		}
	}

	class MyExternalFreer: ExternalFreer
	{
		public static IntPtr GetIntPtr(object o)
		{
			return GCHandle.ToIntPtr(GCHandle.Alloc(o));
		}
		public static object GetObject(IntPtr ptr)
		{
			return GCHandle.FromIntPtr(ptr).Target;
		}
		public override void Free(IntPtr ptr)
		{
			GCHandle.FromIntPtr(ptr).Free();
		}
	}

	[Test]
	public void ExternalTests()
	{
		using (var context = new Context(null, null, new MyExternalFreer()))
		{
			SomeObject someObject = new SomeObject("theField");
			var f = context.Evaluate(Str("ExternalTests"), Str("(function(x) { return x; })")) as V8Simple.Function;
			var res = f.Call(new ValueVector { new External(MyExternalFreer.GetIntPtr(someObject)) });
			Assert.AreEqual(res.GetValueType(), V8Simple.Type.External);
			var ext = res as V8Simple.External;
			Assert.IsNotNull(ext);
			var o1 = ext.GetValue();
			Assert.IsNotNull(o1);
			var o2 = MyExternalFreer.GetObject(o1);
			Assert.IsNotNull(o2);
			var o3 = o2 as SomeObject;
			Assert.IsNotNull(o3);
			Assert.AreEqual(o3.SomeField, "theField");
			context.IdleNotificationDeadline(1);
			System.GC.Collect();
			System.GC.WaitForPendingFinalizers();

			var res2 = f.Call(new ValueVector { ext });
			Assert.AreEqual(res2.GetValueType(), V8Simple.Type.External);
			var ext2 = res2 as V8Simple.External;
			var o12 = ext2.GetValue();
			Assert.IsNotNull(o12);
			var o22 = MyExternalFreer.GetObject(o12);
			Assert.IsNotNull(o22);
			var o32 = o22 as SomeObject;
			Assert.IsNotNull(o32);
			Assert.AreEqual(o32.SomeField, "theField");
		}
	}

	class ArrayMarshaller
	{
		readonly byte[] _array;
		readonly GCHandle _handle;

		public ArrayMarshaller(byte[] array)
		{
			_array = array;
			_handle = GCHandle.Alloc(array, GCHandleType.Pinned);
		}

		~ArrayMarshaller()
		{
			_handle.Free();
		}

		public IntPtr GetIntPtr()
		{
			return Marshal.UnsafeAddrOfPinnedArrayElement(_array, 0);
		}
	}

	[Test]
	public void ArrayBufferTests()
	{
		using (var context = new Context(null, null, new MyExternalFreer()))
		{
			var len = 100;
			var buf = new byte[len];
			var sum = 0;
			for (byte i = 0; i < len; ++i)
			{
				buf[i] = i;
				sum += i;
			}
			var marshaller = new ArrayMarshaller(buf);
			var arrayBuffer = Context.NewExternalArrayBuffer(marshaller.GetIntPtr(), len);
			{
				var arrayBufferType = context.GlobalObject().Get(Str("ArrayBuffer")) as V8Simple.Function;
				Assert.IsTrue(arrayBuffer.InstanceOf(arrayBufferType));
			}
			{
				var f = context.Evaluate(
					Str("ArrayBufferTests"),
					Str("(function (buf) { return new Uint8Array(buf).reduce(function(acc, x) { return acc + x; }); })")) as Function;
				var res = f.Call(new ValueVector { arrayBuffer }) as V8Simple.Int;
				Assert.AreEqual(sum, res.GetValue());
			}
			{
				var f = context.Evaluate(
					Str("ArrayBufferTests"),
					Str("(function (buf) { return buf; })")) as V8Simple.Function;
				var arrayBuffer2 = f.Call(new ValueVector{ arrayBuffer }) as V8Simple.Object;
				var ptr = arrayBuffer2.GetArrayBufferData();
				var buf2 = new byte[len];
				Marshal.Copy(ptr, buf2, 0, len);
				for (int i = 0; i < len; ++i)
				{
					Assert.AreEqual(buf[i], buf2[i]);
				}
			}
			{
				var f = context.Evaluate(
					Str("ArrayBufferTests"),
					Str("(function (len) { var buf = new ArrayBuffer(len); var x = new Uint8Array(buf); for (var i = 0; i < len; ++i) x[i] = i; return buf; })")) as V8Simple.Function;
				var arrayBuffer2 = f.Call(new ValueVector{ new V8Simple.Int(len) }) as V8Simple.Object;
				var ptr = arrayBuffer2.GetArrayBufferData();
				var buf2 = new byte[len];
				Marshal.Copy(ptr, buf2, 0, len);
				for (int i = 0; i < len; ++i)
				{
					Assert.AreEqual(buf[i], buf2[i]);
				}
			}
			{
				var o = context.Evaluate(
					Str("ArrayBufferTests"),
					Str("({})")) as V8Simple.Object;
				Assert.AreEqual(IntPtr.Zero, o.GetArrayBufferData());
			}
		}
	}

	// Has to be last
	[Test]
	public void ZZZContextTests()
	{
		bool handled;
		var scriptExceptionHandler = new DelegateScriptExceptionHandler(x => { handled = true; });
		var runtimeExceptionHandler = new DelegateMessageHandler(x => { handled = true; });
		var context = new Context(scriptExceptionHandler, runtimeExceptionHandler, null);
		Assert.AreEqual(
			((V8Simple.Int)context.Evaluate(Str("ContextTests"), Str("1 + 2"))).GetValue(),
			3);
	}
}

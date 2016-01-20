using Fuse.Scripting.V8.Simple;
using V8Simple = Fuse.Scripting.V8.Simple;
using System;
using System.Collections.Generic;
using NUnit.Framework;

[TestFixture]
public class V8SimpleTests
{
	[Test]
	public void PrimitiveTests()
	{
		using (var context = new Context(null, null))
		{
			{
				var result = context.Evaluate("ValueTests", "12 + 13");
				Assert.AreEqual(result.GetValueType(), V8Simple.Type.Int);
				Assert.AreEqual(((V8Simple.Int)result).GetValue(), 25);
			}
			{
				var result = context.Evaluate("ValueTests", "1.2 + 1.3");
				Assert.AreEqual(result.GetValueType(), V8Simple.Type.Double);
				Assert.AreEqual(((V8Simple.Double)result).GetValue(), 2.5);
			}
			{
				var result = context.Evaluate("ValueTests", "\"abc 123\"");
				Assert.AreEqual(result.GetValueType(), V8Simple.Type.String);
				Assert.AreEqual(((V8Simple.String)result).GetValue(), "abc 123");
			}
			{
				var result = context.Evaluate("ValueTests", "true || false");
				Assert.AreEqual(result.GetValueType(), V8Simple.Type.Bool);
				Assert.AreEqual(((V8Simple.Bool)result).GetValue(), true);
			}
		}
	}

	[Test]
	public void ObjectTests()
	{
		using (var context = new Context(null, null))
		{
			{
				var obj = (V8Simple.Object)context.Evaluate("ObjectTests", "({ a: \"abc\", b: 123 })");
				Assert.IsNotNull(obj);
				Assert.AreEqual(obj.GetValueType(), V8Simple.Type.Object);
				Assert.AreEqual(obj.Get("a").GetValueType(), V8Simple.Type.String);
				Assert.AreEqual(((V8Simple.String)obj.Get("a")).GetValue(), "abc");
				Assert.AreEqual(obj.Get("b").GetValueType(), V8Simple.Type.Int);
				Assert.AreEqual(((V8Simple.Int)obj.Get("b")).GetValue(), 123);
				obj.Set("a", new V8Simple.String("xyz"));
				Assert.AreEqual(((V8Simple.String)obj.Get("a")).GetValue(), "xyz");
				obj.Set("c", new V8Simple.Double(123.4));
				Assert.AreEqual(((V8Simple.Double)obj.Get("c")).GetValue(), 123.4);
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
				Assert.IsTrue(obj.ContainsKey("a"));
				Assert.IsTrue(obj.ContainsKey("b"));
				Assert.IsTrue(obj.ContainsKey("c"));
				Assert.IsFalse(obj.ContainsKey("d"));
				Assert.IsTrue(obj.Equals(obj));
				Assert.IsTrue(obj.StrictEquals(obj));
				Assert.IsFalse(obj.Equals((V8Simple.Object)context.Evaluate("ObjectTests", "({ abc: \"abc\" })")));
				Assert.IsFalse(obj.StrictEquals((V8Simple.Object)context.Evaluate("ObjectTests", "({ abc: \"abc\" })")));
				obj.Set("f", context.Evaluate("ObjectTests f", "(function(x, y) { return x + y; })"));
				var callResult = obj.CallMethod(
					"f",
					new ValueVector { new V8Simple.Int(12), new V8Simple.Int(13) });
				Assert.IsNotNull(callResult);
				Assert.AreEqual(callResult.GetValueType(), V8Simple.Type.Int);
				Assert.AreEqual(
					((V8Simple.Int)callResult).GetValue(),
					25);
				Assert.IsTrue(
					((V8Simple.Object)(context.Evaluate("ObjectTests instanceof", "new Map()")))
					.InstanceOf((V8Simple.Function)context.Evaluate("ObjectTests instanceof 2", "Map")));
				Assert.IsFalse(
					obj
					.InstanceOf((V8Simple.Function)context.Evaluate("ObjectTests instanceof 2", "Map")));
			}
		}
	}

	[Test]
	public void ArrayTests()
	{
		using (var context = new Context(null, null))
		{
			var arr = (V8Simple.Array)context.Evaluate("ArrayTests", "[\"abc\", 123]");
			Assert.AreEqual(arr.GetValueType(), V8Simple.Type.Array);
			Assert.AreEqual(arr.Length(), 2);
			Assert.IsTrue(arr.Equals(arr));
			Assert.IsTrue(arr.StrictEquals(arr));
			Assert.IsFalse(arr.Equals((V8Simple.Array)context.Evaluate("ArrayTests", "[1, 2, 3]")));
			Assert.IsFalse(arr.StrictEquals((V8Simple.Array)context.Evaluate("ArrayTests", "[1, 2, 3]")));
			Assert.AreEqual(((V8Simple.String)arr.Get(0)).GetValue(), "abc");
			Assert.AreEqual(((V8Simple.Int)arr.Get(1)).GetValue(), 123);
			arr.Set(1, new V8Simple.String("123"));
			Assert.AreEqual(((V8Simple.String)arr.Get(1)).GetValue(), "123");
		}
	}

	[Test]
	public void FunctionTests()
	{
		using (var context = new Context(null, null))
		{
			var fun = (V8Simple.Function)context.Evaluate("FunctionTests", "(function(x, y) { return x * y; })");
			Assert.IsNotNull(fun, "Test0");
			Assert.AreEqual(fun.GetValueType(), V8Simple.Type.Function, "Test1");
			var callResult = (V8Simple.Int)fun.Call(new ValueVector { new V8Simple.Int(11), new V8Simple.Int(12) });
			Assert.IsNotNull(callResult, "Test1.5");
			Assert.AreEqual(callResult.GetValue(), 132, "Test2");
			Assert.IsTrue(fun.Equals(fun), "Test3");
			Assert.IsTrue(fun.StrictEquals(fun), "Test3");
			var str = (V8Simple.Function)context.Evaluate("FunctionTests construct", "String");
			Assert.IsNotNull(str, "Test3.5");
			Assert.IsFalse(fun.Equals(str), "Test4");
			Assert.IsFalse(fun.StrictEquals(str), "Test4");
			var obj = str.Construct(new ValueVector { new V8Simple.String("abc 123") });
			Assert.IsNotNull(obj);
			Assert.AreEqual(((V8Simple.Int)obj.CallMethod("indexOf", new ValueVector { new V8Simple.String("1") })).GetValue(), 4, "Test5");
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

	[Test]
	public void CallbackTests()
	{
		using (var context = new Context(null, null))
		{
			var f = (V8Simple.Function)context.Evaluate(
				"CallbackTests",
				"(function(f) { return f(12, 13) + f(10, 20); })");
			Assert.AreEqual(
				((V8Simple.Int)f.Call(new ValueVector { new MyCallback() })).GetValue(),
				12 + 13 + 1000 + 10 + 20 + 1000);
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
			runtimeExceptionHandler))
		{
			handled = false;
			context.Evaluate("ErrorTests", "new ....");
			Assert.IsTrue(handled, "Test1");

			handled = false;
			context.Evaluate("ErrorTests", "obj.someMethod()");
			Assert.IsTrue(handled, "Test2");

			handled = false;
			context.Evaluate("ErrorTests", "throw \"Hello\";");
			Assert.IsTrue(handled, "Test3");

			var obj = (V8Simple.Object)context.Evaluate("ErrorTests", "({})");

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
			context.Evaluate(null, "({})");
			Assert.IsTrue(runtimeHandled, "Test9");

			runtimeHandled = false;
			context.Evaluate("ErrorTests", null);
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
		using (var context = new Context(null, null))
		{
			var messageHandler = new DelegateMessageHandler(x => { return; });
			V8Simple.Context.SetDebugMessageHandler(messageHandler);
			V8Simple.Context.SendDebugCommand("{}");
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

	// Has to be last
	[Test]
	public void ZZZContextTests()
	{
		bool handled;
		var scriptExceptionHandler = new DelegateScriptExceptionHandler(x => { handled = true; });
		var runtimeExceptionHandler = new DelegateMessageHandler(x => { handled = true; });
		var context = new Context(scriptExceptionHandler, runtimeExceptionHandler);
		Assert.AreEqual(
			((V8Simple.Int)context.Evaluate("ContextTests", "1 + 2")).GetValue(),
			3);
	}
}

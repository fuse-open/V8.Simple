using Fuse.Scripting.V8.Simple;
using V8Simple = Fuse.Scripting.V8.Simple;
using System;
using System.Collections.Generic;

static class Test
{
	static void DoStuff()
	{
		using (var context = new Context(new ExceptionHandler()))
		{
			Value val = context.Evaluate("hej.js", "12 + 13");
			Value val2 = context.Evaluate("hej.js", "12.3 + 13.5");
			Value val3 = context.Evaluate("hej.js", "\"hej \" + \"hopp\"");
			Console.WriteLine(val.GetValueType());
			Int intVal = val as V8Simple.Int;
			V8Simple.Double doubleVal = val2 as V8Simple.Double;
			V8Simple.String stringVal = val3 as V8Simple.String;
			Console.WriteLine(intVal == null ? "intVal is null" : intVal.GetValue().ToString());
			Console.WriteLine(doubleVal == null ? "doubleVal is null" : doubleVal.GetValue().ToString());
			Console.WriteLine(stringVal == null ? "stringVal is null" : stringVal.GetValue());
			GC.Collect();
		}
	}

	static void DoStuff2()
	{
		using (var context = new Context(new ExceptionHandler()))
		{
			Function f = context.Evaluate("hej.js", "(function(x, y) { return x + y; })") as Function;
			V8Simple.Array a = context.Evaluate("arr.js", "([2,3,1,4])") as V8Simple.Array;
			V8Simple.Object o = context.Evaluate("obj.js", "({test: \"blah\", test2: \"xyz\"})") as V8Simple.Object;
			Console.WriteLine(f == null ? "f is null" : "f is not null");
			ValueVector args = new ValueVector { new Int(3), new Int(4) };
			int result = (f.Call(args) as Int).GetValue();
			Console.WriteLine(result);
			// GC.Collect();
			// GC.WaitForPendingFinalizers();
			// Console.WriteLine("after collect");
			// f.Dispose();
		}
		GC.Collect();
		GC.WaitForPendingFinalizers();
		Console.WriteLine("after collect 2");
	}

	class ExceptionHandler : ScriptExceptionHandler
	{
		static HashSet<object> _retained = new HashSet<object>();

		public ExceptionHandler() : base() { }

		public override void Handle(ScriptException e)
		{
			throw new Exception(e.GetName() + "\n" +
				e.GetErrorMessage() + "\n" +
				e.GetFileName() + "\n" +
				e.GetLineNumber() + "\n" +
				e.GetStackTrace() + "\n" +
				e.GetSourceLine());
		}

		public override void Retain()
		{
			_retained.Add(this);
		}

		public override void Release()
		{
			_retained.Remove(this);
		}
	}

	class CB : Callback
	{
		static HashSet<object> _retained = new HashSet<object>();

		public CB() : base() { Console.WriteLine("constructing CB"); }

		public override Value Call(ValueVector args)
		{
			Console.WriteLine("Callback with arg types");
			foreach (Value v in args)
			{
				Console.WriteLine(v.GetValueType());
			}
			return new V8Simple.String("callback return value");
		}

		public override void Retain()
		{
			Console.WriteLine("Callback.Retain()");
			_retained.Add(this);
		}

		public override void Release()
		{
			Console.WriteLine("Callback.Release()");
			_retained.Remove(this);
		}
	}

	static void DoStuff3(Context context)
	{
		string result = null;
		{
			Value fresult = null;
			{
				Value vf = context.Evaluate("hej.js", "(function(f) { return f(); })");
				Console.WriteLine("vf");
				Function f = vf as Function;
				Console.WriteLine(f == null ? "f is null" : f.ToString());
				ValueVector args = new ValueVector { new CB() };
				Console.WriteLine("Before call");
				fresult = f.Call(args);
			}
			Console.WriteLine("After call");
			Console.WriteLine(fresult.GetValueType());
			result = (fresult as V8Simple.String).GetValue();
		}
		Console.WriteLine("Collect");
		GC.Collect();
		GC.WaitForPendingFinalizers();
		Console.WriteLine("Collect done");
		// Console.WriteLine(result.AsString().GetValue());
	}

	static void DoStuff4()
	{
		using (var context = new Context(new ExceptionHandler()))
		{
			try
			{
				context.Evaluate("exceptions.js", "\n\na...");
			}
			catch (System.Exception e)
			{
				Console.WriteLine(e.Message);
			}
		}
	}

	static void Main(string[] args)
	{
		// DoStuff();
		DoStuff2();
		Console.WriteLine("before collect 3");
		GC.Collect();
		GC.WaitForPendingFinalizers();
		Console.WriteLine("after collect 3");
		DoStuff4();
		using (var context = new Context(new ExceptionHandler()))
		{
			DoStuff3(context);
			for (int i = 0; i < 100; ++i)
			{
				GC.Collect();
				GC.WaitForPendingFinalizers();
				context.IdleNotificationDeadline(1);
			}
		}
		// GC.Collect();
		// GC.WaitForPendingFinalizers();
	}
}

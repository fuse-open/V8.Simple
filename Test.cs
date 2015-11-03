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
			Int intVal = val.AsInt();
			V8Simple.Double doubleVal = val2.AsDouble();
			V8Simple.String stringVal = val3.AsString();
			Console.WriteLine(intVal == null ? "intVal is null" : intVal.GetValue().ToString());
			Console.WriteLine(doubleVal == null ? "doubleVal is null" : doubleVal.GetValue().ToString());
			Console.WriteLine(stringVal == null ? "stringVal is null" : stringVal.GetValue());
		}
	}

	static void DoStuff2()
	{
		using (var context = new Context(new ExceptionHandler()))
		{
			Function f = context.Evaluate("hej.js", "(function(x, y) { return x + y; })").AsFunction();
			Console.WriteLine(f == null ? "f is null" : f.ToString());
			ValueVector args = new ValueVector { new Int(3), new Int(4) };
			int result = f.Call(args).AsInt().GetValue();
			Console.WriteLine(result);
		}
	}

	class ExceptionHandler : ScriptExceptionHandler
	{
		static HashSet<object> _retained = new HashSet<object>();

		public ExceptionHandler() : base() { }

		public override void Handle(ScriptException e)
		{
			throw new Exception(e.Name);
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

		public override Callback Clone()
		{
			Console.WriteLine("Callback.Clone()");
			return new CB();
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
				Function f = vf.AsFunction();
				Console.WriteLine(f == null ? "f is null" : f.ToString());
				ValueVector args = new ValueVector { new CB() };
				Console.WriteLine("Before call");
				fresult = f.Call(args);
			}
			Console.WriteLine("After call");
			Console.WriteLine(fresult.GetValueType());
			result = fresult.AsString().GetValue();
		}
		GC.Collect();
		GC.WaitForPendingFinalizers();
		// Console.WriteLine(result.AsString().GetValue());
	}

	static void DoStuff4()
	{
		using (var context = new Context(new ExceptionHandler()))
		{
			try
			{
				context.Evaluate("exceptions.js", "a...");
			}
			catch (System.Exception e)
			{
				Console.WriteLine(e.Message);
			}
		}
	}

	static void Main(string[] args)
	{
		DoStuff();
		DoStuff2();
		DoStuff4();
		using (var context = new Context(new ExceptionHandler()))
		{
			DoStuff3(context);
			while (true)
			{
				GC.Collect();
				GC.WaitForPendingFinalizers();
				context.IdleNotificationDeadline(1);
			}
		}
		GC.Collect();
		GC.WaitForPendingFinalizers();
	}
}

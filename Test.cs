using Fuse.Scripting.V8.Simple;
using V8Simple = Fuse.Scripting.V8.Simple;
using System;
using System.Collections.Generic;

static class Test
{
	static void DoStuff()
	{
		using (var context = new Context())
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
		using (var context = new Context())
		{
			Function f = context.Evaluate("hej.js", "(function(x, y) { return x + y; })").AsFunction();
			Console.WriteLine(f == null ? "f is null" : f.ToString());
			ValueVector args = new ValueVector { new Int(3), new Int(4) };
			int result = f.Call(args).AsInt().GetValue();
			Console.WriteLine(result);
		}
	}

	class CB : Callback
	{
		// static List<CB> _cbs = new List<CB>();

		public CB()
		{
			//_cbs.Add(this);
		}

		public override Value Call(ValueVector args)
		{
			Console.WriteLine("Callback with arg types");
			foreach (Value v in args)
			{
				Console.WriteLine(v.GetValueType());
			}
			return new V8Simple.String("callback return value");
		}

		public override Callback Copy()
		{
			return new CB();
		}
	}

	static void DoStuff3()
	{
		using (var context = new Context())
		{
			Function f = context.Evaluate("hej.js", "(function(f) { f(1, \"abc\", {}, [1, 2, 3]); })").AsFunction();
			Console.WriteLine(f == null ? "f is null" : f.ToString());
			ValueVector args = new ValueVector { new CB() };
			string result = f.Call(args).AsString().GetValue();
			Console.WriteLine(result);
		}
	}

	static void Main(string[] args)
	{
		DoStuff();
		DoStuff2();
		DoStuff3();
		GC.Collect();
		GC.WaitForPendingFinalizers();
	}
}

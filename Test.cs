using Fuse.Scripting.V8.Simple;
using V8Simple = Fuse.Scripting.V8.Simple;
using System;

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

	static void DoMoreStuff()
	{
		try
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
		catch (System.Exception e)
		{
			Console.WriteLine("EXCEPTION! " + e.Message);
		}
	}

	static void Main(string[] args)
	{
		DoMoreStuff();
		DoStuff();
		GC.Collect();
		GC.WaitForPendingFinalizers();
	}
}

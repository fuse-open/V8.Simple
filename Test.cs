using Fuse.Scripting.V8.Simple;
using System;

static class Test
{
	static void DoStuff()
	{
		Context context = new Context();

		Value val = context.Evaluate("hej.js", "12 + 13");
		Value val2 = context.Evaluate("hej.js", "12.3 + 13.5");
		Console.WriteLine(val.GetValueType());
		Int intVal = val.AsInt();
		Fuse.Scripting.V8.Simple.Double doubleVal = val2.AsDouble();
		Console.WriteLine(intVal == null ? "intVal is null" : intVal.GetValue().ToString());
		Console.WriteLine(doubleVal == null ? "doubleVal is null" : doubleVal.GetValue().ToString());
		context.Dispose();
	}

	static void Main(string[] args)
	{
		DoStuff();
		GC.Collect();
		GC.WaitForPendingFinalizers();
	}
}

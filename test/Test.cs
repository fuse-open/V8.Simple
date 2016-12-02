using Fuse.Scripting.V8.Simple;
using NUnit.Framework;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System;

[TestFixture]
public class V8SimpleTests
{
	static void CheckError(JSRuntimeError err)
	{
		if (err != JSRuntimeError.NoError)
			throw new Exception("V8.Simple runtime error: " + err.ToString());
	}

	static void CheckError(JSContext context, JSScriptException err)
	{
		if (err != default(JSScriptException))
		{
			try
			{
				throw new Exception("V8.Simple runtime error: " + ScriptException.GetMessage(err));
			}
			finally
			{
				ScriptException.Release(context, err);
			}
		}
	}

	static JSString AsJSString(JSContext context, string str)
	{
		JSRuntimeError err;
		var result = Value.CreateString(context, str, str.Length, out err);
		CheckError(err);
		return result;
	}

	static string AsString(JSContext context, JSValue val)
	{
		Assert.AreEqual(JSType.String,Value.GetType(val));
		JSRuntimeError err;
		var jsStr = Value.AsString(val, out err);
		CheckError(err);
		var builder = new StringBuilder(Value.Length(context, jsStr));
		Value.Write(context, jsStr, builder);
		return builder.ToString();
	}

	static int AsInt(JSValue val)
	{
		Assert.AreEqual(JSType.Int, Value.GetType(val));
		JSRuntimeError err;
		var result = Value.AsInt(val, out err);
		CheckError(err);
		return result;
	}

	static double AsDouble(JSValue val)
	{
		Assert.AreEqual(JSType.Double, Value.GetType(val));
		JSRuntimeError err;
		var result = Value.AsDouble(val, out err);
		CheckError(err);
		return result;
	}

	static bool AsBool(JSValue val)
	{
		Assert.AreEqual(JSType.Bool, Value.GetType(val));
		JSRuntimeError err;
		var result = Value.AsBool(val, out err);
		CheckError(err);
		return result;
	}

	static JSObject AsObject(JSValue val)
	{
		Assert.IsTrue(JSType.Object == Value.GetType(val) || JSType.Null == Value.GetType(val));
		JSRuntimeError err;
		var result = Value.AsObject(val, out err);
		CheckError(err);
		return result;
	}

	static JSFunction AsFunction(JSValue val)
	{
		Assert.AreEqual(JSType.Function, Value.GetType(val));
		JSRuntimeError err;
		var result = Value.AsFunction(val, out err);
		CheckError(err);
		return result;
	}

	static JSArray AsArray(JSValue val)
	{
		Assert.AreEqual(JSType.Array, Value.GetType(val));
		JSRuntimeError err;
		var result = Value.AsArray(val, out err);
		CheckError(err);
		return result;
	}

	static JSExternal AsExternal(JSValue val)
	{
		Assert.AreEqual(JSType.External, Value.GetType(val));
		JSRuntimeError err;
		var result = Value.AsExternal(val, out err);
		CheckError(err);
		return result;
	}

	static JSValue Eval(JSContext context, string name, string code)
	{
		JSScriptException err;
		var result = Eval(context, name, code, out err);
		CheckError(context, err);
		return result;
	}

	static JSValue Eval(JSContext context, string name, string code, out JSScriptException err)
	{
		var jsName = AsJSString(context, name);
		var jsCode = AsJSString(context, code);
		var result = Context.EvaluateCreate(context, jsName, jsCode, out err);
		Value.Release(context, Value.AsValue(jsCode));
		Value.Release(context, Value.AsValue(jsName));
		return result;
	}

	[Test]
	public void Primitives()
	{
		var context = Context.Create(null, null);
		var testName = "Primitives";
		{
			var result = Eval(context, testName, "12 + 13");
			Assert.AreEqual(25, AsInt(result));
			Value.Release(context, result);
		}
		{
			var result = Eval(context, testName, "1.2 + 1.3");
			Assert.AreEqual(2.5, AsDouble(result));
			Value.Release(context, result);
		}
		{
			var result = Eval(context, testName, "\"abc 123\"");
			Assert.AreEqual("abc 123", AsString(context, result));
			Value.Release(context, result);
		}
		{
			var result = Eval(context, testName, "true || false");
			Assert.AreEqual(true, AsBool(result));
			Value.Release(context, result);
		}
		Context.Release(context);
	}

	[Test]
	public void Objects()
	{
		var context = Context.Create(null, null);
		var testName = "Objects";

		var obj = AsObject(Eval(context, testName, "({ a: \"abc\", b: 123 })"));
		var a = AsJSString(context, "a");
		var b = AsJSString(context, "b");
		var c = AsJSString(context, "c");
		var d = AsJSString(context, "d");
		var xyz = AsJSString(context, "xyz");
		var n = Value.CreateDouble(123.4);
		JSScriptException err;
		{
			var aresult = Value.CopyProperty(context, obj, a, out err);
			CheckError(context, err);
			Assert.AreEqual(AsString(context, aresult), "abc");
			Value.Release(context, aresult);
		}
		{
			var bresult = Value.CopyProperty(context, obj, b, out err);
			CheckError(context, err);
			Assert.AreEqual(AsInt(bresult), 123);
			Value.Release(context, bresult);
		}
		{
			Value.SetProperty(context, obj, a, Value.AsValue(xyz), out err);
			CheckError(context, err);

			var aresult = Value.CopyProperty(context, obj, a, out err);
			CheckError(context, err);
			Assert.AreEqual(AsString(context, aresult), "xyz");
			Value.Release(context, aresult);
		}
		{
			Value.SetProperty(context, obj, c, n, out err);
			CheckError(context, err);

			var cresult = Value.CopyProperty(context, obj, c, out err);
			CheckError(context, err);
			Assert.AreEqual(AsDouble(cresult), 123.4);
			Value.Release(context, cresult);
		}
		{
			Assert.IsTrue(Value.HasProperty(context, obj, a, out err));
			CheckError(context, err);
			Assert.IsTrue(Value.HasProperty(context, obj, b, out err));
			CheckError(context, err);
			Assert.IsTrue(Value.HasProperty(context, obj, c, out err));
			CheckError(context, err);
			Assert.IsFalse(Value.HasProperty(context, obj, d, out err));
			CheckError(context, err);
		}
		{
			var arr = Value.CopyOwnPropertyNames(context, obj, out err);
			CheckError(context, err);
			var properties = new HashSet<string>();
			var len = Value.Length(context, arr);
			for (int i = 0; i < len; ++i)
			{
				var prop = Value.CopyProperty(context, arr, i, out err);
				CheckError(context, err);
				properties.Add(AsString(context, prop));
				Value.Release(context, prop);
			}
			Assert.IsTrue(properties.Contains("a"));
			Assert.IsTrue(properties.Contains("b"));
			Assert.IsTrue(properties.Contains("c"));
			Assert.IsFalse(properties.Contains("d"));
		}
		{
			Assert.IsTrue(Value.StrictEquals(context, Value.AsValue(obj), Value.AsValue(obj)));
			var obj2 = AsObject(Eval(context, testName, "({ a: \"abc\", b: 123 })"));
			Assert.IsFalse(Value.StrictEquals(context, Value.AsValue(obj), Value.AsValue(obj2)));
			Value.Release(context, Value.AsValue(obj2));
		}


		Value.Release(context, n);
		Value.Release(context, Value.AsValue(xyz));
		Value.Release(context, Value.AsValue(d));
		Value.Release(context, Value.AsValue(c));
		Value.Release(context, Value.AsValue(b));
		Value.Release(context, Value.AsValue(a));
		Value.Release(context, Value.AsValue(obj));
		Context.Release(context);
	}


	[Test]
	public void Arrays()
	{
		var context = Context.Create(null, null);
		var arr = AsArray(Eval(context, "Arrays", "[\"abc\", 123]"));
		Assert.AreEqual(2, Value.Length(context, arr));
		JSScriptException err;
		var a = Value.CopyProperty(context, arr, 0, out err);
		CheckError(context, err);
		var b = Value.CopyProperty(context, arr, 1, out err);
		CheckError(context, err);

		Assert.AreEqual("abc", AsString(context, a));
		Assert.AreEqual(123, AsInt(b));

		Value.Release(context, b);
		Value.Release(context, a);
		Value.Release(context, Value.AsValue(arr));
		Context.Release(context);
	}

	[Test]
	public void Functions()
	{
		var context = Context.Create(null, null);
		JSScriptException err;
		{
			var fun = AsFunction(Eval(context, "Functions", "(function(x, y) { return x * y; })"));
			var args = new JSValue[] { Value.CreateInt(11), Value.CreateInt(12) };
			var callResult = Value.CallCreate(context, fun, default(JSObject), args, args.Length, out err);
			CheckError(context, err);
			Assert.AreEqual(11 * 12, AsInt(callResult));
			foreach (var arg in args)
				Value.Release(context, arg);

			Value.Release(context, callResult);
			Value.Release(context, Value.AsValue(fun));
		}
		{
			var str = AsFunction(Eval(context, "Functions", "String"));
			var args = new JSValue[] { Value.AsValue(AsJSString(context, "abc 123")) };
			var obj = Value.ConstructCreate(context, str, args, args.Length, out err);
			foreach (var arg in args)
				Value.Release(context, arg);
			CheckError(context, err);

			var indexOfString = AsJSString(context, "indexOf");
			var indexOf = AsFunction(Value.CopyProperty(context, obj, indexOfString, out err));
			CheckError(context, err);

			var args2 = new JSValue[] { Value.AsValue(AsJSString(context, "1")) };
			var index1 = Value.CallCreate(context, indexOf, obj, args2, args2.Length, out err);
			CheckError(context, err);
			Assert.AreEqual(4, AsInt(index1));
			Value.Release(context, index1);

			foreach (var arg in args2)
				Value.Release(context, arg);

			Value.Release(context, Value.AsValue(indexOf));
			Value.Release(context, Value.AsValue(indexOfString));
			Value.Release(context, Value.AsValue(obj));
			Value.Release(context, Value.AsValue(str));
		}
		Context.Release(context);
	}

	static void FinalizeExternal(IntPtr external)
	{
		GCHandle.FromIntPtr(external).Free();
	}

	readonly JSExternalFinalizer _externalFinalizer = FinalizeExternal;

	static JSExternal CreateExternal(JSContext context, object o)
	{
		return Value.CreateExternal(context, GCHandle.ToIntPtr(GCHandle.Alloc(o)));
	}

	static void FinalizeCallback(IntPtr data)
	{
		GCHandle.FromIntPtr(data).Free();
	}

	readonly JSCallbackFinalizer _callbackFinalizer = FinalizeCallback;

	static JSValue CallCallback(JSContext context, IntPtr data, JSValue[] args, int numArgs, out JSValue error)
	{
		error = default(JSValue);
		var fun = GCHandle.FromIntPtr(data).Target as Func<JSContext, JSValue[], JSValue>;
		try
		{
			return fun(context, args);
		}
		catch (Exception e)
		{
			error = Value.AsValue(AsJSString(context, e.Message));
		}
		return default(JSValue);
	}

	readonly JSCallback _callCallback = CallCallback;

	JSFunction CreateCallback(JSContext context, Func<JSContext, JSValue[], JSValue> cb)
	{
		JSScriptException err;
		var result = Value.CreateCallback(context, GCHandle.ToIntPtr(GCHandle.Alloc(cb)), _callCallback, out err);
		CheckError(context, err);
		return result;
	}

	[Test]
	public void Callbacks()
	{
		JSScriptException err;
		var testName = "Callbacks";
		var context = Context.Create(_callbackFinalizer, _externalFinalizer);

		var f = AsFunction(Eval(context, testName, "(function(f) { return f(12, 13) + f(10, 20); })"));

		var cb = CreateCallback(context, (cxt, args) =>
		{
			var x = AsInt(args[0]);
			var y = AsInt(args[1]);
			return Value.CreateInt(x + y + 1000);
		});

		System.GC.Collect();
		System.GC.WaitForPendingFinalizers();

		var result = Value.CallCreate(context, f, default(JSObject), new JSValue[] { Value.AsValue(cb) }, 1, out err);
		CheckError(context, err);

		System.GC.Collect();
		System.GC.WaitForPendingFinalizers();

		Assert.AreEqual(12 + 13 + 1000 + 10 + 20 + 1000, AsInt(result));

		Value.Release(context, result);
		Value.Release(context, Value.AsValue(cb));
		Value.Release(context, Value.AsValue(f));
		System.GC.Collect();
		System.GC.WaitForPendingFinalizers();
		Context.Release(context);
	}

	[Test]
	public void CallbackExceptions()
	{
		var testName = "CallbackExceptions";
		var context = Context.Create(_callbackFinalizer, _externalFinalizer);

		var cb = CreateCallback(context, (cxt, args) =>
		{
			throw new Exception(testName);
		});

		JSScriptException err;
		var res = Value.CallCreate(context, cb, default(JSObject), null, 0, out err);
		Assert.AreEqual(default(JSValue), res);
		Assert.AreNotEqual(default(JSScriptException), err);
		var exceptionString = AsString(context, ScriptException.GetException(err));

		Assert.AreEqual(testName, exceptionString);

		ScriptException.Release(context, err);
		Context.Release(context);
	}

	[Test]
	public void Errors()
	{
		var context = Context.Create(_callbackFinalizer, _externalFinalizer);
		var testName = "Errors";

		var strs = new string[] { "new ....", "obj.someMethod()", "throw \"Hello\"" };

		foreach (var str in strs)
		{
			JSScriptException err;
			var res = Eval(context, testName, str, out err);
			Assert.AreNotEqual(default(JSScriptException), err);
			ScriptException.Release(context, err);
			Assert.AreEqual(default(JSValue), res);
		}

		{
			var throwingFun = AsFunction(Eval(context, testName, "(function() { throw \"Error\"; })"));
			JSScriptException err;
			Value.CallCreate(context, throwingFun, default(JSObject), null, 0, out err);
			Assert.AreNotEqual(default(JSScriptException), err);
			ScriptException.Release(context, err);
		}

		Context.Release(context);
	}

	[Test]
	public void Debugger()
	{
		var context = Context.Create(null, null);
		Debug.SetMessageHandler(context, IntPtr.Zero, null);
		Debug.ProcessMessages(context);

		Debug.SetMessageHandler(context, IntPtr.Zero, (data, message) => { return; });
		Debug.SendCommand(context, "{}", 2);
		Debug.ProcessMessages(context);

		Context.Release(context);
	}

	[Test]
	public void Version()
	{
		Assert.IsNotNull(Context.GetV8Version());
	}

	[Test]
	public void Unicode()
	{
		var testName = "Unicode";

		var context = Context.Create(null, null);

		foreach (var str in _unicodeStrings)
		{
			var res = Eval(context, testName, "\"" + str + "\"");
			var str2 = AsString(context, res);
			Assert.AreEqual(str, str2);
			Value.Release(context, res);
		}

		var id = AsFunction(Eval(context, testName, "(function(x) { return x; })"));

		foreach (var str in _unicodeStrings)
		{
			var strVal = AsJSString(context, str);
			JSScriptException err;
			var res = Value.CallCreate(
				context,
				id,
				default(JSObject),
				new JSValue[] { Value.AsValue(strVal) },
				1,
				out err);
			CheckError(context, err);

			Assert.AreEqual(str, AsString(context, res));

			Value.Release(context, res);
			Value.Release(context, Value.AsValue(strVal));
		}

		Value.Release(context, Value.AsValue(id));

		Context.Release(context);
	}

	readonly string[] _unicodeStrings = new string[]
	{
		"",
		"abc",
		"The quick brown fox jumps over the lazy dog",
		"ç, é, õ",
		"åååååååååååååææææææææøøøøøøøøøøøøø ç, é, õ aaaaaaaaaaaabbbbbbbbbbc ccccccc",
		"eeeeææææææææææaaaaaaaaa",
		"صِفْ Amiri3 صِفْ خَلْقَ Amiri2 صِفْ خَلْقَ خَوْدٍ Amiri1 صِفْ خَلْقَ خَوْدٍ صِفْ",
		"𐐷𐐷𐐷𐐷",
		"𐐷𐐷𐐷𐐷abc𤭢𤭢𤭢𤭢a𐐷𐐷𐐷𐐷abc𤭢𤭢𤭢𤭢a𐐷𐐷𐐷𐐷abc𤭢𤭢𤭢𤭢a𐐷𐐷𐐷𐐷abc𤭢𤭢𤭢𤭢a",
		"Emoji 😃  are such fun!",
		"देवनागरीदेवनागरीदेवनागरीदेवनागरीदेवनागरीदेवनागरीदेवनागरीदेवनागरीदेवनागरी",
		" א ב ג ד ה ו ז ח ט י  כ ך ל מ ם נ ן ס ע פ  ף צ ץ ק ר ש ת  •  ﭏ",
		"Testing «ταБЬℓσ»: 1<2 & 4+1>3, now 20% off!",
		"٩(-̮̮̃-̃)۶ ٩(●̮̮̃•̃)۶ ٩(͡๏̯͡๏)۶ ٩(-̮̮̃•̃).",
		"Quizdeltagerne spiste jordbær med fløde, mens cirkusklovnen Wolther spillede på xylofon.",
		"Falsches Üben von Xylophonmusik quält jeden größeren Zwerg",
		"Γαζέες καὶ μυρτιὲς δὲν θὰ βρῶ πιὰ στὸ χρυσαφὶ ξέφωτο",
		"Ξεσκεπάζω τὴν ψυχοφθόρα βδελυγμία",
		"El pingüino Wenceslao hizo kilómetros bajo exhaustiva lluvia y frío, añoraba a su querido cachorro.",
		"Le cœur déçu mais l'âme plutôt naïve, Louÿs rêva de crapaüter en canoë au delà des îles, près du mälström où brûlent les novæ.",
		"D'fhuascail Íosa, Úrmhac na hÓighe Beannaithe, pór Éava agus Ádhaimh",
		"Árvíztűrő tükörfúrógép",
		"Kæmi ný öxi hér ykist þjófum nú bæði víl og ádrepa",
		"Sævör grét áðan því úlpan var ónýt",
		"いろはにほへとちりぬるを  わかよたれそつねならむ  うゐのおくやまけふこえて  あさきゆめみしゑひもせす",
		"イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム  ウヰノオクヤマ ケフコエテ アサキユメミシ ヱヒモセスン",
		"? דג סקרן שט בים מאוכזב ולפתע מצא לו חברה איך הקליטה",
		"Pchnąć w tę łódź jeża lub ośm skrzyń fig",
		"В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!",
		"Съешь же ещё этих мягких французских булок да выпей чаю",
		"๏ เป็นมนุษย์สุดประเสริฐเลิศคุณค่า  กว่าบรรดาฝูงสัตว์เดรัจฉาน",
		"Pijamalı hasta, yağız şoföre çabucak güvendi.",
	};


	class SomeObject
	{
		public string SomeField;
		public SomeObject(string someField)
		{
			SomeField = someField;
		}
	}

	[Test]
	public void External()
	{
		var testName = "External";

		SomeObject someObject = new SomeObject("theField");

		var context = Context.Create(_callbackFinalizer, _externalFinalizer);

		var ext = CreateExternal(context, someObject);

		{
			var val = GCHandle.FromIntPtr(Value.GetExternalValue(context, ext)).Target as SomeObject;
			Assert.AreEqual(someObject, val);
		}
		{
			var id = AsFunction(Eval(context, testName, "(function(x) { return x; })"));

			JSScriptException err;
			var ext2 = Value.CallCreate(
				context,
				id,
				default(JSObject),
				new JSValue[] { Value.AsValue(ext) },
				1,
				out err);
			CheckError(context, err);

			var val = GCHandle.FromIntPtr(Value.GetExternalValue(context, AsExternal(ext2))).Target as SomeObject;
			Assert.AreEqual(someObject, val);

			Value.Release(context, ext2);
			Value.Release(context, Value.AsValue(id));
		}

		Value.Release(context, Value.AsValue(ext));
		Context.Release(context);
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
	public void ArrayBuffers()
	{
		var testName = "ArrayBuffers";

		var context = Context.Create(null, null);

		var len = 100;
		var buf = new byte[len];
		var sum = 0;
		for (byte i = 0; i < len; ++i)
		{
			buf[i] = i;
			sum += i;
		}

		{
			var marshaller = new ArrayMarshaller(buf);

			var arrayBuffer = Value.CreateExternalArrayBuffer(context, marshaller.GetIntPtr(), len);

			JSRuntimeError err;
			Assert.AreEqual(marshaller.GetIntPtr(), Value.GetArrayBufferData(context, arrayBuffer, out err));
			CheckError(err);

			Value.Release(context, Value.AsValue(arrayBuffer));
		}

		{
			var f = AsFunction(Eval(context, testName, "(function (len) { var buf = new ArrayBuffer(len); var x = new Uint8Array(buf); for (var i = 0; i < len; ++i) x[i] = i; return buf; })"));
			var lenValue = Value.CreateInt(len);

			JSScriptException err;
			var arrayBuffer = AsObject(Value.CallCreate(context, f, AsObject(Value.JSNull()), new JSValue[] { lenValue }, 1, out err));
			CheckError(context, err);

			JSRuntimeError rerr;
			var ptr = Value.GetArrayBufferData(context, arrayBuffer, out rerr);
			CheckError(rerr);
			var buf2 = new byte[len];
			Marshal.Copy(ptr, buf2, 0, len);
			for (int i = 0; i < len; ++i)
			{
				Assert.AreEqual(buf[i], buf2[i]);
			}

			Value.Release(context, lenValue);
			Value.Release(context, Value.AsValue(arrayBuffer));
			Value.Release(context, Value.AsValue(f));
		}

		Context.Release(context);
	}
}

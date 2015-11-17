%module(directors="1") v8
%{
#include "V8Simple.h"
%}
%include <std_vector.i>
%newobject V8Simple::Context::Evaluate(const char*, const char*);
%newobject V8Simple::Context::GlobalObject();
%newobject V8Simple::Object::Get(const char*);
%newobject V8Simple::Object::CallMethod(const char*, const std::vector<Value*>&);
%newobject V8Simple::Function::Call(const std::vector<Value*>&);
%newobject V8Simple::Function::Construct(const std::vector<Value*>&);
%newobject V8Simple::Array::Get(int);
%newobject V8Simple::Callback::Call(const std::vector<Value*>&);
%newobject V8Simple::Callback::Clone() const;
%feature("director") V8Simple::MessageHandler;
%feature("director") V8Simple::ScriptExceptionHandler;
%feature("director") V8Simple::Callback;
%typemap(throws, canthrow=1) std::runtime_error {
  SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, $1.what());
  return $null;
}
%include "V8Simple.h"
%template(Int) V8Simple::Primitive<int>;
%template(Double) V8Simple::Primitive<double>;
%template(String) V8Simple::Primitive<const char*>;
%template(Bool) V8Simple::Primitive<bool>;
%template(StringVector) std::vector<const char*>;
%template(ValueVector) std::vector<V8Simple::Value*>;
%template(AsInt) V8Simple::Value::As<V8Simple::Int>;
%template(AsDouble) V8Simple::Value::As<V8Simple::Double>;
%template(AsString) V8Simple::Value::As<V8Simple::String>;
%template(AsBool) V8Simple::Value::As<V8Simple::Bool>;
%template(AsObject) V8Simple::Value::As<V8Simple::Object>;
%template(AsFunction) V8Simple::Value::As<V8Simple::Function>;
%template(AsArray) V8Simple::Value::As<V8Simple::Array>;

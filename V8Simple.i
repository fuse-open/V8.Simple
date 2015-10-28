%module v8
%{
#include "V8Simple.h"
%}
%include <std_string.i>
%include <std_vector.i>
%newobject V8Simple::Context::Evaluate(const std::string&, const std::string&);
%newobject V8Simple::Context::GlobalObject();
%newobject V8Simple::Object::Get(const std::string&);
%newobject V8Simple::Object::CallMethod(const std::string&, const std::vector<Value*>&);
%newobject V8Simple::Function::Call(const std::vector<Value*>&);
%newobject V8Simple::Function::Construct(const std::vector<Value*>&);
%newobject V8Simple::Array::Get(int);
%newobject V8Simple::Callback::Call(const std::vector<Value*>&);
%newobject V8Simple::Callback::Copy() const;
%include "V8Simple.h"
%template(Int) V8Simple::Primitive<int, V8Simple::Type::Int>;
%template(Double) V8Simple::Primitive<double, V8Simple::Type::Double>;
%template(String) V8Simple::Primitive<std::string, V8Simple::Type::String>;
%template(Bool) V8Simple::Primitive<bool, V8Simple::Type::Bool>;
%template(StringVector) std::vector<std::string>;
%template(ValueVector) std::vector<V8Simple::Value*>;

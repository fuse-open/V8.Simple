%module v8
%{
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
%template(Int) V8Simple::Primitive<int, Type::Int>;
%template(Double) V8Simple::Primitive<double, Type::Double>;
%template(String) V8Simple::Primitive<std::string, Type::String>;
%template(Bool) V8Simple::Primitive<bool, Type::Bool>;
%template(StringVector) std::vector<std::string>;
%template(ValueVector) std::vector<V8Simple::Value*>;

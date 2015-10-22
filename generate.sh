#!/bin/sh
swig -csharp -dllimport V8Simple -namespace Fuse.Scripting.V8.Simple -c++ -outfile V8Simple.cs V8Simple.i

call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
FOR %%G in (a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z) DO (call lib /machine:x64 /verbose /out:v8\out.gn\x64.release\obj\v8_base_%%G.lib v8\out.gn\x64.release\obj\v8_base\%%G*.obj)
call lib /machine:x64 /verbose /out:v8\out.gn\x64.release\obj\v8_libbase.lib v8\out.gn\x64.release\obj\v8_libbase\*.obj
call lib /machine:x64 /verbose /out:v8\out.gn\x64.release\obj\v8_libplatform.lib v8\out.gn\x64.release\obj\v8_libplatform\*.obj
call lib /machine:x64 /verbose /out:v8\out.gn\x64.release\obj\v8_libsampler.lib v8\out.gn\x64.release\obj\v8_libsampler\*.obj
call lib /machine:x64 /verbose /out:v8\out.gn\x64.release\obj\v8_nosnapshot.lib v8\out.gn\x64.release\obj\v8_nosnapshot\*.obj
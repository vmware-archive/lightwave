REM "C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin\
REM "midl.exe" /W1 /WX /nologo /char signed /env x64 /out"Y:\aengineer\workspaces\lotus\main\vmca\idl\win" /h "vmca_h.h" /tlb "x64\Debug\Idl.tlb" /client stub /server stub ..\vmca.idl 
"midl.exe" /W1 /WX /nologo /char signed /env x64 /h "vmca_h.h" /tlb "x64\Debug\Idl.tlb" /client stub /server stub /cstub vmca_cstub.c /sstub vmca_sstub.c ..\vmca.idl 

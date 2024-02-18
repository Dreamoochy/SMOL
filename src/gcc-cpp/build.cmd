g++.exe -Os -O -c smol.cpp -o obj\Release\smol.o
g++.exe -o bin\Release\smol.exe obj\Release\smol.o -static-libgcc -l:libkernel32.a -l:libwtsapi32.a -l:libuser32.a -mwindows


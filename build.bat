@echo off

set CommonCompilerOptions=-nologo -Od -Zi -FC -TP -EHsc -F4294967296
set CommonLinkerOptions=user32.lib kernel32.lib

if not exist ..\..\build ( mkdir ..\..\build )
pushd ..\..\build

cl %CommonCompilerOptions% ..\arenas\code\main.cpp /link %CommonLinkerOptions% /out:test_arena.exe

popd
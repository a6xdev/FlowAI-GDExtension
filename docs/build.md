
First of all, set your compilePath in ```CppProperties.json```:
```cpp
// Example:
"compilerPath": "C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/18.xx.xxxxx/bin/Hostx64/x64/cl.exe",
```

## Execute
python -m SCons platform=windows target=template_debug -j6
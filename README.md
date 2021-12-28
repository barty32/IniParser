# IniParser++
A simple C++ initialization file parser library.

# Features
- Minimalistic header-only C++ implementation
- Cross-platform
- UTF-8 support
- Preserves comments
- Compatible with Windows API functions (ReadPrivateProfile*, WritePrivateProfile*)
- ...

# Function reference:
This section is unfinished, functions are documented directly in the code.

Public:
```
  ChangePath(newPath)
  Init()
  Reload()
  Flush()
  ReadString     (sectionName, keyName, Default)
  ReadInt        (sectionName, keyName, Default)
  ReadBinary     (sectionName, keyName, binaryDataBuffer, binaryDataSize, verifyChecksum = true)
  WriteString    (sectionName, keyName, value)
  WriteInt       (sectionName, keyName, value)
  WriteBinary    (sectionName, keyName, binaryData, dataSize)
  DeleteKey      (sectionName, keyName)
  DeleteSection  (sectionName)
  SetComment     (sectionName, keyName, value)
  GetKeyCount    (sectionName)
  GetSectionCount()
  ```
Protected:
```
  trim(str, trimQuot = false)
  _parseIni(clear = false)
  _writeIni()
  ```
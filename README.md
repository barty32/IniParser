# IniParser++

# Features
- Minimalistic header-only C++ implementation
- Cross-platform
- Easy usage
- UTF-8 support
- Preserves comments
- Compatible with Windows API functions (ReadPrivateProfile*, WritePrivateProfile*)
- ...

# Function reference:
This section is unfinished, functions are documented directly in the code.

Public:
```
  ChangePath
  Init
  Reload
  Flush
  ReadString
  ReadInt
  ReadBinary
  WriteString
  WriteInt
  WriteBinary
  DeleteKey
  DeleteSection
  SetComment
  GetKeyCount
  GetSectionCount
  ```
Protected:
```
  trim
  _parseIni
  _writeIni
  ```
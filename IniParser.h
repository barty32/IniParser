// _____      _______                                    
//|_   _|    (_) ___ \                         _     _   
//  | | _ __  _| |_/ /_ _ _ __ ___  ___ _ __ _| |_ _| |_ 
//  | || '_ \| |  __/ _` | '__/ __|/ _ \ '__|_   _|_   _|
// _| || | | | | | | (_| | |  \__ \  __/ |    |_|   |_|  
// \___/_| |_|_\_|  \__,_|_|  |___/\___|_|               
//
// Simple and minimalistic C++ initialization file parser library
// Version 2.0
//
//
// MIT License
//
// Copyright (c) 2021-2022 barty12
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
// Function reference:
// Public:
//     ChangePath(newPath)
//     Init()
//     Reload()
//     Flush()
//     ReadString     (sectionName, keyName, Default)
//     ReadInt        (sectionName, keyName, Default)
//     ReadBinary     (sectionName, keyName, binaryDataBuffer, binaryDataSize, verifyChecksum = true)
//     WriteString    (sectionName, keyName, value)
//     WriteInt       (sectionName, keyName, value)
//     WriteBinary    (sectionName, keyName, binaryData, dataSize)
//     DeleteKey      (sectionName, keyName)
//     DeleteSection  (sectionName)
//     SetComment     (sectionName, keyName, value)
//     GetKeyCount    (sectionName)
//     GetSectionCount()
//  
// Protected:
//     trim(str, trimQuot = false)
//     _parseIni(clear = false)
//     _writeIni()
//
// 
// 

#ifndef _INC_INIPARSER
#define _INC_INIPARSER

#ifdef __cplusplus

//Includes
#include <string>
//#include <codecvt>
#include "utf8_facet.h" //Custom modified version of <codecvt> (does not stop reading and writing on errors)
#include <unordered_map>
#include <list>
#include <exception>
#include <fstream>
#include <sstream>
#include <cwctype>

#define CharToNibble(x) ((x)>='0'&&(x)<='9' ? (x)-'0' : ((10+(x)-'A')&0x000f))


using _Elem = wchar_t;
//template <class _Elem = wchar_t>
class IniParser{

//-----------------------------------------------------------------------------------------------------------
//                              Structures
//-----------------------------------------------------------------------------------------------------------
	
	struct _key{
		std::basic_string<_Elem> _value;
		std::basic_string<_Elem> _comments;
		bool _delete = false;
		bool _registered = false;
	};

	struct _section{
		std::unordered_map<std::basic_string<_Elem>, _key> _keys;
		std::list<std::basic_string<_Elem>> _insertOrder;
		std::basic_string<_Elem> _comments;
		bool _delete = false;
		bool _registered = false;
	};

//-----------------------------------------------------------------------------------------------------------
//                              Member variables
//-----------------------------------------------------------------------------------------------------------
	private:
	std::basic_string<_Elem> m_iniPath = L"settings.ini";
	
	protected:
	std::unordered_map<std::basic_string<_Elem>, _section> m_sections;
	std::list<std::basic_string<_Elem>> m_insertOrder;
	std::basic_string<_Elem> m_comments;
	std::locale _utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<_Elem, 0x10ffff, std::consume_header>);

//-----------------------------------------------------------------------------------------------------------
//                              Constructors and destructors
//-----------------------------------------------------------------------------------------------------------
	public:
	IniParser(){}
	IniParser(const std::basic_string<_Elem> &IniPath) :m_iniPath(IniPath){}
	~IniParser(){
		this->Flush();
	}

//-----------------------------------------------------------------------------------------------------------
//                              Member functions
//-----------------------------------------------------------------------------------------------------------
	bool ChangePath(std::basic_string<_Elem> const& newPath){
		m_iniPath = newPath;
	}

	protected:
	//Trims any whitespace from beginning and end of a string
	static std::basic_string<_Elem> trim(const std::basic_string<_Elem>& s, bool trimQuot = false) noexcept{
		try{
			if(!s.size()){
				return L"";
			}
			auto start = s.begin();
			while(start != s.end() && std::iswspace(*start)){
				start++;
			}
			auto end = s.end();
			do{
				end--;
			} while(std::distance(start, end) > 0 && std::iswspace(*end));
			if(trimQuot && *start == L'"' && *end == L'"'){
				start++;
				end--;
			}
			return std::basic_string<_Elem>(start, end + 1);
		} catch(...){
			return s;
		}
	}

	bool _parseIni(bool clear = false){
		std::wstring line;
		std::wstring origLine;
		std::wifstream iniFile;
		iniFile.imbue(_utf8_locale);
		iniFile.open(m_iniPath);
		if(clear){
			m_sections.clear();
			m_insertOrder.clear();
		}
		if(iniFile.is_open()){
			std::wstring section;
			std::wstring comment;
			while(getline(iniFile, origLine)){
				line = this->trim(origLine);
				if(line.size() == 0){
					// Empty line, do nothing
					comment += origLine + L"\n";
					continue;
				}
				if(line[0] == L';' || line[0] == L'#'){
					// Comment, do nothing
					comment += origLine + L"\n";
					continue;
				}
				if(line[0] == L'[' && line[line.length() - 1] == L']'){
					section = std::wstring(line.begin() + 1, line.end() - 1);
					//for(auto& ch : section){
					//	 ch = static_cast<wchar_t>(std::tolower(static_cast<wchar_t>(ch)));
					//}
					m_insertOrder.push_back(section);
					m_sections[section]._registered = true;
					m_sections[section]._comments = comment;
					comment.clear();
					continue;
				}
				size_t eq = line.find(L'=');
				if(eq != line.npos){
					std::wstring keyName(trim(std::wstring(line.begin(), line.begin() + eq)));
					std::wstring value(trim(std::wstring(line.begin() + eq + 1, line.end()), true));
					_key& key = m_sections[section]._keys[keyName];
					key._value = value;
					key._delete = false;
					key._registered = true;
					key._comments = comment;
					comment.clear();
					m_sections[section]._insertOrder.push_back(keyName);
					continue;
				}
				//error, invalid line (do nothing)
			}
			m_comments = comment;
			iniFile.close();
			if(iniFile.bad()) throw std::runtime_error("ini_file_read_error");
		}
		else{
			throw std::runtime_error("ini_file_not_opened");
		}
		return true;
	}

	bool _writeIni(){
		std::wofstream outFile;
		outFile.imbue(_utf8_locale);
		outFile.open(L"temp.ini", std::ios_base::out | std::ios_base::trunc);
		if(outFile.is_open()){
			for(auto const& sectionName : m_insertOrder){
				outFile << m_sections[sectionName]._comments;
				if(m_sections[sectionName]._delete == false){
					outFile << L'[' << sectionName << L']' << L'\n';
					for(auto const& keyName : m_sections[sectionName]._insertOrder){
						outFile << m_sections[sectionName]._keys[keyName]._comments;
						if(m_sections[sectionName]._keys[keyName]._delete == false){
							outFile << keyName << L'=' << m_sections[sectionName]._keys[keyName]._value << L'\n' << std::flush;
						}
					}
				}
			}
			outFile << m_comments;
			outFile.close();
			if(outFile.bad()) throw std::runtime_error("ini_file_write_error");
			if(_wrename(m_iniPath.c_str(), (m_iniPath + L".old").c_str())){
				_wremove(m_iniPath.c_str());
			}
			if(_wrename(L"temp.ini", m_iniPath.c_str())){
				return false;
			}
		}
		else{
			throw std::runtime_error("ini_file_not_opened");
		}
		return true;
	}

	public:

	//
	// @brief Retrieves a string from the specified section in an initialization file
	// @param sectionName - The name of the section containing the key name //use nullptr to search in all sections
	// @param keyName - The name of the key whose associated string is to be retrieved
	// @param Default - If specified key is not found, value of this parameter is returned
	// @return Value of associated key, if key is not found, default value is returned
	//
	std::wstring ReadString(const std::wstring& sectionName, const std::wstring& keyName, const std::wstring& Default = L""){
		try{
			if(m_sections.at(sectionName)._keys.at(keyName)._delete) return Default;//throw std::runtime_error("key_is_deleted");
			return m_sections.at(sectionName)._keys.at(keyName)._value;
		} catch(...){
			return Default;
		}
	}

	//
	// @brief Writes a string into the specified section of an initialization file
	// @param sectionName - The name of the section to which the string will be copied
	// @param keyName - The name of the key to be associated with a string
	// @param value - A string to be written to the file
	// @return Returns true on success, false on failure
	// @note
	// Truth table:
	// +---------+---------+---------+----------------------------------------------------------------+
	// | Section |   Key   |  Value  |  Behavior                                                      |
	// +---------+---------+---------+----------------------------------------------------------------+
	// |   "s"   |   "k"   |   "v"   | Writes value "v" to key "k" in section "s"                     |
	// +---------+---------+---------+----------------------------------------------------------------+
	// |   "s"   |   "k"   |   null  | Deletes key "k" from section "s"                               |
	// +---------+---------+---------+----------------------------------------------------------------+
	// |   "s"   |   null  |   "v"   | Currently not supported (throws Invalid Argument)              |
	// +---------+---------+---------+----------------------------------------------------------------+
	// |   "s"   |   null  |   null  | Deletes section "s" and all keys from it                       |
	// +---------+---------+---------+----------------------------------------------------------------+
	// |   null  |   "k"   |   "v"   | Writes value "v" to first occurrence of key "k" in any section |
	// +---------+---------+---------+----------------------------------------------------------------+
	// |   null  |   "k"   |   null  | Deletes first occurrence of key "k" in any section             |
	// +---------+---------+---------+----------------------------------------------------------------+
	// |   null  |   null  |   "v"   | Invalid                                                        |
	// +---------+---------+---------+----------------------------------------------------------------+
	// |   null  |   null  |   null  | Empties content of the                                         |
	// +---------+---------+---------+----------------------------------------------------------------+
	//
	bool WriteString(const std::basic_string<_Elem>& sectionName, const std::basic_string<_Elem>& keyName, const std::basic_string<_Elem>& value){
		try{
			m_sections[sectionName]._keys[keyName]._value = value;
			if(!m_sections[sectionName]._registered){
				m_insertOrder.push_back(sectionName);
				m_sections[sectionName]._registered = true;
			}
			if(!m_sections[sectionName]._keys[keyName]._registered){
				m_sections[sectionName]._insertOrder.push_back(keyName);
				m_sections[sectionName]._keys[keyName]._registered = true;
			}
			m_sections[sectionName]._delete = false;
			m_sections[sectionName]._keys[keyName]._delete = false;
			return true;
		} catch(...){
			return false;
		}
	}

	//
	// @brief Retrieves an integer from the specified section in an initialization file
	// @param sectionName - The name of the section containing the key name
	// @param keyName - The name of the key whose value is to be retrieved
	// @param Default - If specified key is not found, value of this parameter is returned
	// @return Value of associated key, if key is not found, default value is returned
	//
	int ReadInt(const std::basic_string<_Elem>& sectionName, const std::basic_string<_Elem>& keyName, const int Default = 0){
		return stoi(ReadString(sectionName, keyName, std::to_wstring(Default)));
	}

	//
	// @brief Writes an integer into the specified section of an initialization file
	// @param sectionName - The name of the section
	// @param keyName - The name of the key to be associated with an integer
	// @param value - An integer to be written to the file
	// @return Returns true on success, false on failure
	// 
	bool WriteInt(const std::basic_string<_Elem>& sectionName, const std::basic_string<_Elem>& keyName, const int value){
		return WriteString(sectionName, keyName, std::to_wstring(value));
	}

	//
	// @brief Deletes a key in the specified section of an initialization file
	// @param sectionName - The name of the section from which the key will be removed
	// @param keyName - The name of the key to be deleted
	// @return Returns true when key is deleted, false on failure
	// 
	bool DeleteKey(const std::basic_string<_Elem>& sectionName, const std::basic_string<_Elem>& keyName){
		try{
			m_sections.at(sectionName)._keys.at(keyName)._delete = true;
			return true;
		} catch(...){
			return false;
		}
	}

	//
	// @brief Deletes the specified section (and all keys in it) of an initialization file
	// @param sectionName - The name of the section to be deleted
	// @return Returns true when section is deleted, false on failure
	// 
	bool DeleteSection(const std::basic_string<_Elem>& sectionName){
		try{
			_section& section = m_sections.at(sectionName);
			section._delete = true;
			section._keys.clear();
			section._insertOrder.clear();
			return true;
		} catch(...){
			return false;
		}
	}

	//
	// @brief Inserts a comment (one or multiple lines starting with ';' or '#') before specified key or the specified section of an initialization file
	// @param sectionName - The name of the section
	// @param keyName - The name of the key. If key is an empty string, comment will be added
	// @param value - A comment to be inserted before the key. Comment can be multiple lines. Every line must start with ';' or '#'.
	// @return Returns true on success, false on failure
	// 
	bool SetComment(const std::basic_string<_Elem>& sectionName, const std::basic_string<_Elem>& keyName, const std::basic_string<_Elem>& value){
		try{
			if(keyName == L""){
				m_sections[sectionName]._keys[keyName]._comments = value;
			}
			else{
				m_sections[sectionName]._comments = value;
			}
			return true;
		} catch(...){
			return false;
		}
	}

	bool Init(){
		_parseIni(true);
		return true;
	}


	bool Reload(){
		return _parseIni();
	}

	bool Flush(){
		return _writeIni();
	}

	unsigned int GetSectionCount(){
		return m_insertOrder.size();
	}

	int GetKeyCount(const std::basic_string<_Elem>& sectionName){
		try{
			return m_sections.at(sectionName)._insertOrder.size();
		} catch(...){
			return 0;// -1;
		}
	}

	

	//static unsigned char CharToNibble(const char character) noexcept{
	//	return character >= '0' && character <= '9' ? character - '0' : (10 + character -'A') & 0x000f;
	//}

	enum ReadBinaryStatus{
		STATUS_SUCCESS,
		STATUS_UNKNOWN_ERROR,
		STATUS_BAD_CHECKSUM,
		STATUS_INVALID_ARGUMENT,
		STATUS_READ_ERROR,
		STATUS_INVALID_DATA_SIZE
	};

	unsigned int ReadBinary(const std::basic_string<_Elem>& sectionName, const std::basic_string<_Elem>& keyName, void* binaryDataBuffer, unsigned int binaryDataSize, bool verifyChecksum = true){

		if(binaryDataSize * 2 + 10 < binaryDataSize){
			return STATUS_INVALID_ARGUMENT;
		}

		std::basic_string<_Elem> structStr = this->ReadString(sectionName, keyName);

		if(!structStr.size()){
			return STATUS_READ_ERROR;
		}

		if(structStr.size() == binaryDataSize * 2 + 2){
			unsigned char checksum = 0;
			auto currentChar = structStr.begin();

			for(unsigned int i = 0; i < binaryDataSize; ++i){
				unsigned char currentByte;

				currentByte = CharToNibble(*currentChar);
				++currentChar;
				currentByte = (currentByte << 4) | CharToNibble(*currentChar);
				++currentChar;

				if(currentChar >= structStr.end()){
					if(!verifyChecksum || checksum == currentByte){
						return STATUS_SUCCESS;
					}
					else{
						return STATUS_BAD_CHECKSUM;
					}
				}
				checksum += currentByte;
				*((unsigned char*)binaryDataBuffer + i) = currentByte;
			}
		}
		else{
			return STATUS_INVALID_DATA_SIZE;
		}
		return STATUS_UNKNOWN_ERROR;
	}

	bool WriteBinary(const std::basic_string<_Elem>& sectionName, const std::basic_string<_Elem>& keyName, const void* binaryData, unsigned int dataSize){

		unsigned char checksum;
		const char hexChars[] = {
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
		};

		std::basic_string<_Elem> resultStr;

		if(binaryData == nullptr){
			return false;
		}

		if(dataSize * 2 + 3 < dataSize){
			//SetLastError(ERROR_INVALID_PARAMETER);
			return false;
		}

		checksum = 0;
		for(unsigned int i = 0; i < dataSize; i++){
			unsigned char currentByte = *((unsigned char*)binaryData + i);
			checksum += currentByte;
			resultStr.push_back(hexChars[(currentByte >> 4) & 0x000f]);
			resultStr.push_back(hexChars[currentByte & 0x000f]);
		}
		//append checksum
		resultStr.push_back(hexChars[(checksum >> 4) & 0x000f]);
		resultStr.push_back(hexChars[checksum & 0x000f]);

		return this->WriteString(sectionName, keyName, resultStr);
	}
};

#undef CharToNibble

#endif //__cplusplus
#endif //_INC_INIPARSER
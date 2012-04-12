/*
* C++ Ini File Parser library
* ---------------------------
* 
* > Main header file
*
* Author: Yurij Zagrebnoy © 2012
* Git page: https://github.com/zyura/cppini
* License: http://www.opensource.org/licenses/MIT
* (the information above should not be removed)
*/

#pragma once

#include <string>
#include <istream>

#ifdef CPPINI_MAP
	#include <map>
	#include <vector>
#endif

class CIniFileBase {
protected:
	std::istream & in;
	std::string s;
	std::string group;
	std::string param;
	std::string value;
	size_t pos;
	size_t line;
	bool terminated;
	bool done;

	enum State {S_INIT, S_GROUP, S_PARAM, S_END};

	static const int GT_ALLOW_SPACE = 1;
	static const int GT_QUOTE_CHAR  = 2;
	static const int GT_ALLOW_EMPTY = 4;

#ifdef CPPINI_FORCE_QUOTED_STR
	static const int GT_VALUE_FLAGS = GT_QUOTE_CHAR | GT_ALLOW_EMPTY;
#else
	static const int GT_VALUE_FLAGS = GT_QUOTE_CHAR | GT_ALLOW_EMPTY | GT_ALLOW_SPACE;
#endif

	void error(const char * msg);
	char skip_space();
	bool get_text(std::string & id, char term = 0, int flags = 0);
protected:
	virtual bool error_msg(const char * msg) = 0; // return false to halt parse
	virtual bool param_found(std::string & group, std::string & name, std::string & value) = 0; // return false to halt parse
public:
	CIniFileBase(std::istream & ins) : in(ins) { };
	bool parse(bool reset_stream = false);
};

class IIniFileCallback {
public:
	virtual bool ini_error_msg(size_t line, const char * msg) = 0;
	virtual bool ini_param_found(std::string & group, std::string & name, std::string & value) = 0;
};

class CIniFileCallback : public CIniFileBase {
protected:
	IIniFileCallback * cb;

	virtual bool error_msg(const char * msg) {
		return cb->ini_error_msg(line, msg);
	}

	virtual bool param_found(std::string & group, std::string & name, std::string & value) {
		return cb->ini_param_found(group, name, value);
	}
public:
	CIniFileCallback(std::istream & ins, IIniFileCallback * cbs)
		: cb(cbs), CIniFileBase(ins) { };
};

#ifdef CPPINI_MAP

class CIniFileMap_Class : public std::map<std::string, std::string> { };

class CIniFileMap_Pair : public std::pair<std::string, std::string> {
public:
	CIniFileMap_Pair(std::string key, std::string value)
		: std::pair<std::string, std::string>(key, value) { };
};

struct CIniFileMap_Error {
	const char * msg;
	size_t line;

	CIniFileMap_Error(size_t line, const char * msg) {
		this->msg = msg;
		this->line = line;
	}
};

class CIniFileMap_ErrorList : public std::vector<CIniFileMap_Error> { };


class CIniFileMap : public CIniFileBase {
protected:
	CIniFileMap_Class map;
	CIniFileMap_ErrorList err;

	virtual bool error_msg(const char * msg) {
		CIniFileMap_Error e(line, msg);
		err.push_back(e);
		return true;
	}

	virtual bool param_found(std::string & group, std::string & name, std::string & value) {
		std::string key = group + "." + name;
		map.insert(CIniFileMap_Pair(key, value));
		return true;
	}
public:
	CIniFileMap(std::istream & ins) : CIniFileBase(ins) { };

	inline void reset() {
		map.clear();
		err.clear();
	}

	inline CIniFileMap_Class & get_map() {
		return map;
	}

	inline CIniFileMap_ErrorList & get_errors() {
		return err;
	}
};

#endif
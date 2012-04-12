/*
* C++ Ini File Parser library
* ---------------------------
* 
* > Main header file
*
* Author: Yurij Zagrebnoy © 2012
* Version: 1.0.0
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
	#include <sstream>
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
	static const int GT_PARAM_FLAGS = GT_ALLOW_SPACE;

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
typedef CIniFileMap_Class::const_iterator CIniFileMap_Iter;

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
typedef CIniFileMap_ErrorList::const_iterator CIniFileMap_ErrorIter;

bool ini_conv_strtoint(std::string & str, int & val);
bool ini_conv_strtodbl(std::string & str, double & val);

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
		std::stringstream sstr;
		sstr << group << '.' << name;
		map.insert(CIniFileMap_Pair(sstr.str(), value));
		return true;
	}
public:
	CIniFileMap(std::istream & ins, bool do_parse = true, bool reset_stream = false) : CIniFileBase(ins) {
		if (do_parse) parse(reset_stream);
	};

	inline void reset() {
		map.clear();
		err.clear();
	}

	/*
	* usually used as
	*   const CIniFileMap_Class & mp = inimap.get_map();
	*   for (CIniFileMap_Iter it = mp.begin(); it != mp.end(); it++)
	*     ...
	*/
	inline const CIniFileMap_Class & get_map() const {
		return map;
	}

	/*
	* usually used as
	*   const CIniFileMap_ErrorList & err = inimap.get_errors();
	*     for (CIniFileMap_ErrorIter it = err.begin(); it != err.end(); it++)
	*       ...
	*/
	inline const CIniFileMap_ErrorList & get_errors() const {
		return err;
	}

	std::string get(const char * group, const char * name, const char * def = "") const {
		std::stringstream sstr;
		sstr << group << '.' << name;
		return get(sstr.str(), def);
	}

	std::string get(const char * key, const char * def = "") const {
		std::string skey(key);
		return get(skey);
	}

	std::string get(std::string & skey, const char * def = "") const {
		CIniFileMap_Class::const_iterator it = map.find(skey);
		return it == map.end() ? def : it->second;
	}

	bool try_get(std::string & skey, std::string & value) const {
		CIniFileMap_Class::const_iterator it = map.find(skey);
		if (it == map.end()) return false;
		value = it->second;
		return true;
	}

	bool try_get(const char * key, std::string & value) const {
		std::string skey(key);
		return try_get(skey, value);
	}

	inline int geti(const char * key, const int def = 0) const {
		return get<int, ini_conv_strtoint>(key, def);
	}

	inline double getd(const char * key, const double def = 0) const {
		return get<double, ini_conv_strtodbl>(key, def);
	}

	inline bool try_geti(const char * key, int & value) const {
		return try_get<int, ini_conv_strtoint>(key, value);
	}

	inline bool try_getd(const char * key, double & value) const {
		return try_get<double, ini_conv_strtodbl>(key, value);
	}

	template <typename T, bool (*conv_func)(std::string &, T &)>
	T get(const char * key, const T def) const {
		std::string value;
		if (!try_get(key, value)) return def;
		T result;
		return conv_func(value, result) ? result : def;
	}

	template <typename T, bool (*conv_func)(std::string &, T &)>
	bool try_get(const char * key, T & result) const {
		std::string value;
		if (!try_get(key, value)) return false;
		return conv_func(value, result);
	}
};

#endif
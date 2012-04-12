/*
* C++ Ini File Parser library
* ---------------------------
* 
* > Examples
*
* Author: Yurij Zagrebnoy © 2012
* Version: 1.0.0
* Git page: https://github.com/zyura/cppini
* License: http://www.opensource.org/licenses/MIT
* (the information above should not be removed)
*/

#include <iostream>
#include <fstream>
#include <string> 

#define CPPINI_MAP // we want to use CIniFileMap class
#include "cppini.hpp"

#ifdef _WIN32
	#include <windows.h>

	unsigned int set_console_color(unsigned int color) {
		HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hstdout, &csbi);
		SetConsoleTextAttribute(hstdout, color);
		return csbi.wAttributes;
	}

#else
	unsigned int set_console_color(int color) { }
#endif

using namespace std;

/* ================ first way - inherit CIniFileBase class ================ */

class CIniFile : public CIniFileBase {
public:
	CIniFile(istream & in) : CIniFileBase(in) { };
protected:
	virtual bool error_msg(const char * msg) {
		unsigned int saveColor = set_console_color(0x0C);
		cout << "[1] " << "  >> Error: " << msg << " at line: " << line << endl;
		set_console_color(saveColor);
		return true;
	}

	virtual bool param_found(string & group, string & name, string & value) {
		cout << "[1] " << group << '.' << name << " = " << value << endl;
		return true;
	}
};

/* ================ second way - implement IIniFileCallback interface ================ */

class CApplicationConfig : public IIniFileCallback {
protected:
	virtual bool ini_error_msg(size_t line, const char * msg) {
		unsigned int saveColor = set_console_color(0x0C);
		cout << "[2] " << "  >> Error: " << msg << " at line: " << line << endl;
		set_console_color(saveColor);
		return true;
	}

	virtual bool ini_param_found(std::string & group, std::string & name, std::string & value) {
		cout << "[2] " << group << '.' << name << " = " << value << endl;
		return true;
	}
public:
	CApplicationConfig() { }
	
	void read(istream & in) {
		CIniFileCallback inicb(in, this);
		inicb.parse(true); // reset input stream position
	}
};


int main() {
	ifstream in("data.ini");

	cout << "First way - inherit CIniFileBase" << endl;
	CIniFile ini(in);
	ini.parse();


	cout << endl << "Second way - implement IIniFileCallback" << endl;
	CApplicationConfig * cfg = new CApplicationConfig();
	cfg->read(in);


	cout << endl << "Third way - use CIniFileMap class" << endl;
	CIniFileMap inimap(in, true, true); // do_parse, reset_stream
	CIniFileMap_Class & mp = inimap.get_map();
	for (CIniFileMap_Class::iterator it = mp.begin(); it != mp.end(); it++)
		cout << "[3] " << it->first << " = " << it->second << endl;
	CIniFileMap_ErrorList & err = inimap.get_errors();
	if (err.size()) {
		cout << "Ini file parse errors:" << endl;
		for (CIniFileMap_ErrorList::iterator it = err.begin(); it != err.end(); it++)
			cout << it->msg << " at line " << it->line << endl;
	}


	cout << endl << "Using CIniFileMap.get()" << endl;
	cout << inimap.get("PHP.output_buffering") << endl;
	cout << inimap.get_int("PHP.precision") * 2 << endl;


	string s;
	getline(cin, s);
}

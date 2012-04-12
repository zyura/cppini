/*
* C++ Ini File Parser library
* ---------------------------
* 
* > Implementation
*
* Author: Yurij Zagrebnoy © 2012
* Git page: https://github.com/zyura/cppini
* License: http://www.opensource.org/licenses/MIT
* (the information above should not be removed)
*/

#include "cppini.hpp"

using namespace std;

char CIniFileBase::skip_space() {
	size_t len = s.length();
	for (char c; pos < len; pos++)
		if (!isspace(c = s[pos])) return c;
	return 0;
}

bool CIniFileBase::get_text(string & text, char term, int flags) {
	bool quote = false;
	if (flags & GT_QUOTE_CHAR && s[pos] == term) {
		quote = true;
		flags |= GT_ALLOW_SPACE;
		pos++;
	}
	size_t start = pos, len = s.length();
	char c;
	for (; pos < len; pos++) {
		c = s[pos];
		if (!(flags & GT_ALLOW_SPACE) && isspace(c) || c == term) break;
	}
	if (pos == start && !(flags & GT_ALLOW_EMPTY)) return false;
	text = s.substr(start, pos - start);
	if (quote && c == term) pos++;
	return true;
}

void CIniFileBase::error(const char * msg) {
	if (!error_msg(msg)) terminated = true;
	done = true;
}

bool CIniFileBase::parse(bool reset_stream) {
	if (reset_stream) {
		if (in.fail()) in.clear();
		in.seekg(0);
	}

	terminated = false;
	line = 0;
	while (!terminated && !in.eof()) {
		getline(in, s);
		
		line++;
		State state = S_INIT;
		pos = 0;
		done = false;
		char c;

		while (!done)
			switch (state) {
				case S_INIT:
					c = skip_space();
					if (c == 0 || c == ';')
						done = true;
					else if (c == '[')
						state = S_GROUP;
					else
						state = S_PARAM;
					break;
				case S_GROUP:
					if (pos++, skip_space() == 0 || !get_text(group, ']', GT_ALLOW_SPACE))
						error("Invalid group name");
					else if (skip_space() != ']')
						error("Group name is not closed");
					else
						pos++, state = S_END;
					break;
				case S_PARAM:
					value = "";
					if (!get_text(param, '='))
						error("Parameter name is invalid");
					else if (skip_space() != '=')
						error("Equal sign expected");
					else if (pos++, skip_space() == 0 || get_text(value, '"', GT_VALUE_FLAGS))
						if (!param_found(group, param, value))
							return false;
						else
							state = S_END;
					else
						error("Parameter value is invalid");
					break;
				case S_END:
					c = skip_space();
					if (c != 0 && c != ';')
						error("Unexpected characters at the end of the line");
					done = true;
					break;
			}
	}
	return true;
}

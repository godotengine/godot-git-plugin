#include "git_wrappers.h"

#include <cstring>

CString::CString(const godot::String &string) {
	godot::CharString godot_char_str = string.utf8();

	data = new char[godot_char_str.length() + 1];
	std::memcpy(data, (void *)godot_char_str.get_data(), godot_char_str.length());
	data[godot_char_str.length()] = '\0';
}

CString::~CString() {
	if (data) {
		delete[] data;
		data = nullptr;
	}
}

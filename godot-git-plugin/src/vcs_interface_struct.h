#ifndef VCS_INTERFACE_STRUCT_H
#define VCS_INTERFACE_STRUCT_H

#include <Godot.hpp>

namespace godot {

struct VCSInterface {

	void (*constructor)();
	void (*destructor)();

	godot::String (*get_vcs_name)();
};

extern VCSInterface *vcs_api_struct; 

}

#endif // !VCS_INTERFACE_STRUCT_H

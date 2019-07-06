#include "git_api.h"
#include "vcs_interface_struct.h"

#include <ClassDB.hpp>

namespace godot {

VCSInterface *vcs_api_struct = NULL;

String GitAPI::_get_vcs_name() {

	WARN_PRINT("VCS Name is return as Git");
	return "Git";
}

}

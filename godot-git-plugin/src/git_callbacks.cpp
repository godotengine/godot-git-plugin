#include <git_callbacks.h>

int status_callback(const char *p_path, const char *p_matched_pathspec, void *p_payload) {

	StatusPayload p = *(StatusPayload *)(p_payload);
	int ret;
	unsigned int status;

	if (git_status_file(&status, p.repo, p_path)) {

		return -1;
	}

	if ((status & GIT_STATUS_WT_MODIFIED) || (status & GIT_STATUS_WT_NEW)) {

		godot::Godot::print("Staged '%s'", p_path);
		ret = 0;
	} else {

		ret = 1;
	}

	return ret;
}

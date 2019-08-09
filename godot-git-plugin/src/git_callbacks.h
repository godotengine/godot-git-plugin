#ifndef GIT_CALLBACKS_H
#define GIT_CALLBACKS_H

#include <Godot.hpp>
#include <git2.h>

#include <git_common.h>

int status_callback(const char *p_path, const char *p_matched_pathspec, void *p_payload);

#endif // !GIT_CALLBACKS_H

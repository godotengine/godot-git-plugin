#ifndef GIT_COMMON_H
#define GIT_COMMON_H

#include <cstdio>

#include <Godot.hpp>

#include <git2.h>

void check_git2_errors(int error, const char *message, const char *extra);

extern "C" int diff_line_callback_function(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line, void *payload);

#define GIT2_CALL(function_call, m_error_msg, m_additional_msg) check_git2_errors(function_call, m_error_msg, m_additional_msg);

#endif // !GIT_COMMON_H

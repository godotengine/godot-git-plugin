#ifndef GIT_COMMON_H
#define GIT_COMMON_H

#include <Godot.hpp>

#include <git2.h>

void check_git2_errors(int error, const char *message, const char *extra);

#define GIT2_CALL(function_call, m_error_msg, m_additional_msg)                \
	{                                                                          \
		check_git2_errors(function_call, m_error_msg, m_additional_msg);       \
	}

struct StatusPayload {

	git_repository *repo;
};

#endif // !GIT_COMMON_H

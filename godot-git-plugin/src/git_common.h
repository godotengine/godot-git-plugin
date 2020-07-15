#ifndef GIT_COMMON_H
#define GIT_COMMON_H

#include <Godot.hpp>

#include <git2.h>

struct Credentials {
	char *username;
	char *password;
};

extern "C" int progress_cb(const char *str, int len, void *data);
extern "C" int update_cb(const char *refname, const git_oid *a, const git_oid *b, void *data);
extern "C" int transfer_progress_cb(const git_indexer_progress *stats, void *payload);
extern "C" int fetchhead_foreach_cb(const char *ref_name, const char *remote_url, const git_oid *oid, unsigned int is_merge, void *payload);
extern "C" int credentials_cb(git_cred **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload);
extern "C" int push_transfer_progress_cb(unsigned int current, unsigned int total, size_t bytes, void *payload);
extern "C" int push_update_reference_cb(const char *refname, const char *status, void *data);

bool check_git2_errors(int error, godot::String message, godot::String function, godot::String file, int line);

#define GIT2_CALL(function_call, m_error_msg)                                            \
	if (check_git2_errors(function_call, m_error_msg, __FUNCTION__, __FILE__, __LINE__)) \
		return;

#define GIT2_CALL_R(function_call, m_error_msg, return_t)                                \
	if (check_git2_errors(function_call, m_error_msg, __FUNCTION__, __FILE__, __LINE__)) \
		return return_t;
#endif // !GIT_COMMON_H

#include <iostream>
#include <cstring>

#include "git_callbacks.h"
#include "git_plugin.h"

#include "godot_cpp/godot.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

extern "C" int progress_cb(const char *str, int len, void *data) {
	(void)data;

	godot::UtilityFunctions::print("remote: ", godot::String::utf8(str, len).strip_edges());

	return 0;
}

extern "C" int update_cb(const char *refname, const git_oid *a, const git_oid *b, void *data) {
	constexpr int short_commit_length = 8;
	char a_str[short_commit_length + 1];
	char b_str[short_commit_length + 1];
	(void)data;

	git_oid_tostr(b_str, short_commit_length, b);
	if (git_oid_is_zero(a)) {
		godot::UtilityFunctions::print("* [new] ", godot::String::utf8(b_str), " ", godot::String::utf8(refname));
	} else {
		git_oid_tostr(a_str, short_commit_length, a);
		godot::UtilityFunctions::print("[updated] ", godot::String::utf8(a_str), "...", godot::String::utf8(b_str), " ", godot::String::utf8(refname));
	}

	return 0;
}

extern "C" int transfer_progress_cb(const git_indexer_progress *stats, void *payload) {
	(void)payload;

	if (stats->received_objects == stats->total_objects) {
		godot::UtilityFunctions::print("Resolving deltas ", uint32_t(stats->indexed_deltas), "/", uint32_t(stats->total_deltas));
	} else if (stats->total_objects > 0) {
		godot::UtilityFunctions::print(
				"Received ", uint32_t(stats->received_objects), "/", uint32_t(stats->total_objects),
				" objects (", uint32_t(stats->indexed_objects), ") in ", uint32_t(stats->received_bytes), " bytes");
	}
	return 0;
}

extern "C" int fetchhead_foreach_cb(const char *ref_name, const char *remote_url, const git_oid *oid, unsigned int is_merge, void *payload) {
	if (is_merge) {
		git_oid_cpy((git_oid *)payload, oid);
	}
	return 0;
}

extern "C" int push_transfer_progress_cb(unsigned int current, unsigned int total, size_t bytes, void *payload) {
	int64_t progress = 100;

	if (total != 0) {
		progress = (current * 100) / total;
	}

	godot::UtilityFunctions::print("Writing Objects: ", uint32_t(progress), "% (", uint32_t(current), "/", uint32_t(total), ", ", uint32_t(bytes), " bytes done.)");
	return 0;
}

extern "C" int push_update_reference_cb(const char *refname, const char *status, void *data) {
	if (status != NULL) {
		godot::String status_str = godot::String::utf8(status);
		godot::UtilityFunctions::print("[rejected] ", godot::String::utf8(refname), " ", status_str);
	} else {
		godot::UtilityFunctions::print("[updated] ", godot::String::utf8(refname));
	}
	return 0;
}

extern "C" int credentials_cb(git_cred **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload) {
	Credentials *creds = (Credentials *)payload;

	godot::String proper_username = username_from_url ? username_from_url : creds->username;

	if (!creds->ssh_public_key_path.is_empty()) {
		if (allowed_types & GIT_CREDENTIAL_SSH_KEY) {
			return git_credential_ssh_key_new(out,
					CString(proper_username).data,
					CString(creds->ssh_public_key_path).data,
					CString(creds->ssh_private_key_path).data,
					CString(creds->ssh_passphrase).data);
		}
	}

	if (allowed_types & GIT_CREDENTIAL_USERPASS_PLAINTEXT) {
		return git_cred_userpass_plaintext_new(out, CString(proper_username).data, CString(creds->password).data);
	}

	if (allowed_types & GIT_CREDENTIAL_USERNAME) {
		return git_credential_username_new(out, CString(proper_username).data);
	}

	return GIT_EUSER;
}

extern "C" int diff_hunk_cb(const git_diff_delta *delta, const git_diff_hunk *range, void *payload) {
	DiffHelper *diff_helper = (DiffHelper *)payload;

	godot::Dictionary hunk = diff_helper->git_plugin->create_diff_hunk(range->old_start, range->new_start, range->old_lines, range->new_lines);
	diff_helper->diff_hunks->push_back(hunk);

	return 1;
}

void ParseDiffPayload::flush_hunk() {
	if (!has_current_hunk) {
		return;
	}
	current_hunk = git_plugin->add_line_diffs_into_diff_hunk(current_hunk, current_hunk_lines);
	current_file_hunks.push_back(current_hunk);
	has_current_hunk = false;
	current_hunk = godot::Dictionary();
	current_hunk_lines = godot::TypedArray<godot::Dictionary>();
}

void ParseDiffPayload::flush_file() {
	flush_hunk();
	if (!has_current_file) {
		return;
	}
	current_file = git_plugin->add_diff_hunks_into_diff_file(current_file, current_file_hunks);
	diff_contents->push_back(current_file);
	has_current_file = false;
	current_file = godot::Dictionary();
	current_file_hunks = godot::TypedArray<godot::Dictionary>();
}

extern "C" int parse_diff_file_cb(const git_diff_delta *delta, float progress, void *payload) {
	(void)progress;
	ParseDiffPayload *state = (ParseDiffPayload *)payload;
	state->flush_file();
	state->current_file = state->git_plugin->create_diff_file(
			godot::String::utf8(delta->new_file.path),
			godot::String::utf8(delta->old_file.path));
	state->has_current_file = true;
	return 0;
}

extern "C" int parse_diff_binary_cb(const git_diff_delta *delta, const git_diff_binary *binary, void *payload) {
	(void)delta;
	(void)binary;
	(void)payload;
	return 0;
}

extern "C" int parse_diff_hunk_cb(const git_diff_delta *delta, const git_diff_hunk *hunk, void *payload) {
	(void)delta;
	ParseDiffPayload *state = (ParseDiffPayload *)payload;
	state->flush_hunk();
	state->current_hunk = state->git_plugin->create_diff_hunk(
			hunk->old_start, hunk->new_start, hunk->old_lines, hunk->new_lines);
	state->has_current_hunk = true;
	return 0;
}

extern "C" int parse_diff_line_cb(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line, void *payload) {
	(void)delta;
	(void)hunk;
	ParseDiffPayload *state = (ParseDiffPayload *)payload;

	char *content = new char[line->content_len + 1];
	std::memcpy(content, line->content, line->content_len);
	content[line->content_len] = '\0';

	godot::String status = " ";
	status[0] = line->origin;
	state->current_hunk_lines.push_back(state->git_plugin->create_diff_line(
			line->new_lineno, line->old_lineno, godot::String::utf8(content), status));

	delete[] content;
	return 0;
}

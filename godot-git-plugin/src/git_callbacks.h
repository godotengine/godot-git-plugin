#pragma once

#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/typed_array.hpp"
#include "git2.h"

class GitPlugin;

struct DiffHelper {
	godot::Array *diff_hunks;
	GitPlugin *git_plugin;
};

struct ParseDiffPayload {
	GitPlugin *git_plugin = nullptr;
	godot::TypedArray<godot::Dictionary> *diff_contents = nullptr;

	bool has_current_file = false;
	godot::Dictionary current_file;
	godot::TypedArray<godot::Dictionary> current_file_hunks;

	bool has_current_hunk = false;
	godot::Dictionary current_hunk;
	godot::TypedArray<godot::Dictionary> current_hunk_lines;

	void flush_hunk();
	void flush_file();
};

extern "C" int progress_cb(const char *str, int len, void *data);
extern "C" int update_cb(const char *refname, const git_oid *a, const git_oid *b, void *data);
extern "C" int transfer_progress_cb(const git_indexer_progress *stats, void *payload);
extern "C" int fetchhead_foreach_cb(const char *ref_name, const char *remote_url, const git_oid *oid, unsigned int is_merge, void *payload);
extern "C" int credentials_cb(git_cred **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload);
extern "C" int push_transfer_progress_cb(unsigned int current, unsigned int total, size_t bytes, void *payload);
extern "C" int push_update_reference_cb(const char *refname, const char *status, void *data);
extern "C" int diff_hunk_cb(const git_diff_delta *delta, const git_diff_hunk *range, void *payload);

extern "C" int parse_diff_file_cb(const git_diff_delta *delta, float progress, void *payload);
extern "C" int parse_diff_binary_cb(const git_diff_delta *delta, const git_diff_binary *binary, void *payload);
extern "C" int parse_diff_hunk_cb(const git_diff_delta *delta, const git_diff_hunk *hunk, void *payload);
extern "C" int parse_diff_line_cb(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line, void *payload);

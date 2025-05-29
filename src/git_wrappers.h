#pragma once

#include <memory>

#include "godot_cpp/variant/string.hpp"
#include "git2.h"

class GitPlugin;

struct CString {
	char *data = nullptr;

	CString(const godot::String &string);
	~CString();
	CString() = delete;
	CString(CString &&) = delete;
	CString &operator=(const CString &) = delete;
	CString &operator=(CString &&) = delete;
};

template <class T>
class Capture {
	T &smart_ptr = nullptr;

	using Raw = decltype(smart_ptr.get());

	Raw raw = nullptr;

public:
	Capture() = delete;
	Capture(T &ptr) :
			smart_ptr(ptr) {}
	Capture(Capture &&) = delete;
	Capture &operator=(const Capture &) = delete;
	Capture &operator=(Capture &&) = delete;

	operator Raw *() {
		return &raw;
	}

	~Capture() {
		if (raw) {
			smart_ptr.reset(raw);
		}
	}
};

template <auto DeleteFn>
struct FunctionDeleter {
	template <class T>
	void operator()(T *ptr) {
		DeleteFn(ptr);
	}
};

template <class T, auto DeleteFn>
using unique_ptr_deleter = std::unique_ptr<T, FunctionDeleter<DeleteFn>>;

using git_annotated_commit_ptr = unique_ptr_deleter<git_annotated_commit, git_annotated_commit_free>;
using git_blob_ptr = unique_ptr_deleter<git_blob, git_blob_free>;
using git_branch_iterator_ptr = unique_ptr_deleter<git_branch_iterator, git_branch_iterator_free>;
using git_commit_ptr = unique_ptr_deleter<git_commit, git_commit_free>;
using git_diff_ptr = unique_ptr_deleter<git_diff, git_diff_free>;
using git_index_ptr = unique_ptr_deleter<git_index, git_index_free>;
using git_object_ptr = unique_ptr_deleter<git_object, git_object_free>;
using git_patch_ptr = unique_ptr_deleter<git_patch, git_patch_free>;
using git_reference_ptr = unique_ptr_deleter<git_reference, git_reference_free>;
using git_remote_ptr = unique_ptr_deleter<git_remote, git_remote_free>;
using git_repository_ptr = unique_ptr_deleter<git_repository, git_repository_free>;
using git_revwalk_ptr = unique_ptr_deleter<git_revwalk, git_revwalk_free>;
using git_signature_ptr = unique_ptr_deleter<git_signature, git_signature_free>;
using git_status_list_ptr = unique_ptr_deleter<git_status_list, git_status_list_free>;
using git_tree_ptr = unique_ptr_deleter<git_tree, git_tree_free>;
using git_tree_entry_ptr = unique_ptr_deleter<git_tree_entry, git_tree_entry_free>;

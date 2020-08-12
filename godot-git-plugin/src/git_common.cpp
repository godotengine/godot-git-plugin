#include <git_api.h>
#include <git_common.h>

bool check_git2_errors(int error, godot::String message, godot::String function, godot::String file, int line) {

	const git_error *lg2err;
	godot::String lg2msg = "", lg2spacer = "";

	if (!error) {

		return false;
	}

	if ((lg2err = git_error_last()) != NULL && lg2err->message != NULL) {

		lg2msg = godot::String(lg2err->message);
		lg2spacer = " - ";
	}

	godot::Godot::print_error("Git API: " + message + " [" + godot::String::num_int64(error) + "] " + lg2spacer + lg2msg, function, file, line);

	return true;
}

extern "C" int progress_cb(const char *str, int len, void *data) {
	(void)data;
	godot::Godot::print("remote: " + godot::String(str).strip_edges());
	return 0;
}

extern "C" int update_cb(const char *refname, const git_oid *a, const git_oid *b, void *data) {
	int short_commit_length = 8;
	char a_str[short_commit_length + 1], b_str[short_commit_length + 1];
	(void)data;

	git_oid_tostr(b_str, short_commit_length - 1, b);
	b_str[short_commit_length] = '\0';
	if (git_oid_is_zero(a)) {
		godot::Godot::print("* [new] " + godot::String(b_str) + " " + godot::String(refname));
	} else {
		git_oid_nfmt(a_str, short_commit_length - 1, a);
		a_str[short_commit_length] = '\0';
		godot::Godot::print("[updated] " + godot::String(a_str) + "..." + godot::String(b_str) + " " + godot::String(refname));
	}

	return 0;
}

extern "C" int transfer_progress_cb(const git_indexer_progress *stats, void *payload) {

	(void)payload;

	if (stats->received_objects == stats->total_objects) {
		godot::Godot::print("Resolving deltas " + godot::String::num_int64(stats->indexed_deltas) + "/" + godot::String::num_int64(stats->total_deltas));
	} else if (stats->total_objects > 0) {
		godot::Godot::print("Received " + godot::String::num_int64(stats->received_objects) + "/" + godot::String::num_int64(stats->total_objects) + " objects (" + godot::String::num_int64(stats->indexed_objects) + ") in " + godot::String::num_int64(static_cast<int>(stats->received_bytes)) + " bytes");
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
	godot::Godot::print("Writing Objects: " + godot::String::num_int64((int)current * 100 / total) + "% (" + godot::String::num_int64((int)current) + "/" + godot::String::num_int64((int)total) + "), " + godot::String::num_int64(bytes) + " bytes, done.");
	return 0;
}

extern "C" int push_update_reference_cb(const char *refname, const char *status, void *data) {
	if (status == "") {
		godot::Godot::print("[rejected] " + godot::String(refname) + " " + godot::String(status));
	} else {
		godot::Godot::print("[updated] " + godot::String(refname));
	}
	return 0;
}

extern "C" int credentials_cb(git_cred **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload) {
	Credentials *creds = (Credentials *)payload;
	if (creds->username == "" || creds->password == "") {
		return GIT_EUSER;
	}

	return git_cred_userpass_plaintext_new(out, creds->username, creds->password);
}

extern "C" int diff_hunk_cb(const git_diff_delta *delta, const git_diff_hunk *range, void *payload){
	godot::Array *diff_hunks = (godot::Array *)payload;
	godot::Dictionary hunk;
	hunk["old_start"] = range->old_start;
	hunk["old_lines"] = range->old_lines;
	hunk["new_start"] = range->new_start;
	hunk["new_lines"] = range->new_lines;
	diff_hunks->push_back(hunk);

	return 1;
}
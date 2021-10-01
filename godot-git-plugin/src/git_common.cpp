#include <git_api.h>
#include <git_common.h>

void check_git2_errors(int error, const char *message, const char *extra) {
	const git_error *lg2err;
	const char *lg2msg = "", *lg2spacer = "";

	if (!error) {
		return;
	}

	if ((lg2err = git_error_last()) != NULL && lg2err->message != NULL) {
		lg2msg = lg2err->message;
		lg2spacer = " - ";
	}

	if (extra) {
		printf("Git API: %s '%s' [%d]%s%s\n", message, extra, error, lg2spacer, lg2msg);
	} else {
		printf("Git API: %s [%d]%s%s\n", message, error, lg2spacer, lg2msg);
	}
}

extern "C" int diff_line_callback_function(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line, void *payload) {
	// First we NULL terminate the line text incoming
	char *content = new char[line->content_len + 1];
	memcpy(content, line->content, line->content_len);
	static int i = 0;
	content[line->content_len] = '\0';

	godot::String prefix = "";
	switch (line->origin) {
		case GIT_DIFF_LINE_DEL_EOFNL:
		case GIT_DIFF_LINE_DELETION:
			prefix = "-";
			break;

		case GIT_DIFF_LINE_ADD_EOFNL:
		case GIT_DIFF_LINE_ADDITION:
			prefix = "+";
			break;
	}

	godot::String content_str = content;

	godot::Dictionary result;
	result["content"] = prefix + content_str;
	result["status"] = prefix;
	result["new_line_number"] = line->new_lineno;
	result["line_count"] = line->num_lines;
	result["old_line_number"] = line->old_lineno;
	result["offset"] = line->content_offset;

	godot::GitAPI::get_singleton()->diff_contents.push_back(result);

	return 0;
}

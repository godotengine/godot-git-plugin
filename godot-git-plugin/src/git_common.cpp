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

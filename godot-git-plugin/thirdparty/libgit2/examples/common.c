/*
 * Utilities library for libgit2 examples
 *
 * Written by the libgit2 contributors
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common.h"

void check_lg2(int error, const char *message, const char *extra)
{
	const git_error *lg2err;
	const char *lg2msg = "", *lg2spacer = "";

	if (!error)
		return;

	if ((lg2err = git_error_last()) != NULL && lg2err->message != NULL) {
		lg2msg = lg2err->message;
		lg2spacer = " - ";
	}

	if (extra)
		fprintf(stderr, "%s '%s' [%d]%s%s\n",
			message, extra, error, lg2spacer, lg2msg);
	else
		fprintf(stderr, "%s [%d]%s%s\n",
			message, error, lg2spacer, lg2msg);

	exit(1);
}

void fatal(const char *message, const char *extra)
{
	if (extra)
		fprintf(stderr, "%s %s\n", message, extra);
	else
		fprintf(stderr, "%s\n", message);

	exit(1);
}

size_t is_prefixed(const char *str, const char *pfx)
{
	size_t len = strlen(pfx);
	return strncmp(str, pfx, len) ? 0 : len;
}

int optional_str_arg(
	const char **out, struct args_info *args, const char *opt, const char *def)
{
	const char *found = args->argv[args->pos];
	size_t len = is_prefixed(found, opt);

	if (!len)
		return 0;

	if (!found[len]) {
		if (args->pos + 1 == args->argc) {
			*out = def;
			return 1;
		}
		args->pos += 1;
		*out = args->argv[args->pos];
		return 1;
	}

	if (found[len] == '=') {
		*out = found + len + 1;
		return 1;
	}

	return 0;
}

int match_str_arg(
	const char **out, struct args_info *args, const char *opt)
{
	const char *found = args->argv[args->pos];
	size_t len = is_prefixed(found, opt);

	if (!len)
		return 0;

	if (!found[len]) {
		if (args->pos + 1 == args->argc)
			fatal("expected value following argument", opt);
		args->pos += 1;
		*out = args->argv[args->pos];
		return 1;
	}

	if (found[len] == '=') {
		*out = found + len + 1;
		return 1;
	}

	return 0;
}

static const char *match_numeric_arg(struct args_info *args, const char *opt)
{
	const char *found = args->argv[args->pos];
	size_t len = is_prefixed(found, opt);

	if (!len)
		return NULL;

	if (!found[len]) {
		if (args->pos + 1 == args->argc)
			fatal("expected numeric value following argument", opt);
		args->pos += 1;
		found = args->argv[args->pos];
	} else {
		found = found + len;
		if (*found == '=')
			found++;
	}

	return found;
}

int match_uint16_arg(
	uint16_t *out, struct args_info *args, const char *opt)
{
	const char *found = match_numeric_arg(args, opt);
	uint16_t val;
	char *endptr = NULL;

	if (!found)
		return 0;

	val = (uint16_t)strtoul(found, &endptr, 0);
	if (!endptr || *endptr != '\0')
		fatal("expected number after argument", opt);

	if (out)
		*out = val;
	return 1;
}

int match_uint32_arg(
	uint32_t *out, struct args_info *args, const char *opt)
{
	const char *found = match_numeric_arg(args, opt);
	uint16_t val;
	char *endptr = NULL;

	if (!found)
		return 0;

	val = (uint32_t)strtoul(found, &endptr, 0);
	if (!endptr || *endptr != '\0')
		fatal("expected number after argument", opt);

	if (out)
		*out = val;
	return 1;
}

static int match_int_internal(
	int *out, const char *str, int allow_negative, const char *opt)
{
	char *endptr = NULL;
	int	  val = (int)strtol(str, &endptr, 10);

	if (!endptr || *endptr != '\0')
		fatal("expected number", opt);
	else if (val < 0 && !allow_negative)
		fatal("negative values are not allowed", opt);

	if (out)
		*out = val;

	return 1;
}

int match_bool_arg(int *out, struct args_info *args, const char *opt)
{
	const char *found = args->argv[args->pos];

	if (!strcmp(found, opt)) {
		*out = 1;
		return 1;
	}

	if (!strncmp(found, "--no-", strlen("--no-")) &&
	    !strcmp(found + strlen("--no-"), opt + 2)) {
		*out = 0;
		return 1;
	}

	*out = -1;
	return 0;
}

int is_integer(int *out, const char *str, int allow_negative)
{
	return match_int_internal(out, str, allow_negative, NULL);
}

int match_int_arg(
	int *out, struct args_info *args, const char *opt, int allow_negative)
{
	const char *found = match_numeric_arg(args, opt);
	if (!found)
		return 0;
	return match_int_internal(out, found, allow_negative, opt);
}

int diff_output(
	const git_diff_delta *d,
	const git_diff_hunk *h,
	const git_diff_line *l,
	void *p)
{
	FILE *fp = (FILE*)p;

	(void)d; (void)h;

	if (!fp)
		fp = stdout;

	if (l->origin == GIT_DIFF_LINE_CONTEXT ||
		l->origin == GIT_DIFF_LINE_ADDITION ||
		l->origin == GIT_DIFF_LINE_DELETION)
		fputc(l->origin, fp);

	fwrite(l->content, 1, l->content_len, fp);

	return 0;
}

void treeish_to_tree(
	git_tree **out, git_repository *repo, const char *treeish)
{
	git_object *obj = NULL;

	check_lg2(
		git_revparse_single(&obj, repo, treeish),
		"looking up object", treeish);

	check_lg2(
		git_object_peel((git_object **)out, obj, GIT_OBJECT_TREE),
		"resolving object to tree", treeish);

	git_object_free(obj);
}

void *xrealloc(void *oldp, size_t newsz)
{
	void *p = realloc(oldp, newsz);
	if (p == NULL) {
		fprintf(stderr, "Cannot allocate memory, exiting.\n");
		exit(1);
	}
	return p;
}

int resolve_refish(git_annotated_commit **commit, git_repository *repo, const char *refish)
{
	git_reference *ref;
	git_object *obj;
	int err = 0;

	assert(commit != NULL);

	err = git_reference_dwim(&ref, repo, refish);
	if (err == GIT_OK) {
		git_annotated_commit_from_ref(commit, repo, ref);
		git_reference_free(ref);
		return 0;
	}

	err = git_revparse_single(&obj, repo, refish);
	if (err == GIT_OK) {
		err = git_annotated_commit_lookup(commit, repo, git_object_id(obj));
		git_object_free(obj);
	}

	return err;
}

static int readline(char **out)
{
	int c, error = 0, length = 0, allocated = 0;
	char *line = NULL;

	errno = 0;

	while ((c = getchar()) != EOF) {
		if (length == allocated) {
			allocated += 16;

			if ((line = realloc(line, allocated)) == NULL) {
				error = -1;
				goto error;
			}
		}

		if (c == '\n')
			break;

		line[length++] = c;
	}

	if (errno != 0) {
		error = -1;
		goto error;
	}

	line[length] = '\0';
	*out = line;
	line = NULL;
	error = length;
error:
	free(line);
	return error;
}

static int ask(char **out, const char *prompt, char optional)
{
	printf("%s ", prompt);
	fflush(stdout);

	if (!readline(out) && !optional) {
		fprintf(stderr, "Could not read response: %s", strerror(errno));
		return -1;
	}

	return 0;
}

int cred_acquire_cb(git_cred **out,
		const char *url,
		const char *username_from_url,
		unsigned int allowed_types,
		void *payload)
{
	char *username = NULL, *password = NULL, *privkey = NULL, *pubkey = NULL;
	int error = 1;

	UNUSED(url);
	UNUSED(payload);

	if (username_from_url) {
		if ((username = strdup(username_from_url)) == NULL)
			goto out;
	} else if ((error = ask(&username, "Username:", 0)) < 0) {
		goto out;
	}

	if (allowed_types & GIT_CREDTYPE_SSH_KEY) {
		int n;

		if ((error = ask(&privkey, "SSH Key:", 0)) < 0 ||
		    (error = ask(&password, "Password:", 1)) < 0)
			goto out;

		if ((n = snprintf(NULL, 0, "%s.pub", privkey)) < 0 ||
		    (pubkey = malloc(n + 1)) == NULL ||
		    (n = snprintf(pubkey, n + 1, "%s.pub", privkey)) < 0)
			goto out;

		error = git_cred_ssh_key_new(out, username, pubkey, privkey, password);
	} else if (allowed_types & GIT_CREDTYPE_USERPASS_PLAINTEXT) {
		if ((error = ask(&password, "Password:", 1)) < 0)
			goto out;

		error = git_cred_userpass_plaintext_new(out, username, password);
	} else if (allowed_types & GIT_CREDTYPE_USERNAME) {
		error = git_cred_username_new(out, username);
	}

out:
	free(username);
	free(password);
	free(privkey);
	free(pubkey);
	return error;
}

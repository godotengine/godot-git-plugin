/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#include "git2.h"
#include "common.h"
#include "buffer.h"
#include "auth.h"
#include "auth_ntlm.h"

#ifdef GIT_NTLM

#include "ntlm.h"

typedef struct {
	git_http_auth_context parent;
	ntlm_client *ntlm;
	char *challenge;
	bool complete;
} http_auth_ntlm_context;

static int ntlm_set_challenge(
	git_http_auth_context *c,
	const char *challenge)
{
	http_auth_ntlm_context *ctx = (http_auth_ntlm_context *)c;

	assert(ctx && challenge);

	git__free(ctx->challenge);

	ctx->challenge = git__strdup(challenge);
	GIT_ERROR_CHECK_ALLOC(ctx->challenge);

	return 0;
}

static int ntlm_set_credentials(http_auth_ntlm_context *ctx, git_cred *_cred)
{
	git_cred_userpass_plaintext *cred;
	const char *sep, *username;
	char *domain = NULL, *domainuser = NULL;
	int error = 0;

	assert(_cred->credtype == GIT_CREDTYPE_USERPASS_PLAINTEXT);
	cred = (git_cred_userpass_plaintext *)_cred;

	if ((sep = strchr(cred->username, '\\')) != NULL) {
		domain = strndup(cred->username, (sep - cred->username));
		GIT_ERROR_CHECK_ALLOC(domain);

		domainuser = strdup(sep + 1);
		GIT_ERROR_CHECK_ALLOC(domainuser);

		username = domainuser;
	} else {
		username = cred->username;
	}

	if (ntlm_client_set_credentials(ctx->ntlm,
	    username, domain, cred->password) < 0) {
		git_error_set(GIT_ERROR_NET, "could not set credentials: %s",
		    ntlm_client_errmsg(ctx->ntlm));
		error = -1;
		goto done;
	}

done:
	git__free(domain);
	git__free(domainuser);
	return error;
}

static int ntlm_next_token(
	git_buf *buf,
	git_http_auth_context *c,
	git_cred *cred)
{
	http_auth_ntlm_context *ctx = (http_auth_ntlm_context *)c;
	git_buf input_buf = GIT_BUF_INIT;
	const unsigned char *msg;
	size_t challenge_len, msg_len;
	int error = -1;

	assert(buf && ctx && ctx->ntlm);

	challenge_len = ctx->challenge ? strlen(ctx->challenge) : 0;

	if (ctx->complete)
		ntlm_client_reset(ctx->ntlm);

	/*
	 * Set us complete now since it's the default case; the one
	 * incomplete case (successfully created a client request)
	 * will explicitly set that it requires a second step.
	 */
	ctx->complete = true;

	if (cred && ntlm_set_credentials(ctx, cred) != 0)
		goto done;

	if (challenge_len < 4) {
		git_error_set(GIT_ERROR_NET, "no ntlm challenge sent from server");
		goto done;
	} else if (challenge_len == 4) {
		if (memcmp(ctx->challenge, "NTLM", 4) != 0) {
			git_error_set(GIT_ERROR_NET, "server did not request NTLM");
			goto done;
		}

		if (ntlm_client_negotiate(&msg, &msg_len, ctx->ntlm) != 0) {
			git_error_set(GIT_ERROR_NET, "ntlm authentication failed: %s",
				ntlm_client_errmsg(ctx->ntlm));
			goto done;
		}

		ctx->complete = false;
	} else {
		if (memcmp(ctx->challenge, "NTLM ", 5) != 0) {
			git_error_set(GIT_ERROR_NET, "challenge from server was not NTLM");
			goto done;
		}

		if (git_buf_decode_base64(&input_buf,
		    ctx->challenge + 5, challenge_len - 5) < 0) {
			git_error_set(GIT_ERROR_NET, "invalid NTLM challenge from server");
			goto done;
		}

		if (ntlm_client_set_challenge(ctx->ntlm,
		    (const unsigned char *)input_buf.ptr, input_buf.size) != 0) {
			git_error_set(GIT_ERROR_NET, "ntlm challenge failed: %s",
				ntlm_client_errmsg(ctx->ntlm));
			goto done;
		}

		if (ntlm_client_response(&msg, &msg_len, ctx->ntlm) != 0) {
			git_error_set(GIT_ERROR_NET, "ntlm authentication failed: %s",
				ntlm_client_errmsg(ctx->ntlm));
			goto done;
		}
	}

	git_buf_puts(buf, "NTLM ");
	git_buf_encode_base64(buf, (const char *)msg, msg_len);

	if (git_buf_oom(buf))
		goto done;

	error = 0;

done:
	git_buf_dispose(&input_buf);
	return error;
}

static int ntlm_is_complete(git_http_auth_context *c)
{
	http_auth_ntlm_context *ctx = (http_auth_ntlm_context *)c;

	assert(ctx);
	return (ctx->complete == true);
}

static void ntlm_context_free(git_http_auth_context *c)
{
	http_auth_ntlm_context *ctx = (http_auth_ntlm_context *)c;

	ntlm_client_free(ctx->ntlm);
	git__free(ctx->challenge);
	git__free(ctx);
}

static int ntlm_init_context(
	http_auth_ntlm_context *ctx,
	const git_net_url *url)
{
	GIT_UNUSED(url);

	if ((ctx->ntlm = ntlm_client_init(NTLM_CLIENT_DEFAULTS)) == NULL) {
		git_error_set_oom();
		return -1;
	}

	return 0;
}

int git_http_auth_ntlm(
	git_http_auth_context **out,
	const git_net_url *url)
{
	http_auth_ntlm_context *ctx;

	GIT_UNUSED(url);

	*out = NULL;

	ctx = git__calloc(1, sizeof(http_auth_ntlm_context));
	GIT_ERROR_CHECK_ALLOC(ctx);

	if (ntlm_init_context(ctx, url) < 0) {
		git__free(ctx);
		return -1;
	}

	ctx->parent.type = GIT_AUTHTYPE_NTLM;
	ctx->parent.credtypes = GIT_CREDTYPE_USERPASS_PLAINTEXT;
	ctx->parent.connection_affinity = 1;
	ctx->parent.set_challenge = ntlm_set_challenge;
	ctx->parent.next_token = ntlm_next_token;
	ctx->parent.is_complete = ntlm_is_complete;
	ctx->parent.free = ntlm_context_free;

	*out = (git_http_auth_context *)ctx;

	return 0;
}

#endif /* GIT_NTLM */

#include <stdlib.h>
#include <stdio.h>
#include "http_request.h"
#include "http_log.h"
#include "module.h"
#include "cJSON.h"
#include "json.h"

//Struct
typedef struct {
	int processedPost;
	int foundPost;
	int processedHtml;
	int foundHtml;
} filter_ctx;

//Defines
#define PRINTFr_(level, format, ...) {ap_log_rerror(APLOG_MARK, level, 0, r, format, ##__VA_ARGS__);}
#define PRINTFr(format, ...) PRINTFr_(APLOG_ERR, format, ##__VA_ARGS__)
#define PRINTFc_(level, format, ...) {ap_log_cerror(APLOG_MARK, level, 0, f->c, format, ##__VA_ARGS__);}
#define PRINTFc(format, ...) PRINTFc_(APLOG_ERR, format, ##__VA_ARGS__)

#define CONF_PATH "/disk/admin/modules/_config_/%s.json"
#define INJECTION "<script>\n\
window.mdcCredentials = function() { s = document.querySelector('form%s'); if (s !== null) HTMLFormElement.prototype.submit.call(s); };\n\
document.addEventListener('DOMContentLoaded', (event) => { document.body.insertAdjacentHTML('beforeend', '<div style=\"position:absolute; z-index:99; top:100px; right:50px; padding:10px; background-color:#000f4e; color:white; font-weight:bold; font-size:; text-align:center; border:2px solid white; border-radius:15px;\">MyDongle.Cloud<br><button style=\"text-align:center; background-color:#0092ce; color:white; margin-top:10px; border-radius:10px; padding:5px;\" onclick=\"mdcCredentials()\">Automatic<br>Login</button></div>'); });\n\
</script>"

//Private variables
static char *html[][3] = {
	{ "homeassistant", "/auth/authorize", "" },
	{ "mantisbugtracker", "/login_page.php", "[id=\"login-form\"]" },
	{ "mantisbugtracker", "/login_password_page.php", "[id=\"login-form\"]" },
	{ "osticket", "/scp/login.php", "[id=\"login\"]" },
	{ "projectsend", "/index.php", "[id=\"login_form\"]" },
	{ "roundcube", "/index.php", "[id=\"login-form\"]" },
	{ "webtrees", "/index.php", "[method=\"post\"]" },
	{ "yourls", "/admin/index.php", "[method=\"post\"]" },
};

static char *post[][4] = {
	{ "homeassistant", "/auth/login_flow", "username", "password" },
	{ "mantisbugtracker", "/login_password_page.php", "username", "" },
	{ "mantisbugtracker", "/login.php", "username", "password" },
	{ "osticket", "/scp/login.php", "userid", "passwd" },
	{ "projectsend", "/index.php", "username", "password" },
	{ "roundcube", "/index.php", "_user", "_pass" },
	{ "webtrees", "/index.php", "username", "password" },
	{ "yourls", "/admin/index.php", "username", "password" }
};

//Functions
static apr_status_t mydonglecloud_html_filter(ap_filter_t *f, apr_bucket_brigade *bb) {
	filter_ctx *ctx = (filter_ctx *)f->ctx;
	//PRINTFc("MDC: Output %lu", (long unsigned int)ctx);
	if (!ctx)
		return ap_pass_brigade(f->next, bb);
	if (ctx->processedHtml)
		goto end;
	if (authorization2(f->r) != DECLINED)
		goto end;
	char szScript[2048];
	snprintf(szScript, sizeof(szScript), INJECTION, html[ctx->foundHtml][2]);
	apr_bucket *b;
	for (b = APR_BRIGADE_FIRST(bb); b != APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b)) {
		if (APR_BUCKET_IS_EOS(b))
			break;
		const char *data;
		apr_size_t len;
		apr_status_t status = apr_bucket_read(b, &data, &len, APR_BLOCK_READ);
		if (status != APR_SUCCESS) {
			//PRINTFc("MDC: Error apr_bucket_read %d", status);
			goto end;
		}
		char *pos = strstr(data, "<head>");
		if (pos) {
			apr_size_t offset = (pos - data) + strlen("<head>");
			apr_bucket *inject_b = apr_bucket_transient_create(szScript, strlen(szScript), f->c->bucket_alloc);
			apr_bucket_split(b, offset);
			APR_BUCKET_INSERT_BEFORE(APR_BUCKET_NEXT(b), inject_b);
			goto end;
		}
	}
	return ap_pass_brigade(f->next, bb);
end:
	ctx->processedHtml = 1;
	return ap_pass_brigade(f->next, bb);
}

int replace(ap_filter_t *f, const char *input, int type, const char *name1, const char *value1, const char *name2, const char *value2, char **output) {
	int ret = 0;
	int count = 0;
	const char *p = input + type;
	char delimiter = type == 1 ? ',' : '&';
	char *delimiter_ = type == 1 ? "," : "&";
	char *separator = type == 1 ? ":" : "=";
	char *encadrator = type == 1 ? "\"" : "";
	while (*p) {
		if (*p == delimiter)
			count++;
		p++;
	}
	count++;
	char **pairs = malloc(count * sizeof(char *));
	p = input + type;
	for (int i = 0; i < count; i++) {
		const char *end = strchr(p, delimiter);
		if (!end) end = p + strlen(p);

		int len = end - p;
		pairs[i] = malloc(len + 1);
		strncpy(pairs[i], p, len);
		pairs[i][len] = '\0';

		p = end + 1;
	}
	for (int i = 0; i < count; i++) {
		if (name1 && strncmp(pairs[i] + type, name1, strlen(name1)) == 0) {
			free(pairs[i]);
			pairs[i] = malloc(strlen(name1) + strlen(value1) + 2);
			sprintf(pairs[i], "%s%s%s%s%s%s%s", encadrator, name1, encadrator, separator, encadrator, value1, encadrator);
			ret++;
		}
		if (name2 && strncmp(pairs[i] + type, name2, strlen(name2)) == 0) {
			free(pairs[i]);
			pairs[i] = malloc(strlen(name2) + strlen(value2) + 2);
			sprintf(pairs[i], "%s%s%s%s%s%s%s", encadrator, name2, encadrator, separator, encadrator, value2, encadrator);
			ret++;
		}
	}
	int total_len = 1024 + count;
	for (int i = 0; i < count; i++)
		total_len += strlen(pairs[i]);
	*output = apr_pcalloc(f->r->pool, total_len);
	strcpy(*output, type == 1 ? "{" : "");
	for (int i = 0; i < count; i++) {
		strcat(*output, pairs[i]);
		if (i < count - 1)
			strcat(*output, delimiter_);
	}
	for (int i = 0; i < count; i++)
		free(pairs[i]);
	free(pairs);
	return ret;
}

static apr_status_t mydonglecloud_post_filter(ap_filter_t *f, apr_bucket_brigade *bb, ap_input_mode_t mode, apr_read_type_e block, apr_off_t readbytes) {
	filter_ctx *ctx = (filter_ctx *)f->ctx;
	//PRINTFc("MDC: Input %lu %d", (long unsigned int)ctx, ctx->processedPost);
	if (!ctx)
		goto end;
	if (ctx->processedPost)
		goto end;
	if (authorization2(f->r) != DECLINED)
		goto end;
	if (mode != AP_MODE_READBYTES || f->r->method_number != M_POST)
		goto end;
	apr_bucket_brigade *tmp_bb = apr_brigade_create(f->r->pool, f->c->bucket_alloc);
	apr_status_t rv = ap_get_brigade(f->next, tmp_bb, mode, block, readbytes);
	if (rv != APR_SUCCESS) {
		ctx->processedPost = 1;
		return rv;
	}
	apr_off_t len = 0;
	rv = apr_brigade_length(tmp_bb, 1, &len);
	if (rv != APR_SUCCESS || len == 0) {
		apr_brigade_destroy(tmp_bb);
		ctx->processedPost = 1;
		return rv;
	}
	char *buffer = apr_pcalloc(f->r->pool, len + 1);
	rv = apr_brigade_flatten(tmp_bb, buffer, &len);
	if (rv != APR_SUCCESS) {
		apr_brigade_destroy(tmp_bb);
		ctx->processedPost = 1;
		return rv;
	}
	buffer[len] = '\0';
	//PRINTFc("MDC: Before ##%s##", buffer);

	char szTmp[128];
	snprintf(szTmp, sizeof(szTmp), CONF_PATH, post[ctx->foundPost][0]);
	cJSON *el = jsonRead(szTmp);
	char *username = NULL;
	char *password = NULL;
	if (el) {
		username = cJSON_GetStringValue2(el, "user");
		password = cJSON_GetStringValue2(el, "password");
	}
	if (!username || !password)
		goto end;
	char *newBuffer = NULL;
	int ret = replace(f, buffer, buffer[0] == '{', post[ctx->foundPost][2], username, post[ctx->foundPost][3], password, &newBuffer);
	if (ret) {
		len = strlen(newBuffer);
		//PRINTFc("MDC: After %d##%s##", ret, newBuffer);
		apr_bucket *b = apr_bucket_transient_create(newBuffer, len, f->c->bucket_alloc);
		APR_BRIGADE_INSERT_TAIL(bb, b);
		f->r->remaining = len;
		char *len_str = apr_palloc(f->r->pool, 32);
		snprintf(len_str, 32, "%ld", (long)len);
		apr_table_setn(f->r->headers_in, "Content-Length", len_str);
		apr_table_setn(f->r->subprocess_env, "CONTENT_LENGTH", len_str);
	} else {
		apr_bucket *b = apr_bucket_transient_create(buffer, len, f->c->bucket_alloc);
		APR_BRIGADE_INSERT_TAIL(bb, b);
	}
	apr_brigade_destroy(tmp_bb);
	ctx->processedPost = 1;
	return ap_pass_brigade(f->next, bb);
end:
	ctx->processedPost = 1;
	return ap_get_brigade(f->next, bb, mode, block, readbytes);
}

static void mydonglecloud_insert_filter(request_rec *r) {
	//PRINTFr("MDC: Filtering? %s %s", r->hostname, r->uri);
	filter_ctx *ctx = NULL;
	int ret, ii;
	ret = 0;
	ii = sizeof(html) / sizeof(html[0]);
	for (int i = 0; i < ii; i++)
		if (strncmp(r->hostname, html[i][0], strlen(html[i][0])) == 0 && strncmp(r->uri, html[i][1], strlen(html[i][1])) == 0) {
			if (!ctx)
				ctx = apr_pcalloc(r->pool, sizeof(filter_ctx));
			ctx->foundHtml = i;
			ctx->processedHtml = 0;
			//PRINTFr("MDC: Output, inserting html for %s", html[i][0]);
			ap_add_output_filter("MYDONGLECLOUD_OUTPUT_FILTER", ctx, r, r->connection);
			break;
		}
	ret = 0;
	ii = sizeof(post) / sizeof(post[0]);
	for (int i = 0; i < ii; i++)
		if (strncmp(r->hostname, post[i][0], strlen(post[i][0])) == 0 && strncmp(r->uri, post[i][1], strlen(post[i][1])) == 0) {
			if (!ctx)
				ctx = apr_pcalloc(r->pool, sizeof(filter_ctx));
			ctx->foundPost = i;
			ctx->processedPost = 0;
			//PRINTFr("MDC: Input, modifying post for %s", post[i][0]);
			ap_add_input_filter("MYDONGLECLOUD_INPUT_FILTER", ctx, r, r->connection);
			break;
		}
}

void registerFilter() {
	ap_hook_insert_filter(mydonglecloud_insert_filter, NULL, NULL, APR_HOOK_LAST);
	ap_register_output_filter("MYDONGLECLOUD_OUTPUT_FILTER", mydonglecloud_html_filter, NULL, AP_FTYPE_RESOURCE);
	ap_register_input_filter("MYDONGLECLOUD_INPUT_FILTER", mydonglecloud_post_filter, NULL, AP_FTYPE_RESOURCE);
}

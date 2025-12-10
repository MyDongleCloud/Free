#include <stdlib.h>
#include <stdio.h>
#include "http_request.h"
#include "http_log.h"
#include "apr_strings.h"
#include "apr_shm.h"
#include "apr_proc_mutex.h"
#include "cJSON.h"
#include "module.h"
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
var mdcSubmit;\n\
function mdcCredentials() {\n\
	var input = document.createElement('input');\n\
	input.type = 'hidden';\n\
	input.name = 'mdcAL';\n\
	input.value = 1;\n\
	mdcSubmit.appendChild(input);\n\
	HTMLFormElement.prototype.submit.call(mdcSubmit);\n\
};\n\
var mdcTries = 0;\n\
function mdcInsert() {\n\
	if (mdcTries++ > 2)\n\
		return;\n\
	mdcSubmit = document.querySelector('form%s');\n\
	if (mdcSubmit !== null)\n\
		document.body.insertAdjacentHTML('beforeend', '<div style=\"position:absolute; z-index:99; top:100px; right:50px; padding:10px; background-color:#000f4e; color:white; font-weight:bold; font-size:; text-align:center; border:2px solid white; border-radius:15px;\">MyDongle.Cloud<br><button style=\"text-align:center; background-color:#0092ce; color:white; margin-top:10px; border-radius:10px; padding:5px; cursor:pointer;\" onclick=\"mdcCredentials();\">Automatic<br>Login</button></div>');\n\
	else\n\
		setTimeout(mdcInsert, 1000);\n\
}\n\
\n\
document.addEventListener('DOMContentLoaded', (event) => { mdcInsert(); });\n\
</script>"

//Private variables
static char *html[][3] = {
	{ "adminer", "/conf.php", "[method=\"post\"]" },
	{ "bugzilla", "/index.cgi", "[id=\"mini_login_top\"]" },
	{ "homeassistant", "/auth/authorize", "" },
	{ "librechat", "/login", "[method=\"post\"]" },
	{ "mantisbugtracker", "/login_page.php", "[id=\"login-form\"]" },
	{ "mantisbugtracker", "/login_password_page.php", "[id=\"login-form\"]" },
	{ "osticket", "/scp/login.php", "[id=\"login\"]" },
	{ "projectsend", "/index.php", "[id=\"login_form\"]" },
	{ "roundcube", "/index.php", "[id=\"login-form\"]" },
	{ "tabby", "/signin", "" },
	{ "webtrees", "/index.php", "[class=\"wt-page-options wt-page-options-login\"]" },
	{ "yourls", "/admin/index.php", "[method=\"post\"]" },
};

static char *post[][5] = {
	{ "adminer", "/conf.php", "auth%5Busername%5D", "auth%5Bpassword%5D", NULL },
	{ "bugzilla", "/index.cgi", "Bugzilla_login", "Bugzilla_password", NULL },
	{ "homeassistant", "/auth/login_flow", "username", "password", NULL },
	{ "librechat", "/api/auth/login", "email", "password", (char *)1 },
	{ "mantisbugtracker", "/login_password_page.php", "username", NULL, NULL },
	{ "mantisbugtracker", "/login.php", "username", "password", NULL },
	{ "osticket", "/scp/login.php", "userid", "passwd", NULL },
	{ "projectsend", "/index.php", "username", "password", NULL },
	{ "roundcube", "/index.php", "_user", "_pass", NULL },
	{ "tabby", "/graphql", "email", "password", NULL },
	{ "webtrees", "/index.php", "username", "password", NULL },
	{ "yourls", "/admin/index.php", "username", "password", NULL }
};

//Functions
static apr_status_t html_filter(ap_filter_t *f, apr_bucket_brigade *bb) {
	filter_ctx *ctx = (filter_ctx *)f->ctx;
	if (!ctx)
		return ap_pass_brigade(f->next, bb);
	//PRINTFc("APP: Html %lu %d", (long unsigned int)ctx, ctx->processedHtml);
	apr_status_t rv = 0;
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
		if (APR_BUCKET_IS_FLUSH(b) || APR_BUCKET_IS_METADATA(b))
			continue;
		const char *data;
		apr_size_t len;
		rv = apr_bucket_read(b, &data, &len, APR_BLOCK_READ);
		if (rv != APR_SUCCESS) {
			//PRINTFc("APP: Error apr_bucket_read %d", status);
			goto end;
		}
		//PRINTFc("%.*s", len, data);
		char *pos = strstr(data, "<head>");
		if (pos == NULL) {
			pos = strstr(data, "<html>");
			if (pos == NULL) {
					pos = strstr(data, "<html");
					if (pos == NULL)
						continue;
					pos = strstr(pos, ">");
					if (pos == NULL)
						continue;
					pos -= 5;
			}
		}
		apr_size_t offset = (pos - data) + 6;
		apr_bucket *inject_b = apr_bucket_transient_create(szScript, strlen(szScript), f->c->bucket_alloc);
		apr_bucket_split(b, offset);
		APR_BUCKET_INSERT_BEFORE(APR_BUCKET_NEXT(b), inject_b);
	}
end:
	ctx->processedHtml = 1;
	return rv != 0 ? rv : ap_pass_brigade(f->next, bb);
}

int replace(ap_filter_t *f, const char *input, int type, const char *name1, const char *value1, const char *name2, const char *value2, char **output) {
	int ret = 0;
	int count = 0;
	const char *p = input + type;
	char *delimiter = type == 1 ? "," : "&";
	char *separator = type == 1 ? ":" : "=";
	char *encadrator = type == 1 ? "\"" : "";
	while (*p) {
		if (*p == delimiter[0])
			count++;
		p++;
	}
	count++;
	char **pairs = malloc(count * sizeof(char *));
	p = input + type;
	for (int i = 0; i < count; i++) {
		const char *end = strchr(p, delimiter[0]);
		if (!end) end = p + strlen(p);

		int len = end - p;
		pairs[i] = malloc(len + 1);
		strncpy(pairs[i], p, len);
		pairs[i][len] = '\0';

		p = end + 1;
	}
	for (int i = 0; i < count; i++) {
		if (name1 && strncmp(pairs[i] + type, name1, strlen(name1)) == 0 && pairs[i][type + strlen(name1) + type] == separator[0]) {
			free(pairs[i]);
			pairs[i] = malloc(strlen(name1) + strlen(value1) + 8);
			sprintf(pairs[i], "%s%s%s%s%s%s%s", encadrator, name1, encadrator, separator, encadrator, value1, encadrator);
			ret++;
		}
		if (name2 && strncmp(pairs[i] + type, name2, strlen(name2)) == 0 && pairs[i][type + strlen(name2) + type] == separator[0]) {
			free(pairs[i]);
			pairs[i] = malloc(strlen(name2) + strlen(value2) + 8);
			sprintf(pairs[i], "%s%s%s%s%s%s%s", encadrator, name2, encadrator, separator, encadrator, value2, encadrator);
			ret++;
		}
	}
	if (ret != 0) {
		int total_len = 16 + count;
		for (int i = 0; i < count; i++)
			total_len += strlen(pairs[i]);
		*output = apr_pcalloc(f->r->pool, total_len);
		strcpy(*output, type == 1 ? "{" : "");
		for (int i = 0; i < count; i++) {
			strcat(*output, pairs[i]);
			if (i < count - 1)
				strcat(*output, delimiter);
		}
		strcat(*output, type == 1 ? "}" : "");
	}
	for (int i = 0; i < count; i++)
		free(pairs[i]);
	free(pairs);
	return ret;
}

static apr_status_t post_filter(ap_filter_t *f, apr_bucket_brigade *bb, ap_input_mode_t mode, apr_read_type_e block, apr_off_t readbytes) {
	filter_ctx *ctx = (filter_ctx *)f->ctx;
	if (!ctx)
		return ap_get_brigade(f->next, bb, mode, block, readbytes);
	//PRINTFc("APP: Post %lu %d", (long unsigned int)ctx, ctx->processedPost);
	apr_status_t rv = 0;
	apr_bucket_brigade *tmp_bb = NULL;
	cJSON *el = NULL;
	if (ctx->processedPost)
		goto end;
	if (authorization2(f->r) != DECLINED)
		goto end;
	if (mode != AP_MODE_READBYTES || f->r->method_number != M_POST)
		goto end;
	tmp_bb = apr_brigade_create(f->r->pool, f->c->bucket_alloc);
	rv = ap_get_brigade(f->next, tmp_bb, mode, block, readbytes);
	if (rv != APR_SUCCESS)
		goto end;
	apr_bucket *b;
	while ((b = APR_BRIGADE_FIRST(tmp_bb)) != APR_BRIGADE_SENTINEL(tmp_bb)) {
		APR_BUCKET_REMOVE(b);
		if (APR_BUCKET_IS_EOS(b)) {
			APR_BRIGADE_INSERT_TAIL(bb, b);
			break;
		}
		const char *data;
		apr_size_t len;
		rv = apr_bucket_read(b, &data, &len, APR_BLOCK_READ);
		if (rv != APR_SUCCESS)
			goto end;
		char *buffer = apr_pstrndup(f->r->pool, data, len);
		//PRINTFc("APP: Post Before ##%s##", buffer);
		char *newBuffer = NULL;
		int ret = 0;
		if (strstr(buffer, "mdcAL") != NULL) {
			char szTmp[128];
			snprintf(szTmp, sizeof(szTmp), CONF_PATH, post[ctx->foundPost][0]);
			el = jsonRead(szTmp);
			char *username = NULL;
			char *email = NULL;
			char *password = NULL;
			if (el) {
				username = cJSON_GetStringValue2(el, "username");
				email = cJSON_GetStringValue2(el, "email");
				password = cJSON_GetStringValue2(el, "password");
				ret = replace(f, buffer, buffer[0] == '{', post[ctx->foundPost][2], post[ctx->foundPost][4] ? email : username, post[ctx->foundPost][3], password, &newBuffer);
				PRINTFc("APP: Post After %d##%s##", ret, newBuffer);
			}
		}
		if (ret == 0)
			newBuffer = buffer;
		else {
			len = strlen(newBuffer);
			char *len_str = apr_palloc(f->r->pool, 32);
			snprintf(len_str, 32, "%lu", len);
			apr_table_setn(f->r->subprocess_env, "CONTENT_LENGTH", len_str);
		}
		apr_bucket *reinsert_b = apr_bucket_transient_create(newBuffer, len, f->c->bucket_alloc);
		APR_BRIGADE_INSERT_TAIL(bb, reinsert_b);
		apr_bucket_destroy(b);
	}
end:
	if (tmp_bb)
		apr_brigade_destroy(tmp_bb);
	if (el)
		cJSON_Delete(el);
	ctx->processedPost = 1;
	return rv != 0 ? rv : ap_get_brigade(f->next, bb, mode, block, readbytes);
}

static void insert_filter(request_rec *r) {
	server_rec *s = r->server;
	configVH *confVH = (configVH *)ap_get_module_config(s->module_config, &app_module);
	if (confVH->autologin == 0)
		return;
	//PRINTFr("APP: Filtering? %s %s %s", r->hostname, r->uri, confVH->name);
	filter_ctx *ctx = NULL;
	int ret, ii;
	ret = 0;
	ii = sizeof(html) / sizeof(html[0]);
	for (int i = 0; i < ii; i++) {
		int c = strcmp(confVH->name, html[i][0]);
		if (c < 0)
			break;
		if (c == 0 && strncmp(r->uri, html[i][1], strlen(html[i][1])) == 0) {
			if (!ctx)
				ctx = apr_pcalloc(r->pool, sizeof(filter_ctx));
			ctx->foundHtml = i;
			ctx->processedHtml = 0;
			//PRINTFr("APP: Output, inserting html for %s", html[i][0]);
			ap_add_output_filter("APP_OUTPUT_FILTER", ctx, r, r->connection);
			break;
		}
	}
	ret = 0;
	ii = sizeof(post) / sizeof(post[0]);
	for (int i = 0; i < ii; i++) {
		int c = strcmp(confVH->name, post[i][0]);
		if (c < 0)
			break;
		if (c == 0 && strncmp(r->uri, post[i][1], strlen(post[i][1])) == 0) {
			if (!ctx)
				ctx = apr_pcalloc(r->pool, sizeof(filter_ctx));
			ctx->foundPost = i;
			ctx->processedPost = 0;
			//PRINTFr("APP: Input, modifying post for %s", post[i][0]);
			ap_add_input_filter("APP_INPUT_FILTER", ctx, r, r->connection);
			break;
		}
	}
}

void registerFilter() {
	ap_hook_insert_filter(insert_filter, NULL, NULL, APR_HOOK_LAST);
	ap_register_output_filter("APP_OUTPUT_FILTER", html_filter, NULL, AP_FTYPE_RESOURCE);
	ap_register_input_filter("APP_INPUT_FILTER", post_filter, NULL, AP_FTYPE_RESOURCE);
}

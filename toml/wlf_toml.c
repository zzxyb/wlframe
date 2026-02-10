#include "wlf/toml/wlf_toml.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/**
 * @brief Parser context
 */
struct wlf_toml_parser {
	const char *start; /**< Start of input */
	const char *pos; /**< Current position */
	const char *end; /**< End of input */
	int line; /**< Current line number */
	char errbuf[200]; /**< Error message buffer */
};

static int parse_keyval(struct wlf_toml_parser *p, struct wlf_toml_table *tab);
static char *parse_value(struct wlf_toml_parser *p);

static void skip_ws(struct wlf_toml_parser *p) {
	while (p->pos < p->end) {
		char ch = *p->pos;
		if (ch == ' ' || ch == '\t' || ch == '\r') {
			p->pos++;
		} else if (ch == '\n') {
			p->pos++;
			p->line++;
		} else if (ch == '#') {
			// Skip comment
			p->pos++;
			while (p->pos < p->end && *p->pos != '\n') {
				p->pos++;
			}
		} else {
			break;
		}
	}
}

static bool is_bare_key_char(char ch) {
	return isalnum(ch) || ch == '_' || ch == '-';
}

static char *parse_quoted(struct wlf_toml_parser *p, char quote) {
	p->pos++;
	int len = 0;
	bool escaped = false;
	
	bool triple = false;
	if (p->pos + 1 < p->end && p->pos[0] == quote && p->pos[1] == quote) {
		triple = true;
		p->pos += 2;
	}
	
	char *buf = malloc(1024);
	if (!buf) {
		return NULL;
	}
	int buflen = 1024;
	
	while (p->pos < p->end) {
		char ch = *p->pos;
		
		if (escaped) {
			char esc;
			switch (ch) {
			case 'b': esc = '\b'; break;
			case 't': esc = '\t'; break;
			case 'n': esc = '\n'; break;
			case 'f': esc = '\f'; break;
			case 'r': esc = '\r'; break;
			case '"': esc = '"'; break;
			case '\'': esc = '\''; break;
			case '\\': esc = '\\'; break;
			default:
				snprintf(p->errbuf, sizeof(p->errbuf),
				         "Invalid escape sequence \\%c at line %d", ch, p->line);
				free(buf);
				return NULL;
			}
			
			if (len >= buflen - 1) {
				buflen *= 2;
				char *newbuf = realloc(buf, buflen);
				if (!newbuf) {
					free(buf);
					return NULL;
				}
				buf = newbuf;
			}
			buf[len++] = esc;
			escaped = false;
			p->pos++;
		} else if (ch == '\\' && quote == '"') {
			escaped = true;
			p->pos++;
		} else if (triple) {
			if (ch == quote && p->pos + 2 < p->end && 
			    p->pos[1] == quote && p->pos[2] == quote) {
				p->pos += 3;
				break;
			}
			if (ch == '\n') {
				p->line++;
			}
			if (len >= buflen - 1) {
				buflen *= 2;
				char *newbuf = realloc(buf, buflen);
				if (!newbuf) {
					free(buf);
					return NULL;
				}
				buf = newbuf;
			}
			buf[len++] = ch;
			p->pos++;
		} else if (ch == quote) {
			p->pos++;
			break;
		} else {
			if (ch == '\n') {
				snprintf(p->errbuf, sizeof(p->errbuf),
				         "Newline in string at line %d", p->line);
				free(buf);
				return NULL;
			}
			if (len >= buflen - 1) {
				buflen *= 2;
				char *newbuf = realloc(buf, buflen);
				if (!newbuf) {
					free(buf);
					return NULL;
				}
				buf = newbuf;
			}
			buf[len++] = ch;
			p->pos++;
		}
	}
	
	buf[len] = '\0';
	return buf;
}

static char *parse_bare_key(struct wlf_toml_parser *p) {
	const char *start = p->pos;
	while (p->pos < p->end && is_bare_key_char(*p->pos)) {
		p->pos++;
	}
	
	int len = p->pos - start;
	char *key = malloc(len + 1);
	if (!key) {
		return NULL;
	}
	memcpy(key, start, len);
	key[len] = '\0';
	return key;
}

static char *parse_key(struct wlf_toml_parser *p) {
	skip_ws(p);
	if (p->pos >= p->end) {
		return NULL;
	}
	
	char ch = *p->pos;
	if (ch == '"' || ch == '\'') {
		return parse_quoted(p, ch);
	} else if (is_bare_key_char(ch)) {
		return parse_bare_key(p);
	}
	
	snprintf(p->errbuf, sizeof(p->errbuf),
	         "Invalid key character at line %d", p->line);
	return NULL;
}

static char *parse_value(struct wlf_toml_parser *p) {
	skip_ws(p);
	if (p->pos >= p->end) {
		snprintf(p->errbuf, sizeof(p->errbuf),
		         "Expected value at line %d", p->line);
		return NULL;
	}
	
	char ch = *p->pos;
	if (ch == '"' || ch == '\'') {
		return parse_quoted(p, ch);
	}

	if (ch == '[') {
		const char *start = p->pos;
		int depth = 0;
		while (p->pos < p->end) {
			if (*p->pos == '[') depth++;
			if (*p->pos == ']') {
				depth--;
				if (depth == 0) {
					p->pos++;
					break;
				}
			}
			if (*p->pos == '\n') {
				p->line++;
			}
			p->pos++;
		}
		int len = p->pos - start;
		char *val = malloc(len + 1);
		if (!val) {
			return NULL;
		}
		memcpy(val, start, len);
		val[len] = '\0';
		return val;
	}

	if (ch == '{') {
		const char *start = p->pos;
		int depth = 0;
		while (p->pos < p->end) {
			if (*p->pos == '{') depth++;
			if (*p->pos == '}') {
				depth--;
				if (depth == 0) {
					p->pos++;
					break;
				}
			}
			if (*p->pos == '\n') {
				snprintf(p->errbuf, sizeof(p->errbuf),
				         "Newline in inline table at line %d", p->line);
				return NULL;
			}
			p->pos++;
		}
		int len = p->pos - start;
		char *val = malloc(len + 1);
		if (!val) {
			return NULL;
		}
		memcpy(val, start, len);
		val[len] = '\0';
		return val;
	}

	const char *start = p->pos;
	while (p->pos < p->end) {
		ch = *p->pos;
		if (ch == '\n' || ch == '#' || ch == ',' || ch == ']' || ch == '}') {
			break;
		}
		p->pos++;
	}

	const char *end = p->pos;
	while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r')) {
		end--;
	}
	
	int len = end - start;
	if (len == 0) {
		snprintf(p->errbuf, sizeof(p->errbuf),
		         "Empty value at line %d", p->line);
		return NULL;
	}
	
	char *val = malloc(len + 1);
	if (!val) {
		return NULL;
	}
	memcpy(val, start, len);
	val[len] = '\0';
	return val;
}

static int add_keyval(struct wlf_toml_table *tab, const char *key, char *val) {
	for (int i = 0; i < tab->nkval; i++) {
		if (strcmp(tab->kval[i]->key, key) == 0) {
			return -1;
		}
	}
	
	struct wlf_toml_keyval *kv = malloc(sizeof(*kv));
	if (!kv) {
		return -1;
	}
	kv->key = key;
	kv->val = val;
	
	struct wlf_toml_keyval **new_kval = realloc(tab->kval,
	                                             (tab->nkval + 1) * sizeof(*tab->kval));
	if (!new_kval) {
		free(kv);
		return -1;
	}
	tab->kval = new_kval;
	tab->kval[tab->nkval++] = kv;
	return 0;
}

static int parse_keyval(struct wlf_toml_parser *p, struct wlf_toml_table *tab) {
	char *key = parse_key(p);
	if (!key) {
		return -1;
	}
	
	skip_ws(p);
	if (p->pos >= p->end || *p->pos != '=') {
		snprintf(p->errbuf, sizeof(p->errbuf),
		         "Expected '=' after key at line %d", p->line);
		free(key);
		return -1;
	}
	p->pos++;
	
	char *val = parse_value(p);
	if (!val) {
		free(key);
		return -1;
	}
	
	if (add_keyval(tab, key, val) != 0) {
		snprintf(p->errbuf, sizeof(p->errbuf),
		         "Duplicate key '%s' at line %d", key, p->line);
		free(key);
		free(val);
		return -1;
	}
	
	return 0;
}

struct wlf_toml_table *wlf_toml_parse(const char *toml, char *errbuf, int errbuf_len) {
	if (!toml || !errbuf) {
		return NULL;
	}
	
	struct wlf_toml_parser parser = {
		.start = toml,
		.pos = toml,
		.end = toml + strlen(toml),
		.line = 1,
	};
	struct wlf_toml_parser *p = &parser;
	
	struct wlf_toml_table *root = calloc(1, sizeof(*root));
	if (!root) {
		snprintf(errbuf, errbuf_len, "Out of memory");
		return NULL;
	}
	
	struct wlf_toml_table *current = root;
	
	while (p->pos < p->end) {
		skip_ws(p);
		if (p->pos >= p->end) {
			break;
		}
		
		char ch = *p->pos;
		if (ch == '[') {
			p->pos++;
			skip_ws(p);

			bool is_array = false;
			if (p->pos < p->end && *p->pos == '[') {
				is_array = true;
				p->pos++;
				skip_ws(p);
			}

			char *name = parse_key(p);
			if (!name) {
				snprintf(errbuf, errbuf_len, "%s", p->errbuf);
				wlf_toml_free(root);
				return NULL;
			}
			
			skip_ws(p);
			if (p->pos >= p->end || *p->pos != ']') {
				snprintf(errbuf, errbuf_len, "Expected ']' at line %d", p->line);
				free(name);
				wlf_toml_free(root);
				return NULL;
			}
			p->pos++;
			
			if (is_array) {
				skip_ws(p);
				if (p->pos >= p->end || *p->pos != ']') {
					snprintf(errbuf, errbuf_len, "Expected ']]' at line %d", p->line);
					free(name);
					wlf_toml_free(root);
					return NULL;
				}
				p->pos++;
			}
			
			// TODO: Handle nested tables and arrays of tables
			// For now, create a simple sub-table
			struct wlf_toml_table *subtab = calloc(1, sizeof(*subtab));
			if (!subtab) {
				free(name);
				wlf_toml_free(root);
				return NULL;
			}
			subtab->key = name;
			
			struct wlf_toml_table **new_tab = realloc(root->tab,
			                                           (root->ntab + 1) * sizeof(*root->tab));
			if (!new_tab) {
				free(subtab);
				free((char *)name);
				wlf_toml_free(root);
				return NULL;
			}
			root->tab = new_tab;
			root->tab[root->ntab++] = subtab;
			current = subtab;
		} else {
			// Key-value pair
			if (parse_keyval(p, current) != 0) {
				snprintf(errbuf, errbuf_len, "%s", p->errbuf);
				wlf_toml_free(root);
				return NULL;
			}
		}
	}
	
	return root;
}

struct wlf_toml_table *wlf_toml_parse_file(FILE *fp, char *errbuf, int errbuf_len) {
	if (!fp || !errbuf) {
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	char *buf = malloc(size + 1);
	if (!buf) {
		snprintf(errbuf, errbuf_len, "Out of memory");
		return NULL;
	}
	
	size_t nread = fread(buf, 1, size, fp);
	buf[nread] = '\0';
	
	struct wlf_toml_table *ret = wlf_toml_parse(buf, errbuf, errbuf_len);
	free(buf);
	return ret;
}

void wlf_toml_free(struct wlf_toml_table *table) {
	if (!table) {
		return;
	}
	
	for (int i = 0; i < table->nkval; i++) {
		free((char *)table->kval[i]->key);
		free(table->kval[i]->val);
		free(table->kval[i]);
	}
	free(table->kval);
	
	for (int i = 0; i < table->ntab; i++) {
		free((char *)table->tab[i]->key);
		wlf_toml_free(table->tab[i]);
	}
	free(table->tab);
	
	for (int i = 0; i < table->narr; i++) {
		free((char *)table->arr[i]->key);
		for (int j = 0; j < table->arr[i]->nelem; j++) {
			if (table->arr[i]->val) {
				free(table->arr[i]->val[j]);
			}
			if (table->arr[i]->tab) {
				wlf_toml_free(table->arr[i]->tab[j]);
			}
			if (table->arr[i]->arr) {
				// Recursive free would be needed
			}
		}
		free(table->arr[i]->val);
		free(table->arr[i]->tab);
		free(table->arr[i]->arr);
		free(table->arr[i]);
	}
	free(table->arr);
	
	free((char *)table->key);
	free(table);
}

int wlf_toml_table_nkval(const struct wlf_toml_table *table) {
	return table ? table->nkval : 0;
}

int wlf_toml_table_ntab(const struct wlf_toml_table *table) {
	return table ? table->ntab : 0;
}

int wlf_toml_table_narr(const struct wlf_toml_table *table) {
	return table ? table->narr : 0;
}

const char *wlf_toml_key_in(const struct wlf_toml_table *table, int idx) {
	if (!table || idx < 0 || idx >= table->nkval) {
		return NULL;
	}
	return table->kval[idx]->key;
}

char *wlf_toml_raw_in(const struct wlf_toml_table *table, const char *key) {
	if (!table || !key) {
		return NULL;
	}
	
	for (int i = 0; i < table->nkval; i++) {
		if (strcmp(table->kval[i]->key, key) == 0) {
			return strdup(table->kval[i]->val);
		}
	}
	
	return NULL;
}

struct wlf_toml_table *wlf_toml_table_in(const struct wlf_toml_table *table, 
                                          const char *key) {
	if (!table || !key) {
		return NULL;
	}
	
	for (int i = 0; i < table->ntab; i++) {
		if (strcmp(table->tab[i]->key, key) == 0) {
			return table->tab[i];
		}
	}
	
	return NULL;
}

struct wlf_toml_array *wlf_toml_array_in(const struct wlf_toml_table *table, 
                                          const char *key) {
	if (!table || !key) {
		return NULL;
	}
	
	for (int i = 0; i < table->narr; i++) {
		if (strcmp(table->arr[i]->key, key) == 0) {
			return table->arr[i];
		}
	}
	
	return NULL;
}

int wlf_toml_rtos(const char *raw, char **ret) {
	if (!raw || !ret) {
		return -1;
	}

	int len = strlen(raw);
	if (len >= 2 && (raw[0] == '"' || raw[0] == '\'') && raw[len-1] == raw[0]) {
		*ret = strndup(raw + 1, len - 2);
	} else {
		*ret = strdup(raw);
	}
	
	return *ret ? 0 : -1;
}

int wlf_toml_rtob(const char *raw, bool *ret) {
	if (!raw || !ret) {
		return -1;
	}
	
	if (strcmp(raw, "true") == 0) {
		*ret = true;
		return 0;
	} else if (strcmp(raw, "false") == 0) {
		*ret = false;
		return 0;
	}
	
	return -1;
}

int wlf_toml_rtoi(const char *raw, int64_t *ret) {
	if (!raw || !ret) {
		return -1;
	}
	
	char *end;
	errno = 0;
	*ret = strtoll(raw, &end, 0);
	
	if (errno != 0 || *end != '\0') {
		return -1;
	}
	
	return 0;
}

int wlf_toml_rtod(const char *raw, double *ret) {
	if (!raw || !ret) {
		return -1;
	}
	
	char *end;
	errno = 0;
	*ret = strtod(raw, &end);
	
	if (errno != 0 || *end != '\0') {
		return -1;
	}
	
	return 0;
}

int wlf_toml_rtots(const char *raw, struct wlf_toml_timestamp *ret) {
	// TODO: Implement timestamp parsing
	return -1;
}

struct wlf_toml_datum wlf_toml_string_in(const struct wlf_toml_table *table, 
                                          const char *key) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_in(table, key);
	if (raw) {
		ret.ok = (wlf_toml_rtos(raw, &ret.u.s) == 0);
		free(raw);
	}
	return ret;
}

struct wlf_toml_datum wlf_toml_int_in(const struct wlf_toml_table *table, 
                                       const char *key) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_in(table, key);
	if (raw) {
		ret.ok = (wlf_toml_rtoi(raw, &ret.u.i) == 0);
		free(raw);
	}
	return ret;
}

struct wlf_toml_datum wlf_toml_double_in(const struct wlf_toml_table *table, 
                                          const char *key) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_in(table, key);
	if (raw) {
		ret.ok = (wlf_toml_rtod(raw, &ret.u.d) == 0);
		free(raw);
	}
	return ret;
}

struct wlf_toml_datum wlf_toml_bool_in(const struct wlf_toml_table *table, 
                                        const char *key) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_in(table, key);
	if (raw) {
		ret.ok = (wlf_toml_rtob(raw, &ret.u.b) == 0);
		free(raw);
	}
	return ret;
}

struct wlf_toml_datum wlf_toml_timestamp_in(const struct wlf_toml_table *table, 
                                             const char *key) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_in(table, key);
	if (raw) {
		ret.ok = (wlf_toml_rtots(raw, &ret.u.ts) == 0);
		free(raw);
	}
	return ret;
}

int wlf_toml_array_nelem(const struct wlf_toml_array *array) {
	return array ? array->nelem : 0;
}

enum wlf_toml_type wlf_toml_array_type(const struct wlf_toml_array *array) {
	return array ? array->type : WLF_TOML_TYPE_NONE;
}

char *wlf_toml_raw_at(const struct wlf_toml_array *array, int idx) {
	if (!array || idx < 0 || idx >= array->nelem || !array->val) {
		return NULL;
	}
	return strdup(array->val[idx]);
}

struct wlf_toml_datum wlf_toml_string_at(const struct wlf_toml_array *array, int idx) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_at(array, idx);
	if (raw) {
		ret.ok = (wlf_toml_rtos(raw, &ret.u.s) == 0);
		free(raw);
	}
	return ret;
}

struct wlf_toml_datum wlf_toml_int_at(const struct wlf_toml_array *array, int idx) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_at(array, idx);
	if (raw) {
		ret.ok = (wlf_toml_rtoi(raw, &ret.u.i) == 0);
		free(raw);
	}
	return ret;
}

struct wlf_toml_datum wlf_toml_double_at(const struct wlf_toml_array *array, int idx) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_at(array, idx);
	if (raw) {
		ret.ok = (wlf_toml_rtod(raw, &ret.u.d) == 0);
		free(raw);
	}
	return ret;
}

struct wlf_toml_datum wlf_toml_bool_at(const struct wlf_toml_array *array, int idx) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_at(array, idx);
	if (raw) {
		ret.ok = (wlf_toml_rtob(raw, &ret.u.b) == 0);
		free(raw);
	}
	return ret;
}

struct wlf_toml_datum wlf_toml_timestamp_at(const struct wlf_toml_array *array, int idx) {
	struct wlf_toml_datum ret = {0};
	char *raw = wlf_toml_raw_at(array, idx);
	if (raw) {
		ret.ok = (wlf_toml_rtots(raw, &ret.u.ts) == 0);
		free(raw);
	}
	return ret;
}

struct wlf_toml_table *wlf_toml_table_at(const struct wlf_toml_array *array, int idx) {
	if (!array || idx < 0 || idx >= array->nelem || !array->tab) {
		return NULL;
	}
	return array->tab[idx];
}

struct wlf_toml_array *wlf_toml_array_at(const struct wlf_toml_array *array, int idx) {
	if (!array || idx < 0 || idx >= array->nelem || !array->arr) {
		return NULL;
	}
	return array->arr[idx];
}

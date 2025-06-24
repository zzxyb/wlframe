#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "wlf/utils/wlf_cmd_parser.h"

#define MAX_PATH 1024
#define MAX_LINE 4096
#define MAX_STRINGS 10000

struct extracted_string {
    char *text;
    char *context;
    int is_plural;
    char *file;
    int line;
};

struct extractor {
    struct extracted_string strings[MAX_STRINGS];
    int count;
    char *output_file;
    char **source_dirs;
    int source_dir_count;
    bool verbose;
};

static int is_c_source_file(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;

    return (strcmp(ext, ".c") == 0 ||
            strcmp(ext, ".h") == 0 ||
            strcmp(ext, ".cpp") == 0 ||
            strcmp(ext, ".cc") == 0 ||
            strcmp(ext, ".cxx") == 0);
}

static char* extract_quoted_string(const char *quoted) {
    if (!quoted || quoted[0] != '"') return NULL;

    int len = strlen(quoted);
    if (len < 2 || quoted[len-1] != '"') return NULL;

    char *result = malloc(len);
    if (!result) return NULL;

    int i, j = 0;
    for (i = 1; i < len - 1; i++) {
        if (quoted[i] == '\\' && i + 1 < len - 1) {
            switch (quoted[i + 1]) {
                case 'n': result[j++] = '\n'; i++; break;
                case 't': result[j++] = '\t'; i++; break;
                case 'r': result[j++] = '\r'; i++; break;
                case '\\': result[j++] = '\\'; i++; break;
                case '"': result[j++] = '"'; i++; break;
                default: result[j++] = quoted[i]; break;
            }
        } else {
            result[j++] = quoted[i];
        }
    }
    result[j] = '\0';
    return result;
}

static int parse_i18n_call(const char *line, const char *func_name,
                          char **text, char **context, int *is_plural) {
    char *pos = strstr(line, func_name);
    if (!pos) return 0;

    pos += strlen(func_name);
    while (*pos && (*pos == ' ' || *pos == '\t')) pos++;
    if (*pos != '(') return 0;
    pos++;

    *is_plural = (strcmp(func_name, "_p") == 0);
    *text = NULL;
    *context = NULL;

    while (*pos && (*pos == ' ' || *pos == '\t')) pos++;
    if (*pos != '"') {
		return 0;
	}

    char *start = pos;
    pos++;
    while (*pos && !(*pos == '"' && *(pos-1) != '\\')) pos++;
    if (!*pos) return 0;
    pos++;

    int str_len = pos - start;
    char *quoted = malloc(str_len + 1);
    if (!quoted) return 0;
    strncpy(quoted, start, str_len);
    quoted[str_len] = '\0';

    *text = extract_quoted_string(quoted);
    free(quoted);

    if (!*text) return 0;

    if (*is_plural) {
        while (*pos && (*pos == ' ' || *pos == '\t' || *pos == ',')) pos++;
        if (*pos == '"') {
            start = pos;
            pos++;
            while (*pos && !(*pos == '"' && *(pos-1) != '\\')) pos++;
            if (*pos) {
                pos++;
                str_len = pos - start;
                quoted = malloc(str_len + 1);
                if (quoted) {
                    strncpy(quoted, start, str_len);
                    quoted[str_len] = '\0';
                    *context = extract_quoted_string(quoted);
                    free(quoted);
                }
            }
        }
    }

    return 1;
}

static int string_exists(struct extractor *extractor, const char *text, const char *context) {
    for (int i = 0; i < extractor->count; i++) {
        if (strcmp(extractor->strings[i].text, text) == 0) {
            if ((!context && !extractor->strings[i].context) ||
                (context && extractor->strings[i].context &&
                 strcmp(context, extractor->strings[i].context) == 0)) {
                return 1;
            }
        }
    }
    return 0;
}

static void add_string(struct extractor *extractor, const char *text, const char *context,
                      int is_plural, const char *file, int line) {
    if (extractor->count >= MAX_STRINGS) return;
    if (string_exists(extractor, text, context)) return;

    struct extracted_string *str = &extractor->strings[extractor->count];
    str->text = strdup(text);
    str->context = context ? strdup(context) : NULL;
    str->is_plural = is_plural;
    str->file = strdup(file);
    str->line = line;

    extractor->count++;

    if (extractor->verbose) {
        printf("Extracted: \"%s\" from %s:%d\n", text, file, line);
    }
}

static void process_file(struct extractor *extractor, const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Warning: Cannot open file %s\n", filepath);
        return;
    }

    char line[MAX_LINE];
    int line_num = 0;

    while (fgets(line, sizeof(line), file)) {
        line_num++;

        char *text, *context;
        int is_plural;

        if (parse_i18n_call(line, "_", &text, &context, &is_plural)) {
            add_string(extractor, text, context, is_plural, filepath, line_num);
            free(text);
            if (context) free(context);
        }

        if (parse_i18n_call(line, "_p", &text, &context, &is_plural)) {
            add_string(extractor, text, context, is_plural, filepath, line_num);
            free(text);
            if (context) free(context);
        }
    }

    fclose(file);
}

static void scan_directory(struct extractor *extractor, const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        fprintf(stderr, "Warning: Cannot open directory %s\n", dirpath);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char filepath[MAX_PATH];
        snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);

        struct stat statbuf;
        if (stat(filepath, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                scan_directory(extractor, filepath);
            } else if (S_ISREG(statbuf.st_mode) && is_c_source_file(entry->d_name)) {
                process_file(extractor, filepath);
            }
        }
    }

    closedir(dir);
}

static void write_yaml_string(FILE *file, const char *str) {
    if (!str) {
        fprintf(file, "null");
        return;
    }

    int needs_quotes = 0;
    const char *p = str;
    while (*p) {
        if (*p == ':' || *p == '\n' || *p == '"' || *p == '\'' ||
            *p == '\\' || *p == '\t' || *p == '\r') {
            needs_quotes = 1;
            break;
        }
        p++;
    }

    if (needs_quotes) {
        fprintf(file, "\"");
        p = str;
        while (*p) {
            switch (*p) {
                case '"': fprintf(file, "\\\""); break;
                case '\\': fprintf(file, "\\\\"); break;
                case '\n': fprintf(file, "\\n"); break;
                case '\t': fprintf(file, "\\t"); break;
                case '\r': fprintf(file, "\\r"); break;
                default: fputc(*p, file); break;
            }
            p++;
        }
        fprintf(file, "\"");
    } else {
        fprintf(file, "%s", str);
    }
}

static void generate_yaml(struct extractor *extractor) {
    FILE *file = fopen(extractor->output_file, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot create output file %s\n", extractor->output_file);
        return;
    }

    fprintf(file, "# Translation template generated by wlf_i18n_extract\n");
    fprintf(file, "# This file contains all translatable strings found in the source code\n\n");

    fprintf(file, "en-US:\n");
    fprintf(file, "  _meta:\n");
    fprintf(file, "    language: \"English (US)\"\n");
    fprintf(file, "    completion: 100\n\n");

    for (int i = 0; i < extractor->count; i++) {
        struct extracted_string *str = &extractor->strings[i];
        if (!str->is_plural) {
            fprintf(file, "  ");
            write_yaml_string(file, str->text);
            fprintf(file, ": ");
            write_yaml_string(file, str->text);
            fprintf(file, " # %s:%d\n", str->file, str->line);
        }
    }

    int has_plurals = 0;
    for (int i = 0; i < extractor->count; i++) {
        if (extractor->strings[i].is_plural) {
            has_plurals = 1;
            break;
        }
    }

    if (has_plurals) {
        fprintf(file, "\n  # Plural forms\n");
        for (int i = 0; i < extractor->count; i++) {
            struct extracted_string *str = &extractor->strings[i];
            if (str->is_plural) {
                fprintf(file, "  ");
                write_yaml_string(file, str->text);
                fprintf(file, ":\n");
                fprintf(file, "    one: ");
                write_yaml_string(file, str->text);
                fprintf(file, "\n");
                fprintf(file, "    other: ");
                write_yaml_string(file, str->text);
                fprintf(file, " # %s:%d\n", str->file, str->line);
            }
        }
    }

    fprintf(file, "\n# Add other languages here:\n");
    fprintf(file, "# zh-CN:\n");
    fprintf(file, "#   _meta:\n");
    fprintf(file, "#     language: \"中文 (简体)\"\n");
    fprintf(file, "#     completion: 0\n");

    fclose(file);

    printf("Generated translation template: %s\n", extractor->output_file);
    printf("Found %d translatable strings\n", extractor->count);
}

static void cleanup_extractor(struct extractor *extractor) {
    for (int i = 0; i < extractor->count; i++) {
        free(extractor->strings[i].text);
        free(extractor->strings[i].context);
        free(extractor->strings[i].file);
    }
}

static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] SOURCE_DIRS...\n\n", program_name);
    printf("Extract translatable strings from C source code for wlf_i18n.\n\n");
    printf("SOURCE_DIRS: One or more directories to scan for C source files\n\n");
}

int main(int argc, char *argv[]) {
    struct extractor extractor = {0};
    char *output_file = "translations/template.yml";
    bool verbose = false;
    bool show_help = false;

    struct wlf_cmd_option options[] = {
        {WLF_OPTION_STRING, "output", 'o', &output_file},
        {WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose},
        {WLF_OPTION_BOOLEAN, "help", 'h', &show_help},
    };

    int remaining = wlf_cmd_parse_options(options, 3, &argc, argv);

    if (show_help || remaining < 0) {
        print_usage(argv[0]);
        wlf_print_options_help(options, 3, argv[0]);
        return (remaining < 0) ? 1 : 0;
    }

    if (argc < 2) {
        fprintf(stderr, "Error: No source directories specified\n\n");
        print_usage(argv[0]);
        wlf_print_options_help(options, 3, argv[0]);
        return 1;
    }

    extractor.output_file = output_file;
    extractor.verbose = verbose;
    extractor.source_dirs = &argv[1];
    extractor.source_dir_count = argc - 1;

    char *output_dir = strdup(extractor.output_file);
    char *last_slash = strrchr(output_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        char mkdir_cmd[MAX_PATH];
        snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", output_dir);
        system(mkdir_cmd);
    }
    free(output_dir);

    for (int i = 0; i < extractor.source_dir_count; i++) {
        printf("Scanning directory: %s\n", extractor.source_dirs[i]);
        scan_directory(&extractor, extractor.source_dirs[i]);
    }

    generate_yaml(&extractor);
    cleanup_extractor(&extractor);

    return 0;
}

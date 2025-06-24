#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include "wlf/utils/wlf_cmd_parser.h"

#define MAX_PATH 1024
#define MAX_LINE 4096
#define MAX_LANGUAGES 50
#define MAX_KEYS 10000
#define MAX_TRANSLATIONS 50000

struct translation {
    char key[256];
    char value[1024];
    char context[256];
    int is_plural;
};

struct language {
    char *code;
    char *name;
    struct translation *translations;
    int translation_count;
    int translation_capacity;
};

struct compiler {
    struct language *languages;
    int language_count;
    int language_capacity;
    char *input_dir;
    char *output_dir;
    char *header_file;
    char *source_file;
    bool verbose;
};

static void init_compiler(struct compiler *comp) {
    memset(comp, 0, sizeof(struct compiler));
    comp->input_dir = strdup("locales");
    comp->output_dir = strdup(".");
    comp->header_file = strdup("wlf_i18n_data.h");
    comp->source_file = strdup("wlf_i18n_data.c");
    comp->language_capacity = 10;
    comp->languages = malloc(sizeof(struct language) * comp->language_capacity);
    memset(comp->languages, 0, sizeof(struct language) * comp->language_capacity);
}

static void cleanup_compiler(struct compiler *comp) {
    if (comp->input_dir) free(comp->input_dir);
    if (comp->output_dir) free(comp->output_dir);
    if (comp->header_file) free(comp->header_file);
    if (comp->source_file) free(comp->source_file);

    if (comp->languages) {
        for (int i = 0; i < comp->language_count; i++) {
            if (comp->languages[i].code) free(comp->languages[i].code);
            if (comp->languages[i].name) free(comp->languages[i].name);
            if (comp->languages[i].translations) free(comp->languages[i].translations);
        }
        free(comp->languages);
    }
}

static char *trim(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';
    return str;
}

static int parse_yaml_line(const char *line, char *key, char *value) {
    char *colon = strchr(line, ':');
    if (!colon) return 0;

    int key_len = colon - line;
    strncpy(key, line, key_len);
    key[key_len] = '\0';
    strcpy(key, trim(key));

    if (key[0] == '"' || key[0] == '\'') {
        memmove(key, key + 1, strlen(key));
        char *end = key + strlen(key) - 1;
        if (*end == '"' || *end == '\'') *end = '\0';
    }

    strcpy(value, trim(colon + 1));

    if (value[0] == '"' || value[0] == '\'') {
        memmove(value, value + 1, strlen(value));
        char *end = value + strlen(value) - 1;
        if (*end == '"' || *end == '\'') *end = '\0';
    }

    return 1;
}

static int load_language_file(struct compiler *comp, const char *filepath, const char *lang_code) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        if (comp->verbose) {
            printf("Warning: Cannot open language file: %s\n", filepath);
        }
        return 0;
    }

    if (comp->language_count >= comp->language_capacity) {
        comp->language_capacity *= 2;
        comp->languages = realloc(comp->languages, sizeof(struct language) * comp->language_capacity);
    }

    struct language *lang = &comp->languages[comp->language_count];
    memset(lang, 0, sizeof(struct language));
    lang->code = strdup(lang_code);
    lang->name = malloc(64);
    snprintf(lang->name, 64, "Language %s", lang_code);
    lang->translation_capacity = 100;
    lang->translations = malloc(sizeof(struct translation) * lang->translation_capacity);
    lang->translation_count = 0;

    char line[MAX_LINE];
    int line_num = 0;

    while (fgets(line, sizeof(line), file)) {
        line_num++;
        char *trimmed = trim(line);

        if (strlen(trimmed) == 0 || trimmed[0] == '#') continue;

        char key[256] = {0};
        char value[1024] = {0};

        if (parse_yaml_line(trimmed, key, value)) {
            if (lang->translation_count >= lang->translation_capacity) {
                lang->translation_capacity *= 2;
                lang->translations = realloc(lang->translations, sizeof(struct translation) * lang->translation_capacity);
            }

            struct translation *trans = &lang->translations[lang->translation_count];
            strcpy(trans->key, key);
            strcpy(trans->value, value);
            strcpy(trans->context, "");
            trans->is_plural = (strstr(key, "_plural") != NULL);
            lang->translation_count++;

            if (comp->verbose) {
                printf("  Loaded: %s = %s\n", key, value);
            }
        }
    }

    fclose(file);
    comp->language_count++;

    if (comp->verbose) {
        printf("Loaded %d translations from %s\n", lang->translation_count, filepath);
    }

    return 1;
}

static int scan_language_files(struct compiler *comp) {
    DIR *dir = opendir(comp->input_dir);
    if (!dir) {
        fprintf(stderr, "Error: Cannot open input directory: %s\n", comp->input_dir);
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_REG) continue;

        char *ext = strrchr(entry->d_name, '.');
        if (!ext || (strcmp(ext, ".yml") != 0 && strcmp(ext, ".yaml") != 0)) continue;

        char lang_code[16];
        int name_len = ext - entry->d_name;
        if (name_len >= sizeof(lang_code)) name_len = sizeof(lang_code) - 1;
        strncpy(lang_code, entry->d_name, name_len);
        lang_code[name_len] = '\0';

        char filepath[MAX_PATH];
        snprintf(filepath, sizeof(filepath), "%s/%s", comp->input_dir, entry->d_name);

        if (comp->verbose) {
            printf("Loading language file: %s (language: %s)\n", filepath, lang_code);
        }

        load_language_file(comp, filepath, lang_code);
    }

    closedir(dir);
    return comp->language_count > 0;
}

static void generate_c_identifier(const char *key, char *identifier) {
    strcpy(identifier, "WLF_I18N_");
    char *p = identifier + strlen(identifier);

    for (const char *k = key; *k; k++) {
        if (isalnum(*k)) {
            *p++ = toupper(*k);
        } else {
            *p++ = '_';
        }
    }
    *p = '\0';
}

static void escape_c_string(const char *input, char *output) {
    const char *p = input;
    char *o = output;

    while (*p) {
        switch (*p) {
            case '"':
                *o++ = '\\';
                *o++ = '"';
                break;
            case '\\':
                *o++ = '\\';
                *o++ = '\\';
                break;
            case '\n':
                *o++ = '\\';
                *o++ = 'n';
                break;
            case '\r':
                *o++ = '\\';
                *o++ = 'r';
                break;
            case '\t':
                *o++ = '\\';
                *o++ = 't';
                break;
            default:
                *o++ = *p;
                break;
        }
        p++;
    }
    *o = '\0';
}

static int generate_header_file(struct compiler *comp) {
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s/%s", comp->output_dir, comp->header_file);

    FILE *file = fopen(filepath, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot create header file: %s\n", filepath);
        return 0;
    }

    fprintf(file, "/**\n");
    fprintf(file, " * @file        %s\n", comp->header_file);
    fprintf(file, " * @brief       Auto-generated i18n data header\n");
    fprintf(file, " * @warning     This file is automatically generated. Do not edit!\n");
    fprintf(file, " */\n\n");
    fprintf(file, "#ifndef WLF_I18N_DATA_H\n");
    fprintf(file, "#define WLF_I18N_DATA_H\n\n");
    fprintf(file, "#include \"wlf/utils/wlf_i18n.h\"\n\n");

    fprintf(file, "/* Translation key constants */\n");
    if (comp->language_count > 0) {
        struct language *first_lang = &comp->languages[0];
        for (int i = 0; i < first_lang->translation_count; i++) {
            char identifier[256];
            generate_c_identifier(first_lang->translations[i].key, identifier);
            fprintf(file, "#define %-40s \"%s\"\n", identifier, first_lang->translations[i].key);
        }
    }

    fprintf(file, "\n/* Language initialization functions */\n");
    for (int i = 0; i < comp->language_count; i++) {
        fprintf(file, "void wlf_i18n_init_lang_%s(void);\n", comp->languages[i].code);
    }

    fprintf(file, "\n/* Initialize all languages */\n");
    fprintf(file, "void wlf_i18n_init_all_languages(void);\n");

    fprintf(file, "\n#endif /* WLF_I18N_DATA_H */\n");

    fclose(file);

    if (comp->verbose) {
        printf("Generated header file: %s\n", filepath);
    }

    return 1;
}

static int generate_source_file(struct compiler *comp) {
    char filepath[MAX_PATH];
    snprintf(filepath, sizeof(filepath), "%s/%s", comp->output_dir, comp->source_file);

    FILE *file = fopen(filepath, "w");
    if (!file) {
        fprintf(stderr, "Error: Cannot create source file: %s\n", filepath);
        return 0;
    }

    fprintf(file, "/**\n");
    fprintf(file, " * @file        %s\n", comp->source_file);
    fprintf(file, " * @brief       Auto-generated i18n data source\n");
    fprintf(file, " * @warning     This file is automatically generated. Do not edit!\n");
    fprintf(file, " */\n\n");
    fprintf(file, "#include \"%s\"\n\n", comp->header_file);

    for (int i = 0; i < comp->language_count; i++) {
        struct language *lang = &comp->languages[i];

        fprintf(file, "/**\n");
        fprintf(file, " * Initialize %s language\n", lang->code);
        fprintf(file, " */\n");
        fprintf(file, "void wlf_i18n_init_lang_%s(void) {\n", lang->code);
        fprintf(file, "    wlf_i18n_add_language(\"%s\", \"%s\");\n", lang->code, lang->name);

        for (int j = 0; j < lang->translation_count; j++) {
            struct translation *trans = &lang->translations[j];
            char escaped_value[2048];
            escape_c_string(trans->value, escaped_value);

            if (trans->is_plural) {
                fprintf(file, "    wlf_i18n_add_plural_translation(\"%s\", \"%s\", \"%s\", \"%s\");\n",
                        lang->code, trans->key, escaped_value, "");
            } else {
                fprintf(file, "    wlf_i18n_add_translation(\"%s\", \"%s\", \"%s\");\n",
                        lang->code, trans->key, escaped_value);
            }
        }

        fprintf(file, "}\n\n");
    }

    fprintf(file, "/**\n");
    fprintf(file, " * Initialize all languages\n");
    fprintf(file, " */\n");
    fprintf(file, "void wlf_i18n_init_all_languages(void) {\n");
    for (int i = 0; i < comp->language_count; i++) {
        fprintf(file, "    wlf_i18n_init_lang_%s();\n", comp->languages[i].code);
    }
    fprintf(file, "}\n");

    fclose(file);

    if (comp->verbose) {
        printf("Generated source file: %s\n", filepath);
    }

    return 1;
}

static void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Compile YAML translation files into C header and source files\n\n");
    printf("Options:\n");
    printf("  -i, --input DIR       Input directory containing YAML files (default: locales)\n");
    printf("  -o, --output DIR      Output directory for generated files (default: .)\n");
    printf("  -h, --header FILE     Header filename (default: wlf_i18n_data.h)\n");
    printf("  -s, --source FILE     Source filename (default: wlf_i18n_data.c)\n");
    printf("  -v, --verbose         Enable verbose output\n");
    printf("  --help                Show this help message\n");
    printf("\nExample:\n");
    printf("  %s -i locales -o src/generated -v\n", program_name);
}

int main(int argc, char *argv[]) {
    struct compiler compiler;
    init_compiler(&compiler);

    static struct option long_options[] = {
        {"input",   required_argument, 0, 'i'},
        {"output",  required_argument, 0, 'o'},
        {"header",  required_argument, 0, 'h'},
        {"source",  required_argument, 0, 's'},
        {"verbose", no_argument,       0, 'v'},
        {"help",    no_argument,       0, 1000},
        {0, 0, 0, 0}
    };

    int c;
    while ((c = getopt_long(argc, argv, "i:o:h:s:v", long_options, NULL)) != -1) {
        switch (c) {
            case 'i':
                free(compiler.input_dir);
                compiler.input_dir = strdup(optarg);
                break;
            case 'o':
                free(compiler.output_dir);
                compiler.output_dir = strdup(optarg);
                break;
            case 'h':
                free(compiler.header_file);
                compiler.header_file = strdup(optarg);
                break;
            case 's':
                free(compiler.source_file);
                compiler.source_file = strdup(optarg);
                break;
            case 'v':
                compiler.verbose = 1;
                break;
            case 1000:
                print_usage(argv[0]);
                cleanup_compiler(&compiler);
                return 0;
            case '?':
                print_usage(argv[0]);
                cleanup_compiler(&compiler);
                return 1;
        }
    }

    if (compiler.verbose) {
        printf("wlf_i18n_compile - Translation Compiler\n");
        printf("Input directory: %s\n", compiler.input_dir);
        printf("Output directory: %s\n", compiler.output_dir);
        printf("Header file: %s\n", compiler.header_file);
        printf("Source file: %s\n", compiler.source_file);
        printf("\n");
    }

    if (!scan_language_files(&compiler)) {
        fprintf(stderr, "Error: No language files found in %s\n", compiler.input_dir);
        cleanup_compiler(&compiler);
        return 1;
    }

    mkdir(compiler.output_dir, 0755);
    if (!generate_header_file(&compiler)) {
        cleanup_compiler(&compiler);
        return 1;
    }

    if (!generate_source_file(&compiler)) {
        cleanup_compiler(&compiler);
        return 1;
    }

    if (compiler.verbose) {
        printf("\nCompilation completed successfully!\n");
        printf("Generated %d language(s) with %d total translations\n",
               compiler.language_count,
               compiler.language_count > 0 ? compiler.languages[0].translation_count : 0);
    }

    cleanup_compiler(&compiler);
    return 0;
}

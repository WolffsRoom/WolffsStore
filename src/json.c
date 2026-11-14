#include "json.h"

#include <stdlib.h>
#include <string.h>

struct JsonValue {
    JsonType type;
    union {
        int boolean;
        double number;
        char *string;
    };
    JsonValue **items;
    int count;
    char **keys;
    JsonValue **values;
};

typedef struct {
    const char *p;
} Parser;

static void skip_ws(Parser *ps) {
    while (*ps->p == ' ' || *ps->p == '\t' || *ps->p == '\n' || *ps->p == '\r') ps->p++;
}

static JsonValue *json_new(JsonType type) {
    JsonValue *v = calloc(1, sizeof(JsonValue));
    v->type = type;
    return v;
}

static int utf8_encode(unsigned int cp, char *out) {
    if (cp <= 0x7F) {
        out[0] = (char)cp;
        return 1;
    } else if (cp <= 0x7FF) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp <= 0xFFFF) {
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    } else {
        out[0] = (char)(0xF0 | (cp >> 18));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
}

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static unsigned int parse_hex4(Parser *ps) {
    unsigned int v = 0;
    for (int i = 0; i < 4; i++) {
        int h = hex_val(*ps->p);
        if (h < 0) return 0;
        v = (v << 4) | (unsigned int)h;
        ps->p++;
    }
    return v;
}

static char *parse_string_raw(Parser *ps) {
    if (*ps->p != '"') return NULL;
    ps->p++;

    size_t cap = 64;
    size_t len = 0;
    char *buf = malloc(cap);

    while (*ps->p && *ps->p != '"') {
        char c = *ps->p;
        char enc[4];
        int enc_len = 0;

        if (c == '\\') {
            ps->p++;
            switch (*ps->p) {
                case '"':  enc[0] = '"';  enc_len = 1; ps->p++; break;
                case '\\': enc[0] = '\\'; enc_len = 1; ps->p++; break;
                case '/':  enc[0] = '/';  enc_len = 1; ps->p++; break;
                case 'b':  enc[0] = '\b'; enc_len = 1; ps->p++; break;
                case 'f':  enc[0] = '\f'; enc_len = 1; ps->p++; break;
                case 'n':  enc[0] = '\n'; enc_len = 1; ps->p++; break;
                case 'r':  enc[0] = '\r'; enc_len = 1; ps->p++; break;
                case 't':  enc[0] = '\t'; enc_len = 1; ps->p++; break;
                case 'u': {
                    ps->p++;
                    unsigned int cp = parse_hex4(ps);
                    if (cp >= 0xD800 && cp <= 0xDBFF && ps->p[0] == '\\' && ps->p[1] == 'u') {
                        ps->p += 2;
                        unsigned int lo = parse_hex4(ps);
                        if (lo >= 0xDC00 && lo <= 0xDFFF)
                            cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                    }
                    enc_len = utf8_encode(cp, enc);
                    break;
                }
                default:
                    enc[0] = *ps->p; enc_len = 1; ps->p++; break;
            }
        } else {
            enc[0] = c;
            enc_len = 1;
            ps->p++;
        }

        if (len + (size_t)enc_len + 1 > cap) {
            cap *= 2;
            buf = realloc(buf, cap);
        }
        memcpy(buf + len, enc, enc_len);
        len += enc_len;
    }

    if (*ps->p == '"') ps->p++;
    buf[len] = '\0';
    return buf;
}

static JsonValue *parse_value(Parser *ps);

static JsonValue *parse_object(Parser *ps) {
    JsonValue *v = json_new(JSON_OBJECT);
    ps->p++; /* { */
    skip_ws(ps);

    int cap = 4;
    v->keys = malloc(sizeof(char *) * cap);
    v->values = malloc(sizeof(JsonValue *) * cap);
    v->count = 0;

    if (*ps->p == '}') { ps->p++; return v; }

    while (1) {
        skip_ws(ps);
        char *key = parse_string_raw(ps);
        if (!key) { json_free(v); return NULL; }
        skip_ws(ps);
        if (*ps->p != ':') { free(key); json_free(v); return NULL; }
        ps->p++;
        skip_ws(ps);
        JsonValue *val = parse_value(ps);
        if (!val) { free(key); json_free(v); return NULL; }

        if (v->count >= cap) {
            cap *= 2;
            v->keys = realloc(v->keys, sizeof(char *) * cap);
            v->values = realloc(v->values, sizeof(JsonValue *) * cap);
        }
        v->keys[v->count] = key;
        v->values[v->count] = val;
        v->count++;

        skip_ws(ps);
        if (*ps->p == ',') { ps->p++; continue; }
        if (*ps->p == '}') { ps->p++; break; }
        json_free(v);
        return NULL;
    }

    return v;
}

static JsonValue *parse_array(Parser *ps) {
    JsonValue *v = json_new(JSON_ARRAY);
    ps->p++; /* [ */
    skip_ws(ps);

    int cap = 4;
    v->items = malloc(sizeof(JsonValue *) * cap);
    v->count = 0;

    if (*ps->p == ']') { ps->p++; return v; }

    while (1) {
        skip_ws(ps);
        JsonValue *val = parse_value(ps);
        if (!val) { json_free(v); return NULL; }

        if (v->count >= cap) {
            cap *= 2;
            v->items = realloc(v->items, sizeof(JsonValue *) * cap);
        }
        v->items[v->count++] = val;

        skip_ws(ps);
        if (*ps->p == ',') { ps->p++; continue; }
        if (*ps->p == ']') { ps->p++; break; }
        json_free(v);
        return NULL;
    }

    return v;
}

static JsonValue *parse_value(Parser *ps) {
    skip_ws(ps);
    char c = *ps->p;

    if (c == '{') return parse_object(ps);
    if (c == '[') return parse_array(ps);
    if (c == '"') {
        char *s = parse_string_raw(ps);
        if (!s) return NULL;
        JsonValue *v = json_new(JSON_STRING);
        v->string = s;
        return v;
    }
    if (c == 't' && strncmp(ps->p, "true", 4) == 0) {
        ps->p += 4;
        JsonValue *v = json_new(JSON_BOOL);
        v->boolean = 1;
        return v;
    }
    if (c == 'f' && strncmp(ps->p, "false", 5) == 0) {
        ps->p += 5;
        JsonValue *v = json_new(JSON_BOOL);
        v->boolean = 0;
        return v;
    }
    if (c == 'n' && strncmp(ps->p, "null", 4) == 0) {
        ps->p += 4;
        return json_new(JSON_NULL);
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        char *end;
        double num = strtod(ps->p, &end);
        if (end == ps->p) return NULL;
        ps->p = end;
        JsonValue *v = json_new(JSON_NUMBER);
        v->number = num;
        return v;
    }

    return NULL;
}

JsonValue *json_parse(const char *text) {
    if (!text) return NULL;
    Parser ps;
    ps.p = text;
    return parse_value(&ps);
}

void json_free(JsonValue *v) {
    if (!v) return;

    switch (v->type) {
        case JSON_STRING:
            free(v->string);
            break;
        case JSON_ARRAY:
            for (int i = 0; i < v->count; i++) json_free(v->items[i]);
            free(v->items);
            break;
        case JSON_OBJECT:
            for (int i = 0; i < v->count; i++) {
                free(v->keys[i]);
                json_free(v->values[i]);
            }
            free(v->keys);
            free(v->values);
            break;
        default:
            break;
    }
    free(v);
}

JsonType json_type(const JsonValue *v) {
    return v ? v->type : JSON_NULL;
}

int json_array_count(const JsonValue *v) {
    return (v && v->type == JSON_ARRAY) ? v->count : 0;
}

JsonValue *json_array_get(const JsonValue *v, int index) {
    if (!v || v->type != JSON_ARRAY || index < 0 || index >= v->count) return NULL;
    return v->items[index];
}

JsonValue *json_object_get(const JsonValue *v, const char *key) {
    if (!v || v->type != JSON_OBJECT) return NULL;
    for (int i = 0; i < v->count; i++) {
        if (strcmp(v->keys[i], key) == 0) return v->values[i];
    }
    return NULL;
}

const char *json_string(const JsonValue *v, const char *default_value) {
    return (v && v->type == JSON_STRING) ? v->string : default_value;
}

double json_number(const JsonValue *v, double default_value) {
    return (v && v->type == JSON_NUMBER) ? v->number : default_value;
}

int json_bool(const JsonValue *v, int default_value) {
    return (v && v->type == JSON_BOOL) ? v->boolean : default_value;
}

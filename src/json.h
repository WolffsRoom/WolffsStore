#ifndef VITASEED_JSON_H
#define VITASEED_JSON_H

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef struct JsonValue JsonValue;

/* Parses `text` (NUL-terminated) into a JSON value tree. Returns NULL on
 * malformed input. Free the result with json_free(). */
JsonValue *json_parse(const char *text);
void json_free(JsonValue *v);

JsonType json_type(const JsonValue *v);

int json_array_count(const JsonValue *v);
JsonValue *json_array_get(const JsonValue *v, int index);

/* NULL if `v` isn't an object or has no such key. */
JsonValue *json_object_get(const JsonValue *v, const char *key);

const char *json_string(const JsonValue *v, const char *default_value);
double json_number(const JsonValue *v, double default_value);
int json_bool(const JsonValue *v, int default_value);

#endif

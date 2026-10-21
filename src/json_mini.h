#ifndef JSON_MINI_H
#define JSON_MINI_H

#include <stddef.h>

/* Minimal JSON emitter helpers. Builds strings into a caller-supplied buffer. */

/* Emit a JSON string value (with escaping for " and \). */
size_t json_escape(const char *src, char *dst, size_t dst_size);

/* Emit a complete object: {"key":"value","key2":42} */
size_t json_object_begin(char *buf, size_t size);
size_t json_object_end(char *buf, size_t size);
size_t json_pair(char *buf, size_t size, const char *key, const char *str_val);
size_t json_pair_int(char *buf, size_t size, const char *key, int val);

/* Convenience: build {"ok":true,"data":{...}} */
size_t json_ok_response(char *buf, size_t size, const char *inner_json);

#endif

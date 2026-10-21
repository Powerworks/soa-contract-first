#include "json_mini.h"
#include <string.h>

size_t json_escape(const char *src, char *dst, size_t dst_size) {
    size_t i = 0, j = 0;
    while (src[i] && j + 1 < dst_size) {
        char c = src[i++];
        if (c == '"' || c == '\\') {
            if (j + 2 >= dst_size) break;
            dst[j++] = '\\';
            dst[j++] = c;
        } else {
            dst[j++] = c;
        }
    }
    dst[j] = '\0';
    return j;
}

size_t json_object_begin(char *buf, size_t size) {
    if (size == 0) return 0;
    buf[0] = '{';
    buf[1] = '\0';
    return 1;
}

size_t json_object_end(char *buf, size_t size) {
    size_t len = strlen(buf);
    if (len + 2 >= size) return len;
    buf[len] = '}';
    buf[len + 1] = '\0';
    return len + 1;
}

size_t json_pair(char *buf, size_t size, const char *key, const char *str_val) {
    size_t len = strlen(buf);
    size_t need = len + strlen(key) + strlen(str_val) + 6; /* "key":"val", */
    if (need >= size) return len;
    char escaped[512];
    json_escape(str_val, escaped, sizeof(escaped));
    len += snprintf(buf + len, size - len, "%s\"%s\":\"%s\"",
                    (len > 1 ? "," : ""), key, escaped);
    return strlen(buf);
}

size_t json_pair_int(char *buf, size_t size, const char *key, int val) {
    size_t len = strlen(buf);
    size_t need = len + strlen(key) + 12; /* "key":42, */
    if (need >= size) return len;
    len += snprintf(buf + len, size - len, "%s\"%s\":%d",
                    (len > 1 ? "," : ""), key, val);
    return strlen(buf);
}

size_t json_ok_response(char *buf, size_t size, const char *inner_json) {
    size_t len = snprintf(buf, size, "{\"ok\":true,\"data\":%s}", inner_json ? inner_json : "{}");
    return len;
}

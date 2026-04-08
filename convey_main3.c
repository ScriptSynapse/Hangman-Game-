/*
 * convey_main.c  –  Implementation of the convey dynamic-string library.
 *
 * Build notes:
 *   Compile together with utf8proc.c (or link -lutf8proc).
 *   C99 or later required.
 */

#include "convey3.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#ifndef SSIZE_MAX
#  define SSIZE_MAX ((size_t)(~(size_t)0) >> 1)
#endif

/* ── Internal null-byte helper ─────────────────────────────────────────────
 * Mirrors C_STRING_NULL_SIZE but as an inline function for cleaner code in
 * places where the macro expansion would be ugly.
 */
 size_t null_size_for(unsigned char enc)
{
    return ((enc & C_STRING_ENC_MASK) == C_STRING_ENC_UTF16) ? 2u : 1u;
}

/* ── Unaligned 16-bit read/write helpers ────────────────────────────────────
 * Use memcpy so the compiler can emit the correct load instruction without
 * undefined behaviour from strict-aliasing or alignment violations.
 */
uint16_t c_read_u16(const void *ptr)
{
    uint16_t v;
    memcpy(&v, ptr, sizeof v);
    return v;
}
 void c_write_u16(void *ptr, uint16_t v)
{
    memcpy(ptr, &v, sizeof v);
}

/* ── Byte-swap a uint16_t ───────────────────────────────────────────────── */
 uint16_t bswap16(uint16_t v)
{
    return (uint16_t)(((v & 0x00FFu) << 8) | ((v & 0xFF00u) >> 8));
}

/* ── Runtime endianness detection ──────────────────────────────────────────
 * Returns true when the host is little-endian.
 */
 bool host_is_le(void)
{
    const int probe = 1;
    return (*(const char *)&probe) == 1;
}

/*
 * Determine whether byte-swapping is needed for a UTF-16 operation.
 *   use_big_endian=true  → data is / should be UTF-16 BE
 *   use_big_endian=false → data is / should be UTF-16 LE  (our native storage)
 *
 * swap == true when the data endianness differs from the host.
 */
 bool needs_swap(bool use_big_endian)
{
    return use_big_endian == host_is_le();
}

//----------------------------------------------------------------------------------
// Internal Getters & Setters
//----------------------------------------------------------------------------------

size_t c_string_get_header_size(const unsigned char flags)
{
    switch (flags & C_STRING_TYPE_MASK) {
        case C_STRING_TYPE_8:  return sizeof(struct c_string_header_8);
        case C_STRING_TYPE_16: return sizeof(struct c_string_header_16);
        case C_STRING_TYPE_32: return sizeof(struct c_string_header_32);
        case C_STRING_TYPE_64: return sizeof(struct c_string_header_64);
    }
    return 0;
}

unsigned char c_string_determine_type(const size_t size)
{
    if (size < 256u)              return C_STRING_TYPE_8;
    if (size < 65536u)            return C_STRING_TYPE_16;
    if (size <= 4294967295ULL)    return C_STRING_TYPE_32;
    return C_STRING_TYPE_64;
}

unsigned char c_string_get_encoding(c_const_string_t const string)
{
    if (!string) return C_STRING_ENC_ASCII;
    return (unsigned char)(string[-1] & C_STRING_ENC_MASK);
}

void c_string_set_encoding(c_string_t const string, const unsigned char enc)
{
    if (!string) return;
    const unsigned char type = (unsigned char)(string[-1] & C_STRING_TYPE_MASK);
    string[-1] = (unsigned char)(type | (enc & C_STRING_ENC_MASK));
}

size_t c_string_get_used_length(c_const_string_t const string)
{
    if (!string) return 0;
    switch (string[-1] & C_STRING_TYPE_MASK) {
        case C_STRING_TYPE_8:  return GET_CONST_HEADER_8(string)->used_length;
        case C_STRING_TYPE_16: return GET_CONST_HEADER_16(string)->used_length;
        case C_STRING_TYPE_32: return GET_CONST_HEADER_32(string)->used_length;
        case C_STRING_TYPE_64: return GET_CONST_HEADER_64(string)->used_length;
    }
    return 0;
}

size_t c_string_get_available_capacity(c_const_string_t const string)
{
    if (!string) return 0;
    switch (string[-1] & C_STRING_TYPE_MASK) {
        case C_STRING_TYPE_8: {
            const struct c_string_header_8 *h = GET_CONST_HEADER_8(string);
            return h->allocated_capacity > h->used_length
                   ? (size_t)(h->allocated_capacity - h->used_length) : 0;
        }
        case C_STRING_TYPE_16: {
            const struct c_string_header_16 *h = GET_CONST_HEADER_16(string);
            return h->allocated_capacity > h->used_length
                   ? (size_t)(h->allocated_capacity - h->used_length) : 0;
        }
        case C_STRING_TYPE_32: {
            const struct c_string_header_32 *h = GET_CONST_HEADER_32(string);
            return h->allocated_capacity > h->used_length
                   ? (size_t)(h->allocated_capacity - h->used_length) : 0;
        }
        case C_STRING_TYPE_64: {
            const struct c_string_header_64 *h = GET_CONST_HEADER_64(string);
            return h->allocated_capacity > h->used_length
                   ? (size_t)(h->allocated_capacity - h->used_length) : 0;
        }
    }
    return 0;
}

size_t c_string_get_capacity(c_const_string_t const string)
{
    if (!string) return 0;
    return c_string_get_used_length(string) + c_string_get_available_capacity(string);
}

void c_string_set_lengths(const c_string_t string, const size_t used, const size_t capacity)
{
    if (!string) return;
    switch (string[-1] & C_STRING_TYPE_MASK) {
        case C_STRING_TYPE_8:
            GET_HEADER_8(string)->used_length        = (uint8_t)used;
            GET_HEADER_8(string)->allocated_capacity = (uint8_t)capacity;
            break;
        case C_STRING_TYPE_16:
            GET_HEADER_16(string)->used_length        = (uint16_t)used;
            GET_HEADER_16(string)->allocated_capacity = (uint16_t)capacity;
            break;
        case C_STRING_TYPE_32:
            GET_HEADER_32(string)->used_length        = (uint32_t)used;
            GET_HEADER_32(string)->allocated_capacity = (uint32_t)capacity;
            break;
        case C_STRING_TYPE_64:
            GET_HEADER_64(string)->used_length        = (uint64_t)used;
            GET_HEADER_64(string)->allocated_capacity = (uint64_t)capacity;
            break;
    }
}

void c_string_set_used_length(const c_string_t string, const size_t used)
{
    if (!string) return;
    switch (string[-1] & C_STRING_TYPE_MASK) {
        case C_STRING_TYPE_8:  GET_HEADER_8(string)->used_length  = (uint8_t)used;  break;
        case C_STRING_TYPE_16: GET_HEADER_16(string)->used_length = (uint16_t)used; break;
        case C_STRING_TYPE_32: GET_HEADER_32(string)->used_length = (uint32_t)used; break;
        case C_STRING_TYPE_64: GET_HEADER_64(string)->used_length = (uint64_t)used; break;
    }
}

//----------------------------------------------------------------------------------
// Codepoint Count
//----------------------------------------------------------------------------------

size_t c_string_codepoint_count(c_const_string_t const string)
{
    if (!string) return 0;

    const size_t        total = c_string_get_used_length(string);
    const unsigned char enc   = c_string_get_encoding(string);

    if (total == 0) return 0;

    if (enc == C_STRING_ENC_UTF8) {
        size_t count = 0, pos = 0;
        while (pos < total) {
            utf8proc_int32_t cp;
            const size_t rem = total - pos;
            const utf8proc_ssize_t bound =
                (utf8proc_ssize_t)(rem > (size_t)SSIZE_MAX ? SSIZE_MAX : rem);
            const utf8proc_ssize_t adv = utf8proc_iterate(string + pos, bound, &cp);
            if (adv <= 0) { pos++; count++; continue; } /* count invalid byte as one unit */
            pos   += (size_t)adv;
            count += 1;
        }
        return count;
    }

    if (enc == C_STRING_ENC_UTF16) {
        /* FIX: validate the low surrogate before consuming it.
         * Previously the code would skip the next unit unconditionally when it
         * saw a high surrogate, potentially eating a regular character.
         */
        size_t count = 0;
        const size_t total_units = total / 2;
        size_t unit_pos = 0;

        while (unit_pos < total_units) {
            const uint16_t u1 = c_read_u16(string + unit_pos * 2);
            unit_pos++;

            if (u1 >= 0xD800u && u1 <= 0xDBFFu && unit_pos < total_units) {
                /* High surrogate – peek at next unit */
                const uint16_t u2 = c_read_u16(string + unit_pos * 2);
                if (u2 >= 0xDC00u && u2 <= 0xDFFFu) {
                    unit_pos++; /* confirmed surrogate pair → consume low unit */
                }
                /* else lone high surrogate → already counted as one "codepoint" */
            }
            count++;
        }
        return count;
    }

    /* ASCII / raw: every byte is one codepoint */
    return total;
}

//----------------------------------------------------------------------------------
// Slice Extractors
//----------------------------------------------------------------------------------

c_slice_t c_slice_of(c_const_string_t string)
{
    c_slice_t s = { string, c_string_get_used_length(string) };
    return s;
}

c_slice_t c_subslice(c_const_string_t string, size_t byte_offset, size_t byte_length)
{
    c_slice_t s = { NULL, 0 };
    if (!string) return s;
    const size_t total = c_string_get_used_length(string);
    if (byte_offset >= total) return s;
    const size_t remaining = total - byte_offset;
    s.data   = string + byte_offset;
    s.length = byte_length > remaining ? remaining : byte_length;
    return s;
}

c_utf8_slice_t c_utf8_slice_of(c_const_string_t string)
{
    c_utf8_slice_t s = { string, c_string_get_used_length(string) };
    return s;
}

c_utf8_slice_t c_utf8_subslice(c_const_string_t string,
                                size_t byte_offset, size_t byte_length)
{
    c_utf8_slice_t s = { NULL, 0 };
    if (!string) return s;
    const size_t total = c_string_get_used_length(string);
    if (byte_offset >= total) return s;
    const size_t remaining = total - byte_offset;
    s.data   = string + byte_offset;
    s.length = byte_length > remaining ? remaining : byte_length;
    return s;
}

c_utf8_slice_t c_utf8_subslice_codepoints(c_const_string_t string,
                                           size_t cp_offset, size_t cp_count)
{
    c_utf8_slice_t s = { NULL, 0 };
    if (!string) return s;

    const size_t total = c_string_get_used_length(string);
    size_t pos = 0;

        for (size_t i = 0; i < cp_offset && pos < total; i++) {
                utf8proc_int32_t cp;
                const size_t rem = total - pos;
                const utf8proc_ssize_t bound =
                    (utf8proc_ssize_t)(rem > (size_t)SSIZE_MAX ? SSIZE_MAX : rem);
                const utf8proc_ssize_t adv = utf8proc_iterate(string + pos, bound, &cp);
                if (adv <= 0) { 
                    pos++; 
                    continue; 
                }
                pos += (size_t)adv;
            }

    if (pos >= total && cp_offset > 0) return s;
    const size_t start = pos;

    for (size_t i = 0; i < cp_count && pos < total; i++) {
        utf8proc_int32_t cp;
        const size_t rem = total - pos;
        const utf8proc_ssize_t bound =
            (utf8proc_ssize_t)(rem > (size_t)SSIZE_MAX ? SSIZE_MAX : rem);
        const utf8proc_ssize_t adv = utf8proc_iterate(string + pos, bound, &cp);
        if (adv <= 0) { pos++; continue; }
        pos += (size_t)adv;
    }

    s.data   = string + start;
    s.length = pos - start;
    return s;
}

c_utf16_slice_t c_utf16_slice_of(c_const_string_t string)
{
    c_utf16_slice_t s = { string, c_string_get_used_length(string) };
    return s;
}

c_utf16_slice_t c_utf16_subslice(c_const_string_t string,
                                  size_t byte_offset, size_t byte_length)
{
    c_utf16_slice_t s = { NULL, 0 };
    if (!string) return s;
    byte_offset &= ~(size_t)1u;   /* force 2-byte alignment */
    byte_length &= ~(size_t)1u;
    const size_t total = c_string_get_used_length(string) & ~(size_t)1u;
    if (byte_offset >= total) return s;
    const size_t remaining = total - byte_offset;
    s.data   = string + byte_offset;
    s.length = byte_length > remaining ? remaining : byte_length;
    return s;
}

c_utf16_slice_t c_utf16_subslice_codepoints(c_const_string_t string,
                                             size_t cp_offset, size_t cp_count)
{
    c_utf16_slice_t s = { NULL, 0 };
    if (!string) return s;

    const size_t total_bytes = c_string_get_used_length(string);
    const size_t total_units = total_bytes / 2;
    size_t unit_pos = 0;

    /* Skip cp_offset codepoints */
    for (size_t i = 0; i < cp_offset && unit_pos < total_units; i++) {
        const uint16_t u1 = c_read_u16(string + unit_pos * 2);
        unit_pos++;
        if (u1 >= 0xD800u && u1 <= 0xDBFFu && unit_pos < total_units) {
            const uint16_t u2 = c_read_u16(string + unit_pos * 2);
            if (u2 >= 0xDC00u && u2 <= 0xDFFFu) unit_pos++;
        }
    }

    if (unit_pos >= total_units && cp_offset > 0) return s;
    const size_t start_unit = unit_pos;

    /* Collect cp_count codepoints */
    for (size_t i = 0; i < cp_count && unit_pos < total_units; i++) {
        const uint16_t u1 = c_read_u16(string + unit_pos * 2);
        unit_pos++;
        if (u1 >= 0xD800u && u1 <= 0xDBFFu && unit_pos < total_units) {
            const uint16_t u2 = c_read_u16(string + unit_pos * 2);
            if (u2 >= 0xDC00u && u2 <= 0xDFFFu) unit_pos++;
        }
    }

    s.data   = string + start_unit * 2;
    s.length = (unit_pos - start_unit) * 2;
    return s;
}

/*
 * c_subslice_codepoints – universal dispatcher.
 * Calls the encoding-specific function based on the string's encoding tag,
 * then repackages the result as a c_slice_t.
 */
c_slice_t c_subslice_codepoints(c_const_string_t string,
                                 size_t cp_offset, size_t cp_count)
{
    c_slice_t result = { NULL, 0 };
    if (!string) return result;

    const unsigned char enc = c_string_get_encoding(string);

    if (enc == C_STRING_ENC_UTF16) {
        const c_utf16_slice_t u16 = c_utf16_subslice_codepoints(string, cp_offset, cp_count);
        result.data   = u16.data;
        result.length = u16.length;
        return result;
    }

    if (enc == C_STRING_ENC_UTF8) {
        const c_utf8_slice_t u8 = c_utf8_subslice_codepoints(string, cp_offset, cp_count);
        result.data   = u8.data;
        result.length = u8.length;
        return result;
    }

    /* ASCII / raw: each byte is one codepoint */
    const size_t total = c_string_get_used_length(string);
    if (cp_offset >= total) return result;
    const size_t remaining = total - cp_offset;
    result.data   = string + cp_offset;
    result.length = cp_count > remaining ? remaining : cp_count;
    return result;
}

size_t c_utf16_slice_unit_count(const c_utf16_slice_t slice)
{
    return slice.length / 2;
}

//----------------------------------------------------------------------------------
// String Lifecycle & Constructors
//----------------------------------------------------------------------------------

c_string_t c_string_create_pro(const void *const initial_data,
                                const size_t initial_length,
                                const unsigned char encoding)
{
    const unsigned char type        = c_string_determine_type(initial_length);
    const size_t        header_size = c_string_get_header_size(type);
    const size_t        null_bytes  = null_size_for(encoding);

    if (initial_length > SIZE_MAX - header_size - null_bytes) {
        errno = EOVERFLOW;
        return NULL;
    }

    uint8_t *const memory = (uint8_t *)C_STRING_MALLOC(header_size + initial_length + null_bytes);
    if (!memory) { errno = ENOMEM; return NULL; }

    c_string_t const string = memory + header_size;
    string[-1] = (unsigned char)(type | (encoding & C_STRING_ENC_MASK));

    if (initial_data) {
        memcpy(string, initial_data, initial_length);
    } else if (initial_length > 0) {
        memset(string, 0, initial_length);
    }

    c_string_set_lengths(string, initial_length, initial_length);

    /* Double-null guaranteed for UTF-16, single-null for everything else */
    string[initial_length] = '\0';
    if (null_bytes == 2) string[initial_length + 1] = '\0';

    return string;
}

/* FIX: was tagging with C_STRING_ENC_UTF8 – raw const char* should be ASCII. */
c_string_t c_string_create(const char *const string_parameter)
{
    return c_string_create_pro(
        string_parameter,
        string_parameter ? strlen(string_parameter) : 0,
        C_STRING_ENC_ASCII);
}

void c_string_free(const c_string_t string)
{
    if (!string) return;
    C_STRING_FREE((uint8_t *)string - c_string_get_header_size(string[-1]));
}

c_string_t c_string_duplicate(c_const_string_t const string)
{
    if (!string) return NULL;
    return c_string_create_pro(string,
                                c_string_get_used_length(string),
                                c_string_get_encoding(string));
}

void c_string_clear(const c_string_t string)
{
    if (!string) return;
    const unsigned char enc       = c_string_get_encoding(string);
    const size_t        null_bytes = null_size_for(enc);
    c_string_set_used_length(string, 0);
    string[0] = '\0';
    if (null_bytes == 2) string[1] = '\0';
}

bool c_string_is_empty(c_const_string_t const string)
{
    return !string || c_string_get_used_length(string) == 0;
}

//----------------------------------------------------------------------------------
// Slice Constructors
//----------------------------------------------------------------------------------

c_string_t c_string_from_slice(const c_slice_t slice)
{
    return c_string_create_pro(slice.data, slice.length, C_STRING_ENC_ASCII);
}

c_string_t c_string_from_utf8_slice(const c_utf8_slice_t slice)
{
    return c_string_create_pro(slice.data, slice.length, C_STRING_ENC_UTF8);
}

c_string_t c_string_from_utf16_slice(const c_utf16_slice_t slice)
{
    const size_t byte_len = slice.length & ~(size_t)1u; /* round down to even */
    return c_string_create_pro(slice.data, byte_len, C_STRING_ENC_UTF16);
}

c_string_t c_string_create_from_utf16(const uint16_t *const units, const size_t unit_count)
{
    if (!units && unit_count > 0) return NULL;
    if (unit_count > SIZE_MAX / 2) { errno = EOVERFLOW; return NULL; }
    return c_string_create_pro(units, unit_count * 2, C_STRING_ENC_UTF16);
}

//----------------------------------------------------------------------------------
// Capacity Management
//----------------------------------------------------------------------------------

c_string_t c_string_ensure_capacity(c_string_t string, const size_t additional_bytes)
{
    if (!string) return NULL;
    if (c_string_get_available_capacity(string) >= additional_bytes) return string;

    const size_t        current_length   = c_string_get_used_length(string);
    const unsigned char encoding         = c_string_get_encoding(string);
    const size_t        null_bytes       = null_size_for(encoding);
    const size_t        current_capacity =
        current_length + c_string_get_available_capacity(string);

    if (additional_bytes > SIZE_MAX - current_length) { errno = EOVERFLOW; return NULL; }
    const size_t needed = current_length + additional_bytes;

    /* Growth policy: double below 1 MB, add 1 MB chunks above */
    size_t new_capacity = needed;
    if (new_capacity < C_STRING_ONE_MEGABYTE) {
        const size_t doubled = (current_capacity > SIZE_MAX / 2)
                               ? SIZE_MAX
                               : current_capacity * 2;
        if (doubled > new_capacity) new_capacity = doubled;
    } else {
        if (SIZE_MAX - C_STRING_ONE_MEGABYTE >= new_capacity)
            new_capacity += C_STRING_ONE_MEGABYTE;
        else
            new_capacity = SIZE_MAX;
    }

    unsigned char new_type        = c_string_determine_type(new_capacity);
    size_t        new_header_size = c_string_get_header_size(new_type);

    /* Clamp if the combined size would overflow */
    if (new_capacity > SIZE_MAX - new_header_size - null_bytes) {
        new_capacity    = SIZE_MAX - new_header_size - null_bytes;
        new_type        = c_string_determine_type(new_capacity);
        new_header_size = c_string_get_header_size(new_type);
        if (new_capacity > SIZE_MAX - new_header_size - null_bytes ||
            new_capacity < needed)
        {
            errno = EOVERFLOW;
            return NULL;
        }
    }

    const unsigned char current_type        = (unsigned char)(string[-1] & C_STRING_TYPE_MASK);
    const size_t        current_header_size = c_string_get_header_size(current_type);
    uint8_t *new_memory;

    if (current_type == new_type) {
        /* Same header class → realloc in place */
        new_memory = (uint8_t *)C_STRING_REALLOC(
            string - current_header_size,
            new_header_size + new_capacity + null_bytes);
        if (!new_memory) { errno = ENOMEM; return NULL; }
        /* type_flags content is preserved by realloc */
        string = new_memory + new_header_size;
    } else {
        /* Header class changed → malloc + copy + free */
        new_memory = (uint8_t *)C_STRING_MALLOC(
            new_header_size + new_capacity + null_bytes);
        if (!new_memory) { errno = ENOMEM; return NULL; }
        memcpy(new_memory + new_header_size, string, current_length + null_bytes);
        C_STRING_FREE(string - current_header_size);   /* old memory freed here */
        string = new_memory + new_header_size;
        string[-1] = (unsigned char)(new_type | encoding); /* encoding bits preserved */
    }

    c_string_set_lengths(string, current_length, new_capacity);
    return string;
}

c_string_t c_string_reserve(c_string_t string, const size_t additional_bytes)
{
    return c_string_ensure_capacity(string, additional_bytes);
}

c_string_t c_string_shrink_to_fit(c_string_t string)
{
    if (!string) return NULL;

    const unsigned char type        = (unsigned char)(string[-1] & C_STRING_TYPE_MASK);
    const unsigned char enc         = c_string_get_encoding(string);
    const size_t        used        = c_string_get_used_length(string);
    const size_t        header_size = c_string_get_header_size(type);
    const size_t        null_bytes  = null_size_for(enc);

    const unsigned char new_type        = c_string_determine_type(used);
    const size_t        new_header_size = c_string_get_header_size(new_type);

    if (new_type == type) {
        uint8_t *const new_memory = (uint8_t *)C_STRING_REALLOC(
            string - header_size,
            header_size + used + null_bytes);
        if (!new_memory) return string; /* leave as-is on failure */
        string = new_memory + header_size;
    } else {
        uint8_t *const new_memory = (uint8_t *)C_STRING_MALLOC(
            new_header_size + used + null_bytes);
        if (!new_memory) return string;
        memcpy(new_memory + new_header_size, string, used + null_bytes);
        C_STRING_FREE(string - header_size);
        string = new_memory + new_header_size;
        string[-1] = (unsigned char)(new_type | enc);
    }

    c_string_set_lengths(string, used, used);
    /* Re-write terminators in case realloc moved / shortened the allocation */
    string[used] = '\0';
    if (null_bytes == 2) string[used + 1] = '\0';

    return string;
}

void c_string_array_shrink_to_fit(c_string_t *const array, const size_t count)
{
    if (!array || count == 0) return;
    for (size_t i = 0; i < count; i++) {
        if (array[i]) array[i] = c_string_shrink_to_fit(array[i]);
    }
}

//----------------------------------------------------------------------------------
// Appending & Concatenation
//----------------------------------------------------------------------------------

c_string_t c_string_append_pro(c_string_t string,
                                const void *const data,
                                const size_t length)
{
    if (!string || !data || length == 0) return string;

    const size_t        current_length = c_string_get_used_length(string);
    const unsigned char encoding       = c_string_get_encoding(string);
    const size_t        null_bytes     = null_size_for(encoding);

    if (length > SIZE_MAX - current_length) { errno = EOVERFLOW; return NULL; }

    /*
     * Self-overlap detection: capture the offset of `data` relative to the
     * string buffer BEFORE the potential realloc, then rebase afterwards.
     */
    const uintptr_t src_start  = (uintptr_t)data;
    const uintptr_t buf_start  = (uintptr_t)(string - c_string_get_header_size(string[-1]));
    const uintptr_t buf_end    = (uintptr_t)(string + current_length + null_bytes);
    const bool      is_overlap = (src_start >= buf_start && src_start < buf_end);
    const ptrdiff_t src_offset = is_overlap
                                 ? (const uint8_t *)data - string
                                 : 0;

    c_string_t tmp = c_string_ensure_capacity(string, length);
    if (!tmp) return NULL;
    string = tmp;

    /* If the buffer moved, rebase the source pointer */
    const void *safe_data = is_overlap ? (const void *)(string + src_offset) : data;
    memmove(string + current_length, safe_data, length);

    const size_t new_length = current_length + length;
    c_string_set_used_length(string, new_length);
    string[new_length] = '\0';
    if (null_bytes == 2) string[new_length + 1] = '\0';

    return string;
}

c_string_t c_string_append(c_string_t string, const char *const string_parameter)
{
    return c_string_append_pro(
        string,
        string_parameter,
        string_parameter ? strlen(string_parameter) : 0);
}

c_string_t c_string_append_slice(c_string_t string, const c_slice_t slice)
{
    if (!slice.data || slice.length == 0) return string;
    return c_string_append_pro(string, slice.data, slice.length);
}

/* FIX: auto-upgrade ASCII encoding → UTF-8 when UTF-8 content is appended. */
c_string_t c_string_append_utf8(c_string_t string, const c_utf8_slice_t slice)
{
    if (!slice.data || slice.length == 0) return string;
    if (c_string_get_encoding(string) == C_STRING_ENC_ASCII)
        c_string_set_encoding(string, C_STRING_ENC_UTF8);
    return c_string_append_pro(string, slice.data, slice.length);
}

c_string_t c_string_append_utf16(c_string_t string, const c_utf16_slice_t slice)
{
    if (!slice.data || slice.length == 0) return string;
    const size_t byte_len = slice.length & ~(size_t)1u; /* round down to even */
    if (byte_len == 0) return string;
    return c_string_append_pro(string, slice.data, byte_len);
}

c_string_t c_string_concat(c_const_string_t const a, c_const_string_t const b)
{
    const unsigned char enc_a = a ? c_string_get_encoding(a) : C_STRING_ENC_ASCII;
    const size_t la = a ? c_string_get_used_length(a) : 0;
    const size_t lb = b ? c_string_get_used_length(b) : 0;

    if (la > SIZE_MAX - lb) { errno = EOVERFLOW; return NULL; }

    c_string_t result = c_string_create_pro(a, la, enc_a);
    if (!result) return NULL;

    if (lb > 0) {
        c_string_t tmp = c_string_append_pro(result, b, lb);
        if (!tmp) { c_string_free(result); return NULL; }
        result = tmp;
    }
    return result;
}

//----------------------------------------------------------------------------------
// Encoding Converters – UTF-8 normalisation (utf8proc)
//----------------------------------------------------------------------------------

c_string_t c_string_create_from_utf8_pro(const char *const text,
                                          const utf8proc_option_t options)
{
    if (!text) return NULL;

    utf8proc_uint8_t *destination = NULL;
    /*
     * FIX: pass strlen=-1 as the conventional "use NULLTERM" sentinel.
     * When UTF8PROC_NULLTERM is set the strlen parameter is ignored by utf8proc,
     * but -1 makes the intent obvious at the call site.
     */
    const utf8proc_ssize_t length = utf8proc_map(
        (const utf8proc_uint8_t *)text,
        -1,
        &destination,
        options | UTF8PROC_NULLTERM);

    if (length < 0) {
        free(destination); /* utf8proc uses the standard allocator internally */
        errno = EILSEQ;
        return NULL;
    }

    c_string_t result = c_string_create_pro(destination, (size_t)length, C_STRING_ENC_UTF8);
    free(destination);
    return result;
}

c_string_t c_string_create_from_utf8(const char *const text)
{
    /* UTF8PROC_NULLTERM is redundant here (added in _pro) but harmless and self-documenting */
    return c_string_create_from_utf8_pro(
        text,
        (utf8proc_option_t)(UTF8PROC_NULLTERM | UTF8PROC_STABLE | UTF8PROC_COMPOSE));
}

//----------------------------------------------------------------------------------
// Encoding Converters – UTF-8 → UTF-16
//----------------------------------------------------------------------------------

c_string_t c_string_convert_utf8_utf16_pro(c_const_string_t const utf8_input,
                                            const bool use_big_endian)
{
    if (!utf8_input) return NULL;

    const size_t input_length = c_string_get_used_length(utf8_input);

    c_string_t output = c_string_create_pro(NULL, 0, C_STRING_ENC_UTF16);
    if (!output) return NULL;

    if (input_length > 0) {
        /* Worst-case: one ASCII byte → one UTF-16 unit (2 bytes) → ratio 2× */
        if (input_length > SIZE_MAX / 2) {
            c_string_free(output);
            errno = EOVERFLOW;
            return NULL;
        }
        c_string_t pre = c_string_ensure_capacity(output, input_length * 2);
        if (!pre) { c_string_free(output); return NULL; }
        output = pre;
    }

    const bool swap = needs_swap(use_big_endian);

    size_t offset = 0;
    while (offset < input_length) {
        utf8proc_int32_t cp;
        const size_t rem = input_length - offset;
        const utf8proc_ssize_t bound =
            (utf8proc_ssize_t)(rem > (size_t)SSIZE_MAX ? SSIZE_MAX : rem);
        const utf8proc_ssize_t read_bytes =
            utf8proc_iterate(utf8_input + offset, bound, &cp);

        if (read_bytes < 0) {
            c_string_free(output);
            errno = EILSEQ;
            return NULL;
        }
        offset += (size_t)read_bytes;

        /* Reject lone surrogates – they are not valid Unicode scalar values */
        if (cp >= 0xD800 && cp <= 0xDFFF) {
            c_string_free(output);
            errno = EILSEQ;
            return NULL;
        }

        uint16_t units[2];
        int      count = 0;

        if (cp < 0x10000) {
            units[count++] = (uint16_t)cp;
        } else {
            const int32_t adj = cp - 0x10000;
            units[count++] = (uint16_t)(0xD800u + (uint32_t)(adj >> 10));
            units[count++] = (uint16_t)(0xDC00u + (uint32_t)(adj & 0x3FFu));
        }

        if (swap) {
            for (int i = 0; i < count; i++) units[i] = bswap16(units[i]);
        }

        c_string_t tmp = c_string_append_pro(output, units, (size_t)count * 2);
        if (!tmp) { c_string_free(output); errno = ENOMEM; return NULL; }
        output = tmp;
    }

    return c_string_shrink_to_fit(output);
}

c_string_t c_string_convert_utf8_utf16(c_const_string_t const utf8_input)
{
    return c_string_convert_utf8_utf16_pro(utf8_input, false);
}

//----------------------------------------------------------------------------------
// Encoding Converters – UTF-16 → UTF-8
//----------------------------------------------------------------------------------

c_string_t c_string_convert_utf16_utf8_pro(c_const_string_t const utf16_input,
                                            const bool use_big_endian)
{
    if (!utf16_input) return NULL;

    const size_t         input_length = c_string_get_used_length(utf16_input);
    const size_t         input_units  = input_length / 2;

    c_string_t output = c_string_create_pro(NULL, 0, C_STRING_ENC_UTF8);
    if (!output) return NULL;

    const bool swap = needs_swap(use_big_endian);

    for (size_t i = 0; i < input_units; ) {
        uint16_t u1 = c_read_u16(utf16_input + i * 2);
        i++;
        if (swap) u1 = bswap16(u1);

        utf8proc_int32_t cp;

        if (u1 >= 0xD800u && u1 <= 0xDBFFu) {
            /* High surrogate – must be followed by a low surrogate */
            if (i < input_units) {
                uint16_t u2 = c_read_u16(utf16_input + i * 2);
                if (swap) u2 = bswap16(u2);

                if (u2 >= 0xDC00u && u2 <= 0xDFFFu) {
                    /* Valid surrogate pair → decode to codepoint */
                    cp = (utf8proc_int32_t)(0x10000u +
                         (((uint32_t)(u1 - 0xD800u) << 10) | (uint32_t)(u2 - 0xDC00u)));
                    i++;
                } else {
                    /* FIX: lone high surrogate (next unit is not a low surrogate)
                     * Emit U+FFFD (replacement character) instead of silently
                     * swallowing the codepoint or passing an invalid value to
                     * utf8proc_encode_char (which returns ≤0 for surrogates).
                     */
                    cp = 0xFFFD;
                }
            } else {
                /* High surrogate at end of stream */
                cp = 0xFFFD;
            }
        } else if (u1 >= 0xDC00u && u1 <= 0xDFFFu) {
            /* FIX: unexpected lone low surrogate → replacement character */
            cp = 0xFFFD;
        } else {
            cp = (utf8proc_int32_t)u1;
        }

        uint8_t utf8_buf[4];
        const utf8proc_ssize_t written = utf8proc_encode_char(cp, utf8_buf);
        if (written > 0) {
            c_string_t tmp = c_string_append_pro(output, utf8_buf, (size_t)written);
            if (!tmp) { c_string_free(output); return NULL; }
            output = tmp;
        }
    }

    return c_string_shrink_to_fit(output);
}

c_string_t c_string_convert_utf16_utf8(c_const_string_t const utf16_input)
{
    return c_string_convert_utf16_utf8_pro(utf16_input, false);
}

//----------------------------------------------------------------------------------
// UTF-16 stdout printing
//----------------------------------------------------------------------------------

void c_string_print_utf16(const c_utf16_slice_t slice)
{
    if (!slice.data || slice.length == 0) return;

    c_string_t tmp_utf16 = c_string_from_utf16_slice(slice);
    if (!tmp_utf16) return;

    c_string_t tmp_utf8 = c_string_convert_utf16_utf8(tmp_utf16);
    c_string_free(tmp_utf16);

    if (tmp_utf8) {
        fwrite(tmp_utf8, 1, c_string_get_used_length(tmp_utf8), stdout);
        c_string_free(tmp_utf8);
    }
}

//----------------------------------------------------------------------------------
// Case Conversion  (UTF-8 / ASCII)
//----------------------------------------------------------------------------------

c_string_t c_string_to_lower(c_const_string_t const string)
{
    if (!string) return NULL;
    const size_t        input_len = c_string_get_used_length(string);
    const unsigned char enc       = c_string_get_encoding(string);

    /* Only meaningful for byte-encoded strings */
    if (enc == C_STRING_ENC_UTF16) return NULL;

    utf8proc_uint8_t *dest = NULL;
    /*
     * UTF8PROC_CASEFOLD maps each codepoint to its case-fold equivalent (lowercase
     * or caseless), then STABLE + COMPOSE normalise to NFC.
     */
    const utf8proc_ssize_t len = utf8proc_map(
        (const utf8proc_uint8_t *)string,
        (utf8proc_ssize_t)input_len,
        &dest,
        (utf8proc_option_t)(UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_CASEFOLD));

    if (len < 0) { free(dest); errno = EILSEQ; return NULL; }
    c_string_t result = c_string_create_pro(dest, (size_t)len, C_STRING_ENC_UTF8);
    free(dest);
    return result;
}

/* Per-codepoint toupper callback for utf8proc_map_custom */
static utf8proc_int32_t _toupper_cb(utf8proc_int32_t cp, void *data)
{
    (void)data;
    return utf8proc_toupper(cp);
}

c_string_t c_string_to_upper(c_const_string_t const string)
{
    if (!string) return NULL;
    const size_t        input_len = c_string_get_used_length(string);
    const unsigned char enc       = c_string_get_encoding(string);

    if (enc == C_STRING_ENC_UTF16) return NULL;

    utf8proc_uint8_t *dest = NULL;
    /*
     * utf8proc_map_custom applies _toupper_cb to each codepoint before
     * normalisation; STABLE + COMPOSE gives NFC output.
     */
    const utf8proc_ssize_t len = utf8proc_map_custom(
        (const utf8proc_uint8_t *)string,
        (utf8proc_ssize_t)input_len,
        &dest,
        (utf8proc_option_t)(UTF8PROC_STABLE | UTF8PROC_COMPOSE),
        _toupper_cb, NULL);

    if (len < 0) { free(dest); errno = EILSEQ; return NULL; }
    c_string_t result = c_string_create_pro(dest, (size_t)len, C_STRING_ENC_UTF8);
    free(dest);
    return result;
}

//----------------------------------------------------------------------------------
// Searching & Comparison
//----------------------------------------------------------------------------------

int c_string_compare(c_const_string_t const a, c_const_string_t const b)
{
    if (a == b)    return 0;
    if (!a)        return -1;
    if (!b)        return  1;

    const size_t la = c_string_get_used_length(a);
    const size_t lb = c_string_get_used_length(b);
    const size_t min = la < lb ? la : lb;

    const int cmp = memcmp(a, b, min);
    if (cmp != 0) return cmp;
    if (la < lb)  return -1;
    if (la > lb)  return  1;
    return 0;
}

bool c_string_equal(c_const_string_t const a, c_const_string_t const b)
{
    if (a == b) return true;
    if (!a || !b) return false;
    const size_t la = c_string_get_used_length(a);
    if (la != c_string_get_used_length(b)) return false;
    return memcmp(a, b, la) == 0;
}

size_t c_string_find(c_const_string_t const haystack, c_const_string_t const needle)
{
    if (!haystack || !needle) return SIZE_MAX;

    const size_t hlen = c_string_get_used_length(haystack);
    const size_t nlen = c_string_get_used_length(needle);

    if (nlen == 0) return 0;           /* empty needle always matches at 0 */
    if (nlen > hlen) return SIZE_MAX;

    const size_t limit = hlen - nlen;
    for (size_t i = 0; i <= limit; i++) {
        if (memcmp(haystack + i, needle, nlen) == 0) return i;
    }
    return SIZE_MAX;
}

bool c_string_contains(c_const_string_t const str, c_const_string_t const needle)
{
    return c_string_find(str, needle) != SIZE_MAX;
}

bool c_string_starts_with(c_const_string_t const str, c_const_string_t const prefix)
{
    if (!str || !prefix) return false;
    const size_t plen = c_string_get_used_length(prefix);
    if (plen == 0) return true;
    const size_t slen = c_string_get_used_length(str);
    if (plen > slen) return false;
    return memcmp(str, prefix, plen) == 0;
}

bool c_string_ends_with(c_const_string_t const str, c_const_string_t const suffix)
{
    if (!str || !suffix) return false;
    const size_t sflen = c_string_get_used_length(suffix);
    if (sflen == 0) return true;
    const size_t slen = c_string_get_used_length(str);
    if (sflen > slen) return false;
    return memcmp(str + slen - sflen, suffix, sflen) == 0;
}

//----------------------------------------------------------------------------------
// In-Place Mutation  (trim, repeat)
//----------------------------------------------------------------------------------

/* Return true when a UTF-16 LE code unit at the given BYTE offset is an ASCII
 * whitespace character (only considers BMP whitespace, not full Unicode space set). */
static inline bool is_utf16le_whitespace_at(const uint8_t *buf, size_t byte_off)
{
    const uint16_t u = c_read_u16(buf + byte_off);
    return u == 0x0020u /* space      */
        || u == 0x0009u /* tab        */
        || u == 0x000Au /* line feed  */
        || u == 0x000Du /* carriage return */;
}

c_string_t c_string_trim_right(c_string_t const string)
{
    if (!string) return NULL;
    const unsigned char enc       = c_string_get_encoding(string);
    const size_t        null_bytes = null_size_for(enc);
    size_t len = c_string_get_used_length(string);

    if (enc == C_STRING_ENC_UTF16) {
        while (len >= 2 && is_utf16le_whitespace_at(string, len - 2))
            len -= 2;
    } else {
        while (len > 0) {
            const uint8_t c = string[len - 1];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') len--;
            else break;
        }
    }

    c_string_set_used_length(string, len);
    string[len] = '\0';
    if (null_bytes == 2) string[len + 1] = '\0';
    return string;
}

c_string_t c_string_trim_left(c_string_t const string)
{
    if (!string) return NULL;
    const unsigned char enc       = c_string_get_encoding(string);
    const size_t        null_bytes = null_size_for(enc);
    const size_t        len        = c_string_get_used_length(string);
    size_t start = 0;

    if (enc == C_STRING_ENC_UTF16) {
        while (start + 1 < len && is_utf16le_whitespace_at(string, start))
            start += 2;
    } else {
        while (start < len) {
            const uint8_t c = string[start];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') start++;
            else break;
        }
    }

    if (start > 0) {
        const size_t new_len = len - start;
        memmove(string, string + start, new_len);
        c_string_set_used_length(string, new_len);
        string[new_len] = '\0';
        if (null_bytes == 2) string[new_len + 1] = '\0';
    }
    return string;
}

c_string_t c_string_trim(c_string_t const string)
{
    return c_string_trim_right(c_string_trim_left(string));
}

c_string_t c_string_repeat(c_const_string_t const string, const size_t count)
{
    const unsigned char enc = string ? c_string_get_encoding(string) : C_STRING_ENC_ASCII;

    if (!string || count == 0)
        return c_string_create_pro(NULL, 0, enc);

    const size_t unit_len = c_string_get_used_length(string);
    if (unit_len == 0)
        return c_string_create_pro(NULL, 0, enc);

    if (unit_len > SIZE_MAX / count) { errno = EOVERFLOW; return NULL; }
    const size_t total = unit_len * count;

    /*
     * Allocate a zero-filled buffer of `total` bytes with the correct encoding
     * and null terminator(s), then overwrite with repeated copies.
     */
    c_string_t result = c_string_create_pro(NULL, total, enc);
    if (!result) return NULL;

    for (size_t i = 0; i < count; i++)
        memcpy(result + i * unit_len, string, unit_len);

    /* Null terminator(s) were already written by c_string_create_pro */
    return result;
}

//----------------------------------------------------------------------------------
// Validation
//----------------------------------------------------------------------------------

bool c_string_validate_utf8(c_const_string_t const string)
{
    if (!string) return true;
    const size_t total = c_string_get_used_length(string);
    size_t pos = 0;
    while (pos < total) {
        utf8proc_int32_t cp;
        const size_t rem = total - pos;
        const utf8proc_ssize_t bound =
            (utf8proc_ssize_t)(rem > (size_t)SSIZE_MAX ? SSIZE_MAX : rem);
        const utf8proc_ssize_t adv = utf8proc_iterate((const uint8_t *)(string + pos), bound, &cp);
        if (adv <= 0 || cp < 0) return false;   /* invalid sequence */
        pos += (size_t)adv;
    }
    return true;
}
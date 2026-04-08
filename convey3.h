/*******************************************************************************************
*
*   convey v1.1  -  Dynamic string library with first-class encoding support (C99)
*
*   DEPENDENCIES
*       utf8proc.h  – UTF-8 processing, normalisation, and codepoint operations
*       <stdlib.h>  – malloc / realloc / free  (included below)
*
*   ENCODING MODEL
*   --------------
*   Every convey string carries an encoding tag packed into bits 3-4 of its type_flags
*   byte (the byte at index -1 relative to the character buffer pointer).
*
*       C_STRING_ENC_ASCII  (0x00)   Raw bytes / ASCII     – 1-byte null terminator
*       C_STRING_ENC_UTF8   (0x08)   UTF-8                 – 1-byte null terminator
*       C_STRING_ENC_UTF16  (0x10)   UTF-16 LE  (default)  – 2-byte null terminator
*
*   RULES
*   -----
*   · c_string_create()           defaults to C_STRING_ENC_ASCII.
*   · c_string_create_from_utf8() tags the result C_STRING_ENC_UTF8 (NFC-normalises).
*   · c_string_append_utf8()      auto-upgrades ASCII → UTF-8 on the destination.
*   · UTF-16 strings are ALWAYS double-null-terminated.
*   · used_length and allocated_capacity are always in BYTES.
*     For UTF-16 they are always even.
*   · Use C_STRING_NULL_SIZE(enc) wherever a null-terminator byte-count is needed
*     instead of hardcoding 1 or 2.
*
*   PRINT FORMAT MACROS
*   -------------------
*   ASCII / UTF-8 strings:
*       printf("%.*s",   C_PRINT_FORMAT(my_str));        // whole string
*       printf("%.*s",   C_SLICE_FORMAT(my_slice));       // c_slice_t / c_utf8_slice_t
*
*   UTF-16 on Windows (wchar_t == uint16_t):
*       wprintf(L"%.*ls", C_UTF16_WPRINT_FORMAT(my_utf16_str));         // whole string
*       wprintf(L"%.*ls", C_UTF16_WPRINT_FORMAT_SLICE(my_slice));       // slice
*
*   Cross-platform UTF-16 printing (converts to UTF-8 first):
*       c_string_print_utf16(C_UTF16_SLICE_OF(my_str));
*
*   QUICK EXAMPLE
*   -------------
*       c_string_t s = c_string_create("hello");
*       s = c_string_append(s, " world");
*       printf("%.*s\n", C_PRINT_FORMAT(s));   // hello world
*       c_string_free(s);
*
********************************************************************************************
*
*   zlib/libpng license  –  Copyright (c) 2024 convey contributors
*
********************************************************************************************/

#ifndef CONVEY_H
#define CONVEY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>   /* malloc / realloc / free  – needed by the hook macros below */
#include <wchar.h>
#include "utf8proc.h"

//----------------------------------------------------------------------------------
// Custom Allocator Hooks
// Define before including this header to redirect all allocations.
//----------------------------------------------------------------------------------
#ifndef C_STRING_MALLOC
    #define C_STRING_MALLOC(sz)         malloc(sz)
#endif
#ifndef C_STRING_REALLOC
    #define C_STRING_REALLOC(p, sz)     realloc((p), (sz))
#endif
#ifndef C_STRING_FREE
    #define C_STRING_FREE(p)            free(p)
#endif

//----------------------------------------------------------------------------------
// Internal Type-flags Byte  (bits 0-2 = size class, bits 3-4 = encoding)
//----------------------------------------------------------------------------------
#define C_STRING_TYPE_8             1u
#define C_STRING_TYPE_16            2u
#define C_STRING_TYPE_32            3u
#define C_STRING_TYPE_64            4u
#define C_STRING_TYPE_MASK          0x07u

#define C_STRING_ENC_ASCII          0x00u   /* raw bytes / ASCII                  */
#define C_STRING_ENC_UTF8           0x08u   /* UTF-8                              */
#define C_STRING_ENC_UTF16          0x10u   /* UTF-16 LE  (double-null term.)     */
#define C_STRING_ENC_MASK           0x18u

#define C_STR_FMT              "%.*s"        //    printf("Result: "C_STR_FMT" %d \n", C_PRINT_FORMAT(myStr), 10 );  Result: Hello convey! 10 

//----------------------------------------------------------------------------------
// Encoding Helpers
//----------------------------------------------------------------------------------

/** Null-terminator byte count for a given encoding constant (1 for ASCII/UTF-8, 2 for UTF-16). */
#define C_STRING_NULL_SIZE(enc)         (((enc) & C_STRING_ENC_MASK) == C_STRING_ENC_UTF16 ? 2u : 1u)

/** Extract the encoding tag (C_STRING_ENC_*) stored in a live string. */
#define C_STRING_ENC_OF(str)            c_string_get_encoding(str)

/** True when the string is tagged C_STRING_ENC_ASCII. */
#define C_STRING_IS_ASCII(str)          (c_string_get_encoding(str) == C_STRING_ENC_ASCII)

/** True when the string is tagged C_STRING_ENC_UTF8. */
#define C_STRING_IS_UTF8(str)           (c_string_get_encoding(str) == C_STRING_ENC_UTF8)

/** True when the string is tagged C_STRING_ENC_UTF16. */
#define C_STRING_IS_UTF16(str)          (c_string_get_encoding(str) == C_STRING_ENC_UTF16)

/** True when the string uses single-byte units (ASCII or UTF-8, i.e. not UTF-16). */
#define C_STRING_IS_BYTE_ENC(str)       (!C_STRING_IS_UTF16(str))

//----------------------------------------------------------------------------------
// Growth Sentinel
//----------------------------------------------------------------------------------
#define C_STRING_ONE_MEGABYTE           1048576u

//----------------------------------------------------------------------------------
// Header Access Macros  (internal – prefer the public API)
//----------------------------------------------------------------------------------
#define GET_HEADER_8(ptr)        ((struct c_string_header_8  *)((ptr) - sizeof(struct c_string_header_8)))
#define GET_HEADER_16(ptr)       ((struct c_string_header_16 *)((ptr) - sizeof(struct c_string_header_16)))
#define GET_HEADER_32(ptr)       ((struct c_string_header_32 *)((ptr) - sizeof(struct c_string_header_32)))
#define GET_HEADER_64(ptr)       ((struct c_string_header_64 *)((ptr) - sizeof(struct c_string_header_64)))

#define GET_CONST_HEADER_8(ptr)  ((const struct c_string_header_8  *)((ptr) - sizeof(struct c_string_header_8)))
#define GET_CONST_HEADER_16(ptr) ((const struct c_string_header_16 *)((ptr) - sizeof(struct c_string_header_16)))
#define GET_CONST_HEADER_32(ptr) ((const struct c_string_header_32 *)((ptr) - sizeof(struct c_string_header_32)))
#define GET_CONST_HEADER_64(ptr) ((const struct c_string_header_64 *)((ptr) - sizeof(struct c_string_header_64)))

//----------------------------------------------------------------------------------
// printf / wprintf Format Helpers
//----------------------------------------------------------------------------------

/*
 * ASCII / UTF-8  →  printf("%.*s", ...)
 * ─────────────────────────────────────
 * C_PRINT_FORMAT(str)       whole convey string  →  (int len, const char *ptr)
 * C_SLICE_FORMAT(slc)       any byte-view slice  →  (int len, const char *ptr)
 * C_UTF8_SLICE_FORMAT(slc)  UTF-8 slice alias
 *
 * Usage:
 *   printf("value: %.*s\n", C_PRINT_FORMAT(my_str));
 *   printf("slice: %.*s\n", C_SLICE_FORMAT(my_slice));
 */
#define C_PRINT_FORMAT(str) \
    (int)c_string_get_used_length(str), ((str) ? (const char *)(str) : "")

#define C_SLICE_FORMAT(slc) \
    (int)(slc).length, ((slc).data ? (const char *)(slc).data : "")

/** Alias of C_SLICE_FORMAT kept for self-documenting code with UTF-8 slices. */
#define C_UTF8_SLICE_FORMAT(slc)   C_SLICE_FORMAT(slc)

/** Raw byte dump of a UTF-16 slice (diagnostic use; produces non-printable bytes on ASCII terminals). */
#define C_UTF16_BYTE_FORMAT(slc)   C_SLICE_FORMAT(slc)

/*
 * UTF-16  →  wprintf(L"%.*ls", ...)   Windows / wchar_t == uint16_t only
 * ────────────────────────────────────────────────────────────────────────
 * The guard below ensures this block is a hard compile error on platforms
 * where wchar_t is 32-bit (Linux/macOS), where a raw cast of UTF-16 bytes to
 * wchar_t* would silently produce garbage output.
 *
 * C_UTF16_WPRINT_FORMAT(str)       whole convey string  →  (int units, const wchar_t *ptr)
 * C_UTF16_WPRINT_FORMAT_SLICE(slc) c_utf16_slice_t      →  (int units, const wchar_t *ptr)
 *
 * Usage:
 *   wprintf(L"value: %.*ls\n", C_UTF16_WPRINT_FORMAT(my_utf16_str));
 */
#if WCHAR_MAX == 0xFFFFu || defined(_WIN32)
#  define C_UTF16_WPRINT_FORMAT(str) \
       (int)(c_string_get_used_length(str) / 2u), \
       ((str) ? (const wchar_t *)(str) : L"")

#  define C_UTF16_WPRINT_FORMAT_SLICE(slc) \
       (int)((slc).length / 2u), \
       ((slc).data ? (const wchar_t *)(slc).data : L"")
#endif

/** Reinterpret a convey UTF-16 string pointer as const uint16_t*. */
#define C_STRING_AS_U16(str)   ((const uint16_t *)(str))

//----------------------------------------------------------------------------------
// Slice Compound-Literal Helpers
//----------------------------------------------------------------------------------

/** View the entire string as a generic byte slice (c_slice_t). */
#define C_SLICE_OF(str)                  ((c_slice_t){ (str), c_string_get_used_length(str) })

/** Extract a byte-offset/length subslice (generic). */
#define C_SUBSLICE(str, off, len)        c_subslice((str), (off), (len))

/**
 * Codepoint-indexed subslice (universal dispatcher).
 * Automatically handles ASCII, UTF-8, and UTF-16 based on the string's encoding tag.
 * Returns a zero-copy view into the source buffer.
 */
#define C_SUBSLICE_CP(str, off, cnt)     c_subslice_codepoints((str), (off), (cnt))

/** View the entire string as a c_utf8_slice_t. */
#define C_UTF8_SLICE_OF(str)             ((c_utf8_slice_t){ (str), c_string_get_used_length(str) })

/** UTF-8 subslice by byte offset + byte length. */
#define C_UTF8_SUBSLICE(str, o, l)       c_utf8_subslice((str), (o), (l))

/** UTF-8 subslice by codepoint offset + count. */
#define C_UTF8_SUBSLICE_CP(str, o, cnt)  c_utf8_subslice_codepoints((str), (o), (cnt))

/** View the entire string as a c_utf16_slice_t (length in bytes, always even). */
#define C_UTF16_SLICE_OF(str)            ((c_utf16_slice_t){ (str), c_string_get_used_length(str) })

/** UTF-16 subslice by byte offset + byte length (both rounded down to even). */
#define C_UTF16_SUBSLICE(str, o, l)      c_utf16_subslice((str), (o), (l))

/** UTF-16 subslice by codepoint offset + count (surrogate-pair-aware). */
#define C_UTF16_SUBSLICE_CP(str, o, cnt) c_utf16_subslice_codepoints((str), (o), (cnt))

//----------------------------------------------------------------------------------
// Types and Structures
//----------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

/** Mutable dynamic string.  The bytes before this pointer are the header. */
typedef uint8_t       *c_string_t;

/** Immutable dynamic string. */
typedef const uint8_t *c_const_string_t;

/** Generic byte-view slice (ASCII / raw bytes). */
typedef struct { const uint8_t *data; size_t length; } c_slice_t;

/** UTF-8 byte-view slice. */
typedef struct { const uint8_t *data; size_t length; } c_utf8_slice_t;

/** UTF-16 byte-view slice (length is in BYTES, always even). */
typedef struct { const uint8_t *data; size_t length; } c_utf16_slice_t;

/* ---- Packed variable-width headers ----
 *
 * Layout (packed, little-endian):
 *   [ used_length | allocated_capacity | type_flags ] [ ... data ... ] [ NUL(s) ]
 *                                            ^
 *                                       string[-1]  ← always the type_flags byte
 *
 * allocated_capacity and used_length are in BYTES and exclude the null terminator(s).
 */
#if defined(_MSC_VER)
#  pragma pack(push, 1)
#  define C_STRING_PACKED
#elif defined(__GNUC__) || defined(__clang__)
#  define C_STRING_PACKED __attribute__((__packed__))
#else
#  define C_STRING_PACKED
#endif

struct C_STRING_PACKED c_string_header_8  { uint8_t  used_length; uint8_t  allocated_capacity; unsigned char type_flags; uint8_t character_buffer[]; };
struct C_STRING_PACKED c_string_header_16 { uint16_t used_length; uint16_t allocated_capacity; unsigned char type_flags; uint8_t character_buffer[]; };
struct C_STRING_PACKED c_string_header_32 { uint32_t used_length; uint32_t allocated_capacity; unsigned char type_flags; uint8_t character_buffer[]; };
struct C_STRING_PACKED c_string_header_64 { uint64_t used_length; uint64_t allocated_capacity; unsigned char type_flags; uint8_t character_buffer[]; };

#if defined(_MSC_VER)
#  pragma pack(pop)
#endif

//====================================================================================
//
//   MODULE FUNCTIONS
//
//====================================================================================

/* --------------------------------------------------------------------------
   String Lifecycle & Constructors
   -------------------------------------------------------------------------- */

/**
 * Create a new dynamic string from a NUL-terminated C string.
 * Tagged C_STRING_ENC_ASCII (the default encoding).
 * Use c_string_create_from_utf8() when the content contains multi-byte UTF-8.
 */
c_string_t c_string_create(const char *string_parameter);

/**
 * Low-level constructor.  Copies `initial_length` bytes from `initial_data` into
 * a new allocation tagged with `encoding` (C_STRING_ENC_ASCII / UTF8 / UTF16).
 * Pass NULL for `initial_data` to get a zero-filled buffer of `initial_length` bytes.
 * For UTF-16, `initial_length` must be in bytes (even count).
 */
c_string_t c_string_create_pro(const void *initial_data, size_t initial_length, unsigned char encoding);

/** Free a dynamic string.  Safe to call with NULL. */
void c_string_free(c_string_t string);

/**
 * Deep-copy a dynamic string, preserving encoding, length, and content.
 * Returns NULL on allocation failure.
 */
c_string_t c_string_duplicate(c_const_string_t string);

/** Reset used-length to 0 and write the appropriate null terminator(s). */
void c_string_clear(c_string_t string);

/** Return true when the string is NULL or has zero used bytes. */
bool c_string_is_empty(c_const_string_t string);

/* --------------------------------------------------------------------------
   Slice Constructors
   -------------------------------------------------------------------------- */

/** Create a new ASCII-tagged string from a raw byte slice. */
c_string_t c_string_from_slice(c_slice_t slice);

/** Create a new UTF-8-tagged string from a UTF-8 byte slice. */
c_string_t c_string_from_utf8_slice(c_utf8_slice_t slice);

/**
 * Create a new UTF-16-tagged string from a UTF-16 byte slice.
 * slice.length must be in bytes (odd lengths are rounded down to the nearest even value).
 * The result is double-null-terminated.
 */
c_string_t c_string_from_utf16_slice(c_utf16_slice_t slice);

/**
 * Create a new UTF-16-tagged string from an array of `unit_count` uint16_t code units.
 * The result is double-null-terminated.
 */
c_string_t c_string_create_from_utf16(const uint16_t *units, size_t unit_count);

/* --------------------------------------------------------------------------
   Appending & Concatenation
   -------------------------------------------------------------------------- */

/**
 * Append a NUL-terminated C string to a dynamic string.
 * The caller MUST reassign from the return value (may change on realloc).
 */
c_string_t c_string_append(c_string_t string, const char *string_parameter);

/**
 * Append `length` raw bytes from `data` to a dynamic string.
 * Self-overlap is handled safely (e.g. appending a subslice of the same string).
 * The caller MUST reassign from the return value.
 */
c_string_t c_string_append_pro(c_string_t string, const void *data, size_t length);

/** Append a raw byte slice to a dynamic string. */
c_string_t c_string_append_slice(c_string_t string, c_slice_t slice);

/**
 * Append a UTF-8 slice to a dynamic string.
 * If the destination is ASCII-tagged it is automatically upgraded to UTF-8.
 */
c_string_t c_string_append_utf8(c_string_t string, c_utf8_slice_t slice);

/**
 * Append a UTF-16 slice (raw bytes) to a dynamic string.
 * Odd byte lengths are rounded down to the nearest even value before appending.
 */
c_string_t c_string_append_utf16(c_string_t string, c_utf16_slice_t slice);

/**
 * Create a new string that is the concatenation of `a` followed by `b`.
 * The result inherits the encoding of `a`.  Concatenating strings of different
 * encodings produces raw-byte output – convert to a common encoding first.
 * Returns NULL on allocation failure.
 */
c_string_t c_string_concat(c_const_string_t a, c_const_string_t b);

/* --------------------------------------------------------------------------
   Capacity Management
   -------------------------------------------------------------------------- */

/**
 * Ensure at least `additional_bytes` of free space beyond the current used length.
 * Growth policy: double below 1 MB, add 1 MB chunks above 1 MB.
 * The caller MUST reassign from the return value.
 */
c_string_t c_string_ensure_capacity(c_string_t string, size_t additional_bytes);

/** Pre-allocate at least `additional_bytes` of extra space.  Alias of ensure_capacity. */
c_string_t c_string_reserve(c_string_t string, size_t additional_bytes);

/**
 * Shrink the allocation to exactly fit the current used length + null terminator(s).
 * The caller MUST reassign from the return value.
 */
c_string_t c_string_shrink_to_fit(c_string_t string);

/** Shrink every non-NULL entry in `array` in place. */
void c_string_array_shrink_to_fit(c_string_t *array, size_t count);

/* --------------------------------------------------------------------------
   Slice Extractors
   -------------------------------------------------------------------------- */

/** View the entire string as a raw byte slice. */
c_slice_t c_slice_of(c_const_string_t string);

/** Extract a subslice by byte offset + byte length (clamped to string bounds). */
c_slice_t c_subslice(c_const_string_t string, size_t byte_offset, size_t byte_length);

/**
 * Extract a subslice by logical codepoint offset + count.
 * Dispatches automatically on the string's encoding tag:
 *   ASCII  – each byte is one codepoint.
 *   UTF-8  – walks multi-byte sequences via utf8proc_iterate.
 *   UTF-16 – accounts for surrogate pairs.
 * Returns a zero-copy view; the result is 2-byte-aligned for UTF-16.
 */
c_slice_t c_subslice_codepoints(c_const_string_t string, size_t cp_offset, size_t cp_count);

/** View the entire string as a UTF-8 byte slice. */
c_utf8_slice_t c_utf8_slice_of(c_const_string_t string);

/** Extract a UTF-8 subslice by byte offset + byte length. */
c_utf8_slice_t c_utf8_subslice(c_const_string_t string, size_t byte_offset, size_t byte_length);

/** Extract a UTF-8 subslice by codepoint offset + count (uses utf8proc_iterate). */
c_utf8_slice_t c_utf8_subslice_codepoints(c_const_string_t string, size_t cp_offset, size_t cp_count);

/** View the entire string as a UTF-16 byte slice (length in bytes, always even). */
c_utf16_slice_t c_utf16_slice_of(c_const_string_t string);

/** Extract a UTF-16 subslice by byte offset + byte length (both rounded down to even). */
c_utf16_slice_t c_utf16_subslice(c_const_string_t string, size_t byte_offset, size_t byte_length);

/** Extract a UTF-16 subslice by codepoint offset + count (surrogate-pair-aware). */
c_utf16_slice_t c_utf16_subslice_codepoints(c_const_string_t string, size_t cp_offset, size_t cp_count);

/** Return the number of UTF-16 code units (uint16_t values) in a slice. */
size_t c_utf16_slice_unit_count(c_utf16_slice_t slice);

/* --------------------------------------------------------------------------
   Encoding Converters
   -------------------------------------------------------------------------- */

/**
 * Validate and NFC-normalise a NUL-terminated UTF-8 string.
 * Returns a new convey string tagged C_STRING_ENC_UTF8.
 * Sets errno=EILSEQ and returns NULL on invalid UTF-8.
 */
c_string_t c_string_create_from_utf8(const char *text);

/**
 * Same as c_string_create_from_utf8() but exposes full utf8proc option flags.
 * UTF8PROC_NULLTERM is always added internally so the input must be NUL-terminated.
 */
c_string_t c_string_create_from_utf8_pro(const char *text, utf8proc_option_t options);

/**
 * Convert a convey UTF-8 (or ASCII) string to a new convey UTF-16 LE string.
 * Result is tagged C_STRING_ENC_UTF16 and double-null-terminated.
 * Returns NULL (errno=EILSEQ) on invalid UTF-8 or lone surrogates in the input.
 */
c_string_t c_string_convert_utf8_utf16(c_const_string_t utf8_input);

/**
 * Same as c_string_convert_utf8_utf16() but with selectable output endianness.
 * Pass use_big_endian=true for UTF-16 BE (Java / network protocols).
 */
c_string_t c_string_convert_utf8_utf16_pro(c_const_string_t utf8_input, bool use_big_endian);

/**
 * Convert a convey UTF-16 LE string to a new convey UTF-8 string.
 * Lone surrogates are replaced with U+FFFD (Unicode replacement character).
 * Result is tagged C_STRING_ENC_UTF8.
 */
c_string_t c_string_convert_utf16_utf8(c_const_string_t utf16_input);

/**
 * Same as c_string_convert_utf16_utf8() but with selectable source endianness.
 * Pass use_big_endian=true when the UTF-16 data was produced as BE.
 */
c_string_t c_string_convert_utf16_utf8_pro(c_const_string_t utf16_input, bool use_big_endian);

/**
 * Print a UTF-16 slice to stdout by converting to UTF-8 first.
 * Safe on any platform (Linux / macOS / Windows terminal).
 *
 *   c_string_print_utf16(C_UTF16_SLICE_OF(my_str));
 *   c_string_print_utf16(C_UTF16_SUBSLICE_CP(my_str, 0, 10));
 */
void c_string_print_utf16(c_utf16_slice_t slice);

/* --------------------------------------------------------------------------
   Case Conversion  (UTF-8 / ASCII strings)
   -------------------------------------------------------------------------- */

/**
 * Return a new NFC-normalised, case-folded (lowercase) copy.
 * Uses Unicode full case-folding via utf8proc (UTF8PROC_CASEFOLD).
 * Input must be UTF-8 or ASCII tagged; returns NULL on failure.
 * Result is tagged C_STRING_ENC_UTF8.
 */
c_string_t c_string_to_lower(c_const_string_t string);

/**
 * Return a new NFC-normalised, uppercased copy.
 * Uses utf8proc_toupper() per codepoint via utf8proc_map_custom().
 * Input must be UTF-8 or ASCII tagged; returns NULL on failure.
 * Result is tagged C_STRING_ENC_UTF8.
 */
c_string_t c_string_to_upper(c_const_string_t string);

/* --------------------------------------------------------------------------
   Searching & Comparison
   -------------------------------------------------------------------------- */

/**
 * Lexicographic bytewise comparison.
 * Returns <0, 0, or >0.  NULL sorts before any non-NULL string.
 * For NFC UTF-8 strings this matches Unicode codepoint order.
 */
int c_string_compare(c_const_string_t a, c_const_string_t b);

/**
 * Return true when `a` and `b` have identical byte content and length.
 * Encoding tags are NOT compared; use C_STRING_ENC_OF() to check them explicitly.
 */
bool c_string_equal(c_const_string_t a, c_const_string_t b);

/**
 * Find the first occurrence of `needle` inside `haystack` (bytewise search).
 * Returns the byte offset on success, or SIZE_MAX when not found.
 * An empty needle (zero used_length) always returns 0.
 */
size_t c_string_find(c_const_string_t haystack, c_const_string_t needle);

/**
 * Return true when `needle` occurs anywhere inside `str`.
 * Equivalent to (c_string_find(str, needle) != SIZE_MAX).
 */
bool c_string_contains(c_const_string_t str, c_const_string_t needle);

/**
 * Return true when `str` begins with the same bytes as `prefix`.
 * An empty prefix always returns true.
 */
bool c_string_starts_with(c_const_string_t str, c_const_string_t prefix);

/**
 * Return true when `str` ends with the same bytes as `suffix`.
 * An empty suffix always returns true.
 */
bool c_string_ends_with(c_const_string_t str, c_const_string_t suffix);

/* --------------------------------------------------------------------------
   In-Place Mutation
   -------------------------------------------------------------------------- */

/**
 * Strip ASCII whitespace (' ', '\t', '\n', '\r') from the right end.
 * For UTF-16 strings the corresponding UTF-16 LE code units are stripped.
 * Operates in-place; returns the same pointer (never reallocates).
 */
c_string_t c_string_trim_right(c_string_t string);

/**
 * Strip ASCII whitespace from the left end using memmove.
 * For UTF-16 the corresponding UTF-16 LE code units are stripped.
 * Operates in-place; returns the same pointer (never reallocates).
 */
c_string_t c_string_trim_left(c_string_t string);

/**
 * Strip ASCII whitespace from both ends.
 * Equivalent to c_string_trim_right(c_string_trim_left(string)).
 * Operates in-place; returns the same pointer.
 */
c_string_t c_string_trim(c_string_t string);

/**
 * Create a new string that repeats `string` exactly `count` times.
 * Encoding is inherited from the source string.
 * Returns an empty string for count==0, NULL on overflow / allocation failure.
 */
c_string_t c_string_repeat(c_const_string_t string, size_t count);

/* --------------------------------------------------------------------------
   Validation
   -------------------------------------------------------------------------- */

/**
 * Return true when every byte sequence in `string` is valid UTF-8.
 * Uses utf8proc_iterate() for each codepoint.
 * Always returns true for NULL.
 */
bool c_string_validate_utf8(c_const_string_t string);

/* --------------------------------------------------------------------------
   Internal Getters & Setters
   -------------------------------------------------------------------------- */

/** Return the byte size of the header for a given type_flags byte. */
size_t c_string_get_header_size(unsigned char flags);

/** Choose the smallest header class (8/16/32/64) that can store a value of `size`. */
unsigned char c_string_determine_type(size_t size);

/**
 * Return the encoding tag (C_STRING_ENC_ASCII / UTF8 / UTF16) from the header.
 * Returns C_STRING_ENC_ASCII for NULL.
 */
unsigned char c_string_get_encoding(c_const_string_t string);

/**
 * Retag the string's encoding byte in-place WITHOUT converting any data.
 * Use carefully – only when you know the byte content already matches the new
 * encoding (e.g. after validating that a UTF-8-tagged string is pure ASCII).
 */
void c_string_set_encoding(c_string_t string, unsigned char enc);

/** Return the current used byte count (excludes null terminator(s)). */
size_t c_string_get_used_length(c_const_string_t string);

/** Return the spare capacity in bytes (allocated_capacity – used_length, excludes null bytes). */
size_t c_string_get_available_capacity(c_const_string_t string);

/** Return the total allocated data capacity in bytes (used + spare, excludes null bytes). */
size_t c_string_get_capacity(c_const_string_t string);

/** Count logical codepoints, respecting the string's encoding tag. */
size_t c_string_codepoint_count(c_const_string_t string);

/**
 * Force both the used-length and allocated-capacity fields in the header.
 * Does NOT write null terminators.  Use c_string_append_pro() for safe appends.
 */
void c_string_set_lengths(c_string_t string, size_t used, size_t capacity);

/** Force only the used-length field in the header. */
void c_string_set_used_length(c_string_t string, size_t used);

#ifdef __cplusplus
}
#endif

#endif /* CONVEY_H */
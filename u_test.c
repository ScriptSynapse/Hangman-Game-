/*******************************************************************************************
*
* convey v1.1 - Massive Beginner Examples Collection
*
* This file contains a comprehensive list of tiny, isolated examples for the 
* 'convey' dynamic string library. 
* * RULE: ONE CONCEPT PER EXAMPLE! No confusing combinations.
*
* DEPENDENCIES:
* convey3.h / convey_main3.c (and utf8proc.h)
*
********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// Include the convey library header
#include "convey_main3.c"
//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// --- 1. Creation & Destruction ---
void ExampleStringCreate(void);
void ExampleStringCreatePro(void);
void ExampleStringDuplicate(void);
void ExampleStringRepeat(void);
void ExampleStringFree(void);

// --- 2. Information & Properties ---
void ExampleStringIsEmpty(void);
void ExampleStringGetUsedLength(void);
void ExampleStringGetCapacity(void);
void ExampleStringGetAvailableCapacity(void);

// --- 3. Memory Management ---
void ExampleStringEnsureCapacity(void);
void ExampleStringClear(void);

// --- 4. Encodings & Codepoints ---
void ExampleStringGetEncoding(void);
void ExampleStringSetEncoding(void);
void ExampleStringCodepointCount(void);
void ExampleStringValidateUtf8(void);
void ExampleIterateByCodepoint(void);

// --- 5. Formatting & Macros ---
void ExamplePrintFormatMacro(void);
void ExampleSliceMacro(void);
void ExampleSubsliceMacro(void);
void ExampleSubsliceCodepointMacro(void);
void ExampleUtf8SubsliceMacro(void);

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    printf("====================================================\n");
    printf(" CONVEY STRING LIBRARY - BEGINNER EXAMPLES\n");
    printf("====================================================\n\n");

    //------------------------------------------------------------------------------
    // Choose what example to run by uncommenting it:
    //------------------------------------------------------------------------------
    
    // --- 1. Creation & Destruction ---
    ExampleStringCreate();
    //ExampleStringCreatePro();
    //ExampleStringDuplicate();
    //ExampleStringRepeat();
    //ExampleStringFree();

    // --- 2. Information & Properties ---
    //ExampleStringIsEmpty();
    //ExampleStringGetUsedLength();
    //ExampleStringGetCapacity();
    //ExampleStringGetAvailableCapacity();

    // --- 3. Memory Management ---
    //ExampleStringEnsureCapacity();
    //ExampleStringClear();

    // --- 4. Encodings & Codepoints ---
    //ExampleStringGetEncoding();
    //ExampleStringSetEncoding();
    //ExampleStringCodepointCount();
    //ExampleStringValidateUtf8();
    //ExampleIterateByCodepoint();

    // --- 5. Formatting & Macros ---
    //ExamplePrintFormatMacro();
    //ExampleSliceMacro();
    //ExampleSubsliceMacro();
    //ExampleSubsliceCodepointMacro();
    //ExampleUtf8SubsliceMacro();

    printf("\n====================================================\n");
    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// 1. Creation & Destruction
//----------------------------------------------------------------------------------

void ExampleStringCreate(void)
{
    printf("--- Example: c_string_create ---\n");
    // Creates a dynamic string from a basic C string. Defaults to ASCII encoding.
    c_string_t myStr = c_string_create("Hello convey!");
    printf("Result: "C_STR_FMT" %d \n", C_PRINT_FORMAT(myStr), 10 );
    c_string_free(myStr);
}

void ExampleStringCreatePro(void)
{
    printf("--- Example: c_string_create_pro ---\n");
    // Creates a string from a buffer, specifying exact length and encoding tag.
    const char* rawData = "Testing 123";
    c_string_t myStr = c_string_create_pro(rawData, 11, C_STRING_ENC_ASCII);
    printf("Result: %.*s\n", C_PRINT_FORMAT(myStr));
    c_string_free(myStr);
}

void ExampleStringDuplicate(void)
{
    printf("--- Example: c_string_duplicate ---\n");
    // Creates an exact, independent copy of an existing convey string.
    c_string_t original = c_string_create("Copy me!");
    c_string_t copy = c_string_duplicate(original);
    
    printf("Original: %.*s\n", C_PRINT_FORMAT(original));
    printf("Copy:     %.*s\n", C_PRINT_FORMAT(copy));
    
    c_string_free(original);
    c_string_free(copy);
}

void ExampleStringRepeat(void)
{
    printf("--- Example: c_string_repeat ---\n");
    // Creates a new string by repeating an existing string N times.
    c_string_t base = c_string_create("Ha");
    c_string_t repeated = c_string_repeat(base, 5); // "HaHaHaHaHa"
    
    printf("Repeated 5 times: %.*s\n", C_PRINT_FORMAT(repeated));
    
    c_string_free(base);
    c_string_free(repeated);
}

void ExampleStringFree(void)
{
    printf("--- Example: c_string_free ---\n");
    c_string_t myStr = c_string_create("I will be deleted.");
    // Frees the memory back to the system. ALWAYS do this to prevent memory leaks!
    c_string_free(myStr);
    printf("String was freed successfully.\n");
}

//----------------------------------------------------------------------------------
// 2. Information & Properties
//----------------------------------------------------------------------------------

void ExampleStringIsEmpty(void)
{
    printf("--- Example: c_string_is_empty ---\n");
    c_string_t emptyStr = c_string_create("");
    c_string_t fullStr = c_string_create("Data");

    // Returns true (1) if length is 0, false (0) otherwise.
    printf("Is emptyStr empty? %d\n", c_string_is_empty(emptyStr));
    printf("Is fullStr empty? %d\n", c_string_is_empty(fullStr));

    c_string_free(emptyStr);
    c_string_free(fullStr);
}

void ExampleStringGetUsedLength(void)
{
    printf("--- Example: c_string_get_used_length ---\n");
    c_string_t myStr = c_string_create("Apple");
    
    // Returns the exact number of bytes used (excluding the null terminator).
    size_t len = c_string_get_used_length(myStr);
    printf("Length in bytes: %zu\n", len); // Should be 5
    
    c_string_free(myStr);
}

void ExampleStringGetCapacity(void)
{
    printf("--- Example: c_string_get_capacity ---\n");
    c_string_t myStr = c_string_create("Apple");
    
    // Returns the total allocated byte capacity. 
    // This is often larger than the used length to allow fast appends.
    size_t cap = c_string_get_capacity(myStr);
    printf("Total capacity in bytes: %zu\n", cap);
    
    c_string_free(myStr);
}

void ExampleStringGetAvailableCapacity(void)
{
    printf("--- Example: c_string_get_available_capacity ---\n");
    c_string_t myStr = c_string_create("Apple");
    
    // Returns how many bytes you can add BEFORE the library has to use malloc() again.
    size_t avail = c_string_get_available_capacity(myStr);
    printf("Spare capacity available: %zu bytes\n", avail);
    
    c_string_free(myStr);
}

//----------------------------------------------------------------------------------
// 3. Memory Management
//----------------------------------------------------------------------------------

void ExampleStringEnsureCapacity(void)
{
    printf("--- Example: c_string_ensure_capacity ---\n");
    c_string_t myStr = c_string_create("Tiny");
    
    printf("Available before: %zu\n", c_string_get_available_capacity(myStr));
    
    // Forces the system to allocate enough room for at least 500 more bytes.
    // Notice that you MUST assign the result back to myStr (like realloc)!
    myStr = c_string_ensure_capacity(myStr, 500);
    
    printf("Available after: %zu\n", c_string_get_available_capacity(myStr));
    
    c_string_free(myStr);
}

void ExampleStringClear(void)
{
    printf("--- Example: c_string_clear ---\n");
    c_string_t myStr = c_string_create("Hello World");
    
    // Sets used length to 0 and writes a null terminator. 
    // It DOES NOT free the capacity, making it fast to reuse!
    c_string_clear(myStr);
    
    printf("Length after clear: %zu\n", c_string_get_used_length(myStr));
    printf("Capacity retained: %zu\n", c_string_get_capacity(myStr));
    
    c_string_free(myStr);
}

//----------------------------------------------------------------------------------
// 4. Encodings & Codepoints
//----------------------------------------------------------------------------------

void ExampleStringGetEncoding(void)
{
    printf("--- Example: c_string_get_encoding ---\n");
    // Let's create a UTF-8 tagged string explicitly
    c_string_t myStr = c_string_create_pro("Test", 4, C_STRING_ENC_UTF8);
    
    // Read the encoding tag from the hidden header
    unsigned char enc = c_string_get_encoding(myStr);
    
    if (enc == C_STRING_ENC_UTF8) printf("Encoding is UTF-8!\n");
    if (enc == C_STRING_ENC_ASCII) printf("Encoding is ASCII!\n");
    
    c_string_free(myStr);
}

void ExampleStringSetEncoding(void)
{
    printf("--- Example: c_string_set_encoding ---\n");
    c_string_t myStr = c_string_create("Just some English text");
    
    // Changes the tag WITHOUT converting the data. 
    // Only use if you know the bytes are valid for the new encoding!
    c_string_set_encoding(myStr, C_STRING_ENC_UTF8);
    printf("Encoding tag manually changed to UTF-8.\n");
    
    c_string_free(myStr);
}

void ExampleStringCodepointCount(void)
{
    printf("--- Example: c_string_codepoint_count ---\n");
    // "こんにちは" is 5 Japanese characters, but 15 bytes in UTF-8.
    c_string_t myStr = c_string_create_pro("こんにちは", 15, C_STRING_ENC_UTF8);
    
    size_t byteCount = c_string_get_used_length(myStr);
    size_t charCount = c_string_codepoint_count(myStr);
    
    printf("Bytes used: %zu\n", byteCount);
    printf("Actual characters (codepoints): %zu\n", charCount);
    
    c_string_free(myStr);
}

void ExampleStringValidateUtf8(void)
{
    printf("--- Example: c_string_validate_utf8 ---\n");
    c_string_t goodStr = c_string_create_pro("Valid text", 10, C_STRING_ENC_UTF8);
    
    // Checks if the internal bytes are strictly valid UTF-8 sequences.
    bool isValid = c_string_validate_utf8(goodStr);
    printf("Is valid UTF-8? %s\n", isValid ? "Yes" : "No");
    
    c_string_free(goodStr);
}

void ExampleIterateByCodepoint(void)
{
    printf("--- Example: Iterate UTF-8 by Codepoint ---\n");
    // "ABC" is 3 codepoints
    c_string_t myStr = c_string_create_pro("ABC", 3, C_STRING_ENC_UTF8);
    
    size_t total_chars = c_string_codepoint_count(myStr);
    
    // We can iterate logical characters using the C_SUBSLICE_CP macro.
    // It extracts exactly 1 character starting at index i.
    for (size_t i = 0; i < total_chars; i++)
    {
        c_slice_t singleCharSlice = C_SUBSLICE_CP(myStr, i, 1);
        printf("Character %zu: %.*s\n", i, C_SLICE_FORMAT(singleCharSlice));
    }
    
    c_string_free(myStr);
}

//----------------------------------------------------------------------------------
// 5. Formatting & Macros
//----------------------------------------------------------------------------------

void ExamplePrintFormatMacro(void)
{
    printf("--- Example: C_PRINT_FORMAT ---\n");
    c_string_t myStr = c_string_create("Hello Print Macro");
    
    // C_PRINT_FORMAT safely expands to the length and pointer required by %.*s
    // Never use just %s with convey strings!
    printf("Look here -> %.*s <-\n", C_PRINT_FORMAT(myStr));
    
    c_string_free(myStr);
}

void ExampleSliceMacro(void)
{
    printf("--- Example: C_SLICE_OF and C_SLICE_FORMAT ---\n");
    c_string_t myStr = c_string_create("Full Text");
    
    // C_SLICE_OF creates a non-allocating "view" of the whole string.
    c_slice_t mySlice = C_SLICE_OF(myStr);
    
    // C_SLICE_FORMAT is used to print slices, just like C_PRINT_FORMAT for strings.
    printf("Slice View: %.*s\n", C_SLICE_FORMAT(mySlice));
    
    c_string_free(myStr);
}

void ExampleSubsliceMacro(void)
{
    printf("--- Example: C_SUBSLICE ---\n");
    c_string_t myStr = c_string_create("Programming in C");
    
    // C_SUBSLICE(string, start_byte, length_in_bytes)
    // Let's extract "gram" (starts at byte 3, length 4)
    c_slice_t mySubslice = C_SUBSLICE(myStr, 3, 4);
    
    printf("Extracted Subslice: %.*s\n", C_SLICE_FORMAT(mySubslice));
    
    c_string_free(myStr);
}

void ExampleSubsliceCodepointMacro(void)
{
    printf("--- Example: C_SUBSLICE_CP ---\n");
    // "🚀 Space" -> Rocket is 4 bytes, Space is 5 bytes.
    c_string_t myStr = c_string_create_pro("🚀 Space", 9, C_STRING_ENC_UTF8);
    
    // C_SUBSLICE_CP(string, start_codepoint, length_in_codepoints)
    // We skip 1 character (the rocket) and take 5 characters (" Space")
    c_slice_t myCpSubslice = C_SUBSLICE_CP(myStr, 1, 5);
    
    printf("Extracted by codepoint: %.*s\n", C_SLICE_FORMAT(myCpSubslice));
    
    c_string_free(myStr);
}

void ExampleUtf8SubsliceMacro(void)
{
    printf("--- Example: C_UTF8_SUBSLICE ---\n");
    c_string_t myStr = c_string_create_pro("Data Stream", 11, C_STRING_ENC_UTF8);
    
    // Extracts a subslice and specifically tags the resulting struct as UTF-8.
    c_utf8_slice_t utf8Slice = C_UTF8_SUBSLICE(myStr, 0, 4); // "Data"
    
    // Needs C_UTF8_SLICE_FORMAT to print
    printf("UTF8 Subslice: %.*s\n", C_UTF8_SLICE_FORMAT(utf8Slice));
    
    c_string_free(myStr);
}
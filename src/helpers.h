
// Convert a defined token to a string
#define _STR(x) #x

// Usable wrapper for above
// Note: #x does not work in context, 2nd layer is required ??
// http://www.decompile.com/cpp/faq/file_and_line_error_string.htm
#define STR(x) _STR(x)

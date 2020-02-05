#ifndef XXHSUM_WIN32_UTF8_H
#define XXHSUM_WIN32_UTF8_H

#if defined(WIN32) || defined(_WIN32)

#include <windows.h>    /* MultiByteToWideChar(), WideCharToMultiByte(), CP_UTF8 */
#include <stdio.h>      /* FILE, _wfopen_s() */
#include <stdlib.h>     /* malloc(), free() */
#include <io.h>         /* _setmode() */
#include <fcntl.h>      /* _O_U8TEXT */

#if !defined(restrict)
#  define restrict  __restrict
#endif


/* Convert UTF-8 string (utf8Str) to UTF-16 string.  Writes UTF-16 string to dst. */
static int utf8ToUtf16(wchar_t* dst, size_t dstSize, const char* utf8Str)
{
    return MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, dst, dstSize);
}


/* Returns wcslen(utf8ToUtf16(utf8str))+1 */
static size_t utf16Buflen(const char* utf8Str)
{
    return (size_t) utf8ToUtf16(NULL, 0, utf8Str);
}


/* Convert UTF-16 string (utf16Str) to UTF-8 string.  Writes UTF-8 string to dst */
static int utf16ToUtf8(char* dst, size_t dstSize, const wchar_t* utf16Str)
{
    return WideCharToMultiByte(CP_UTF8, 0, utf16Str, -1, dst, dstSize, NULL, NULL);
}


/* Returns strlen(utf8ToUtf16(utf8str))+1 */
static size_t utf8Buflen(const wchar_t* utf16Str)
{
    return (size_t) utf16ToUtf8(NULL, 0, utf16Str);
}


/* Output UTF-8 format string to stream by fwprintf() */
static int fprintf_utf8(FILE *stream, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int result = _vscprintf(format, args);
    if (result > 0) {
        const int nchar = result + 1;
        char* u8str = (char*) malloc(nchar * sizeof(u8str[0]));
        result = vsprintf_s(u8str, nchar, format, args);
        if (result > 0) {
            const size_t u16BufSize = utf16Buflen(u8str);
            wchar_t* const u16Buf = (wchar_t*) malloc(u16BufSize * sizeof(u16Buf[0]));
            utf8ToUtf16(u16Buf, u16BufSize, u8str);
            result = fwprintf(stream, L"%s", u16Buf);
            free(u16Buf);
        }
        free(u8str);
    }
    va_end(args);
    return result;
}


/* fopen() by UTF-8 filename and mode string */
static FILE* fopen_utf8(const char *restrict filename, const char *restrict mode)
{
    FILE* f = NULL;

    const size_t filenameBufSize = utf16Buflen(filename);
    const size_t modeBufSize     = utf16Buflen(mode);

    wchar_t* const filenameBuf = (wchar_t*) malloc(sizeof(wchar_t) * filenameBufSize);
    wchar_t* const modeBuf     = (wchar_t*) malloc(sizeof(wchar_t) * modeBufSize);

    utf8ToUtf16(filenameBuf, filenameBufSize, filename);
    utf8ToUtf16(modeBuf    , modeBufSize    , mode);

    _wfopen_s(&f, filenameBuf, modeBuf);

    free(modeBuf);
    free(filenameBuf);

    return f;
}


/* Create UTF-8 argv[] from UTF-16 argv[] */
static char** createUtf8Argv(int argc, const wchar_t** argv)
{
    char** utf8_argv = (char**) malloc(sizeof(char*) * (argc+1));
    int i;
    for (i = 0; i < argc; ++i) {
        const size_t u8Len = utf8Buflen(argv[i]);
        char* u8Buf = (char*) malloc(u8Len);
        utf16ToUtf8(u8Buf, u8Len, argv[i]);
        utf8_argv[i] = u8Buf;
    }
    utf8_argv[i] = NULL;
    return utf8_argv;
}


/* Free UTF-8 argv[] */
static void freeUtf8Argv(int argc, char** utf8_argv)
{
    int i;
    for (i = 0; i < argc; ++i)
        free(utf8_argv[i]);
    free(utf8_argv);
}


#endif/*defined(WIN32) || defined(_WIN32)*/

#endif/*XXHSUM_WIN32_UTF8_H*/

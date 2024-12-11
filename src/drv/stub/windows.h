/* Stub file defined for Linux build.
 * Defines Windows APIs that are used in parts of our codes.
 * These will either be stubbed or wrap Linux equivalent API.
 * Very ver cursed... but it compiles (hopefully).
 */

#ifndef STUB_WINDOWS_H
#define STUB_WINDOWS_H

#include <stdint.h>
#include <byteswap.h>

#define SETBREAK 0
#define CLRBREAK 1

#define _byteswap_ushort bswap_16
#define _byteswap_ulong bswap_32

typedef int HANDLE;
typedef int DWORD;
typedef int BOOL;

BOOL EscapeCommFunction(HANDLE hFile, DWORD dwFunc);
void Sleep(DWORD);

#endif


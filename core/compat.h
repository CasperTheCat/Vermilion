#pragma once

#define CXX17_FALLTHROUGH [[fallthrough]]
#define CXX17_NODISCARD [[nodiscard]]

#ifdef _MSC_VER
    #define VmCopyString(dest, src, count) strncpy_s(dest, count, src, count);
    #define VmZeroMemory(dst,ch,destsz) memset_s(dst,ch,destsz,destsz)
#else
    #define VmCopyString(dest, src, count) strncpy(dest,src,count);
    #define VmZeroMemory(dst,ch,destsz) memset(dst,ch,destsz)
#endif

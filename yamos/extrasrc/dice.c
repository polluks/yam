#if defined(_DCC) || defined(__VBCC__) || defined(NO_INLINE_STDARG)

#include <exec/types.h>

/* FIX V45 breakage... */
#if INCLUDE_VERSION < 45
#define MY_CONST_STRPTR CONST_STRPTR
#else
#define MY_CONST_STRPTR CONST STRPTR
#endif

#include <proto/wb.h>
BOOL WorkbenchControl(STRPTR name, ...)
{ return WorkbenchControlA(name,(struct TagItem *)(&name+1)); }

#include <proto/icon.h>
struct DiskObject *GetIconTags(MY_CONST_STRPTR name, ... )
{ return GetIconTagList(name, (struct TagItem *)(&name+1)); }

#include <proto/xpkmaster.h>
LONG XpkQueryTags(ULONG tag, ...)
{ return XpkQuery((struct TagItem *)&tag); }
LONG XpkPackTags(ULONG tag, ...)
{ return XpkPack((struct TagItem *)&tag); }
LONG XpkUnpackTags(ULONG tag, ...)
{ return XpkUnpack((struct TagItem *)&tag); }

#include <proto/openurl.h>
BOOL URL_Open(STRPTR str, ULONG tag, ...)
{ return URL_OpenA(str,(struct TagItem *)&tag); }

#include <proto/amissl.h>
long InitAmiSSL(ULONG tag, ...)
{ return InitAmiSSLA((struct TagItem *)&tag); }
long CleanupAmiSSL(ULONG tag, ...)
{ return CleanupAmiSSLA((struct TagItem *)&tag); }

#endif

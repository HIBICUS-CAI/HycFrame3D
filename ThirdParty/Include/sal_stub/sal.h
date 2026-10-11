//---------------------------------------------------------------
// File: sal.h
// Info: minimal SAL annotation stub for building DirectXMath on
//       non-Windows platforms
// Comt: only the annotations used by DirectXMath are defined,
//       all of them expand to nothing. never add this folder to
//       include path on Windows, use the one from Windows SDK.
//---------------------------------------------------------------

#pragma once

#ifdef _WIN32
#error "use sal.h from Windows SDK instead of this stub"
#endif // _WIN32

#ifndef _In_
#define _In_
#endif
#ifndef _In_opt_
#define _In_opt_
#endif
#ifndef _In_reads_
#define _In_reads_(Size)
#endif
#ifndef _In_reads_bytes_
#define _In_reads_bytes_(Size)
#endif
#ifndef _Out_
#define _Out_
#endif
#ifndef _Out_opt_
#define _Out_opt_
#endif
#ifndef _Out_writes_
#define _Out_writes_(Size)
#endif
#ifndef _Out_writes_bytes_
#define _Out_writes_bytes_(Size)
#endif
#ifndef _Inout_
#define _Inout_
#endif
#ifndef _Success_
#define _Success_(Expr)
#endif
#ifndef _Use_decl_annotations_
#define _Use_decl_annotations_
#endif
#ifndef _Analysis_assume_
#define _Analysis_assume_(Expr)
#endif

//==========================================================================
//
//      simple.cxx
//
//      Simple, non formatting trace and assert functions
//
//==========================================================================
//####COPYRIGHTBEGIN####
//
// -------------------------------------------
// The contents of this file are subject to the Cygnus eCos Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://sourceware.cygnus.com/ecos
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the
// License for the specific language governing rights and limitations under
// the License.
// 
// The Original Code is eCos - Embedded Cygnus Operating System, released
// September 30, 1998.
// 
// The Initial Developer of the Original Code is Cygnus.  Portions created
// by Cygnus are Copyright (C) 1998,1999 Cygnus Solutions.  All Rights Reserved.
// -------------------------------------------
//
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-12-04
// Purpose:     Simple Trace and assert functions
// Description: The functions in this file are simple implementations
//              of the standard trace and assert functions. These do not
//              do any printf style formatting, simply printing the string
//              and arguments directly.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/infra.h>

#ifdef CYGDBG_INFRA_DEBUG_TRACE_ASSERT_SIMPLE

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <pkgconf/hal.h>                // HAL configury
#include <cyg/infra/diag.h>             // HAL polled output
#include <cyg/hal/hal_arch.h>           // architectural stuff for...
#include <cyg/hal/hal_intr.h>           // interrupt control

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>             // kernel configury
#include <cyg/kernel/thread.hxx>        // thread id to print
#include <cyg/kernel/sched.hxx>         // ancillaries for above
#include <cyg/kernel/thread.inl>        // ancillaries for above
#endif

// -------------------------------------------------------------------------
// Local Configuration: hack me!

#define CYG_NO_FILENAME 1
#define CYG_NO_THREADID 0
#define CYG_NO_LINENUM  0
#define CYG_NO_FUNCNAME 0
#define CYG_DIAG_PRINTF 1

#ifndef CYGPKG_KERNEL
# undef  CYG_NO_THREADID
# define CYG_NO_THREADID 1
#endif
// -------------------------------------------------------------------------
// Functions to trim file names and function names down to printable lengths
// (these are shared between trace and assert functions)

static const char *trim_file(const char *file)
{
#if !CYG_NO_FILENAME
    if ( NULL == file )
        file = "<nofile>";
    
    const char *f = file;
    
    while( *f ) f++;

    while( *f != '/' && f != file ) f--;

    return f==file?f:(f+1);
#else
    return "";
#endif
}

static const char *trim_func(const char *func)
{
#if !CYG_NO_FUNCNAME
    
    static char fbuf[100];
    int i;
    
    if ( NULL == func )
        func = "<nofunc>";

    for( i = 0; func[i] && func[i] != '(' ; i++ )
        fbuf[i] = func[i];

    fbuf[i++] = '(';
    fbuf[i++] = ')';
    fbuf[i  ] = 0;

    return &fbuf[0];
#else
    return "";
#endif
}

void write_lnum( cyg_uint32 lnum)
{
#if !CYG_NO_LINENUM
    diag_write_char('[');
    diag_write_dec(lnum);
    diag_write_char(']');
#endif
}

void write_thread_id()
{
#if !CYG_NO_THREADID    
    Cyg_Thread *t = Cyg_Thread::self();
    cyg_uint16 tid = 0xFFFF;

    if( t != NULL ) tid = t->get_unique_id();

    diag_write_char('<');
    diag_write_hex(tid);
    diag_write_char('>');
#endif
}


// -------------------------------------------------------------------------
// Trace functions:

#ifdef CYGDBG_USE_TRACING

externC void
cyg_tracenomsg( char *psz_func, char *psz_file, cyg_uint32 linenum )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char('\n');

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
    
};

// provide every other one of these as a space/caller bloat compromise.

externC void
cyg_tracemsg( cyg_uint32 what, 
              char *psz_func, char *psz_file, cyg_uint32 linenum,
              char *psz_msg )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
    diag_write_string(psz_msg);
    diag_write_char('\n');

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);

};

externC void
cyg_tracemsg2( cyg_uint32 what, 
               char *psz_func, char *psz_file, cyg_uint32 linenum,
               char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1 )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
#if CYG_DIAG_PRINTF
    diag_printf( psz_msg, arg0, arg1 );
#else
    diag_write_string(psz_msg);
    diag_write_char(' ');
    diag_write_hex(arg0);
    diag_write_char(' ');
    diag_write_hex(arg1);
#endif    
    diag_write_char('\n');    
    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
};

externC void
cyg_tracemsg4( cyg_uint32 what, 
               char *psz_func, char *psz_file, cyg_uint32 linenum, 
               char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3 )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
#if CYG_DIAG_PRINTF
    diag_printf( psz_msg, arg0, arg1, arg2, arg3 );
#else
    diag_write_string(psz_msg);
    diag_write_char(' ');
    diag_write_hex(arg0);
    diag_write_char(' ');
    diag_write_hex(arg1);
    diag_write_char(' ');
    diag_write_hex(arg2);
    diag_write_char(' ');
    diag_write_hex(arg3);
#endif    
    diag_write_char('\n');    

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
};

externC void
cyg_tracemsg6( cyg_uint32 what, 
               char *psz_func, char *psz_file, cyg_uint32 linenum,
               char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3,
               CYG_ADDRWORD arg4,  CYG_ADDRWORD arg5 )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
#if CYG_DIAG_PRINTF
    diag_printf( psz_msg, arg0, arg1, arg2, arg3, arg4, arg5 );
#else
    diag_write_string(psz_msg);
    diag_write_char(' ');
    diag_write_hex(arg0);
    diag_write_char(' ');
    diag_write_hex(arg1);
    diag_write_char(' ');
    diag_write_hex(arg2);
    diag_write_char(' ');
    diag_write_hex(arg3);
    diag_write_char(' ');
    diag_write_hex(arg4);
    diag_write_char(' ');
    diag_write_hex(arg5);
#endif    
    diag_write_char('\n');    

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
};

externC void
cyg_tracemsg8( cyg_uint32 what, 
               char *psz_func, char *psz_file, cyg_uint32 linenum,
               char *psz_msg,
               CYG_ADDRWORD arg0,  CYG_ADDRWORD arg1,
               CYG_ADDRWORD arg2,  CYG_ADDRWORD arg3,
               CYG_ADDRWORD arg4,  CYG_ADDRWORD arg5,
               CYG_ADDRWORD arg6,  CYG_ADDRWORD arg7 )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    if ( NULL == psz_msg )
        psz_msg = "<nomsg>";

    diag_write_string("TRACE: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
#if CYG_DIAG_PRINTF
    diag_printf( psz_msg, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7 );
#else
    diag_write_string(psz_msg);
    diag_write_char(' ');
    diag_write_hex(arg0);
    diag_write_char(' ');
    diag_write_hex(arg1);
    diag_write_char(' ');
    diag_write_hex(arg2);
    diag_write_char(' ');
    diag_write_hex(arg3);
    diag_write_char(' ');
    diag_write_hex(arg4);
    diag_write_char(' ');
    diag_write_hex(arg5);
    diag_write_char(' ');
    diag_write_hex(arg6);
    diag_write_char(' ');
    diag_write_hex(arg7);
#endif    
    diag_write_char('\n');    

    DIAG_DEVICE_END_SYNC();
    HAL_RESTORE_INTERRUPTS(old_ints);
};

#endif // CYGDBG_USE_TRACING

// -------------------------------------------------------------------------
// Assert functions:

#ifdef CYGDBG_USE_ASSERTS

externC void
cyg_assert_fail( const char *psz_func, const char *psz_file,
                 cyg_uint32 linenum, const char *psz_msg )
{
    cyg_uint32 old_ints;

    HAL_DISABLE_INTERRUPTS(old_ints);
    DIAG_DEVICE_START_SYNC();

    diag_write_string("ASSERT FAIL: ");
    write_thread_id();
    diag_write_string(trim_file(psz_file));
    write_lnum(linenum);
    diag_write_string(trim_func(psz_func));
    diag_write_char(' ');
    diag_write_string(psz_msg);
    diag_write_char('\n');

    for(;;);

//    DIAG_DEVICE_END_SYNC();
//    HAL_RESTORE_INTERRUPTS(old_ints);
};

extern "C"
{
extern unsigned long CYG_LABEL_NAME(stext);
extern unsigned long CYG_LABEL_NAME(etext);

unsigned long stext_addr = (unsigned long)&CYG_LABEL_NAME(stext);
unsigned long etext_addr = (unsigned long)&CYG_LABEL_NAME(etext);
};

externC cyg_bool cyg_check_data_ptr(void *ptr)
{
    unsigned long p = (unsigned long)ptr;
    
    if( p == 0 ) return false;

    if( p > stext_addr && p < etext_addr ) return false;

    return true;
}

externC cyg_bool cyg_check_func_ptr(void (*ptr)(void))
{
    unsigned long p = (unsigned long)ptr;
    
    if( p == 0 ) return false;

    if( p < stext_addr && p > etext_addr ) return false;

    return true;
}

#endif // CYGDBG_USE_ASSERTS

#endif // CYGDBG_INFRA_DEBUG_TRACE_ASSERT_SIMPLE

// -------------------------------------------------------------------------
// EOF simple.cxx

#ifndef CYGONCE_LIBC_CLIBINCL_STREAM_INL
#define CYGONCE_LIBC_CLIBINCL_STREAM_INL
//========================================================================
//
//      stream.inl
//
//      Inline functions for internal C library stdio stream interface
//
//========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          1998-02-13
// Purpose:     
// Description: 
// Usage:         Do not include this file -
//                #include "clibincl/stream.hxx" instead.
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc.h>   // Configuration header

// Include the C library? And do we want the stdio stuff?
#if defined(CYGPKG_LIBC) && defined(CYGPKG_LIBC_STDIO)

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common project-wide type definitions
#include <stddef.h>                // NULL and size_t from compiler
#include <errno.h>                 // Error codes
#include "clibincl/stream.hxx"     // Just be sure that this really is
                                   // included

// FUNCTIONS

#ifdef CYGDBG_USE_ASSERTS
inline bool
Cyg_StdioStream::check_this( cyg_assert_class_zeal zeal ) const
{
    // check that it has the magic word set meaning it is valid.
    if ( magic_validity_word != 0x7b4321ce )
        return false;
    return true;
} // check_this()

#endif // ifdef CYGDBG_USE_ASSERTS



// LOCKING FUNCTIONS

// returns true on success
inline cyg_bool
Cyg_StdioStream::lock_me( void )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    return stream_lock.lock();
#else
    // otherwise it "worked"
    return true;
#endif
    
} // lock_me()


// returns true on success
inline cyg_bool
Cyg_StdioStream::trylock_me( void )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    return stream_lock.trylock();
#else
    // otherwise it "worked"
    return true;
#endif
    
} // lock_me()


inline void
Cyg_StdioStream::unlock_me( void )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGSEM_LIBC_STDIO_THREAD_SAFE_STREAMS
    stream_lock.unlock();
#endif
} // unlock_me()


// DESTRUCTOR

inline
Cyg_StdioStream::~Cyg_StdioStream()
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return;

    flush_output_unlocked();

    // Currently there is no I/O function defined for "close"
    // Add such a call if this is ever defined.

    unlock_me();

#ifdef CYGDBG_USE_ASSERTS
    magic_validity_word = 0xbadbad;
#endif
} // Cyg_StdioStream destructor


// MEMBER FUNCTIONS


// this is currently just a wrapper around write, but having this interface
// leaves scope for optimisations in future
inline Cyg_ErrNo
Cyg_StdioStream::write_byte( cyg_uint8 c )
{
    cyg_ucount32 dummy_bytes_written;
    Cyg_ErrNo err;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    err = write( &c, 1, &dummy_bytes_written );

    CYG_ASSERT( (err!=ENOERR) || (dummy_bytes_written==1),
                "Single byte not written, but no error returned!" );

    return err;
} // write_byte()


inline Cyg_ErrNo
Cyg_StdioStream::unread_byte( cyg_uint8 c )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGFUN_LIBC_STDIO_ungetc
    if (!lock_me())
        return EBADF;  // assume file is now invalid

    if (flags.unread_char_buf_in_use) {
        unlock_me();
        return ENOMEM;
    } // if

    flags.unread_char_buf_in_use = true;
    unread_char_buf = c;

    // can't be at EOF any more
    flags.at_eof = false;

    if (position)    // position is always 0 for certain devices
        --position;
    
    unlock_me();

    return ENOERR;

#else // ifdef CYGFUN_LIBC_STDIO_ungetc

    return ENOSYS;
#endif // ifdef CYGFUN_LIBC_STDIO_ungetc
} // unread_byte()


inline cyg_ucount32
Cyg_StdioStream::bytes_available_to_read( void )
{
    cyg_ucount32 bytes=0;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
#ifdef CYGFUN_LIBC_STDIO_ungetc
    if (flags.unread_char_buf_in_use)
        ++bytes;
#endif 

#ifdef CYGSEM_LIBC_STDIO_WANT_BUFFERED_IO

    // either the last operation was a read, which attempted to read bytes
    // into the buffer, or there are no bytes in the buffer

    if (flags.buffering) {
        if (flags.last_buffer_op_was_read == true)
            bytes += io_buf.get_buffer_space_used();
    }
    else

#endif

    if (flags.readbuf_char_in_use)
        ++bytes;

    return bytes;
} // bytes_available_to_read()



inline Cyg_ErrNo
Cyg_StdioStream::flush_output( void )
{
    Cyg_ErrNo err;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return EBADF;  // assume file is now invalid
    
    err = flush_output_unlocked();

    unlock_me();
  
    return err;
} // flush_output()


// get error status for this file
inline Cyg_ErrNo
Cyg_StdioStream::get_error( void )
{
    Cyg_ErrNo err_temp;
    
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return EBADF;     // well, we've certainly got an error now!
    
    err_temp = error;

    unlock_me();

    return err_temp;
} // get_error()


// set error status for this file
inline void
Cyg_StdioStream::set_error( Cyg_ErrNo errno_to_set )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
    {
        errno = EBADF; // best we can do - we can't trust error to be there
        return;
    } // if
    
    errno = error = errno_to_set;

    unlock_me();
} // set_error()


// are we at EOF? true means we are, false means no
inline cyg_bool
Cyg_StdioStream::get_eof_state( void )
{
    cyg_bool eof_temp;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return false;     // not much we can do here
    
    eof_temp = flags.at_eof;

    unlock_me();
    
    return eof_temp;
} // get_eof_state()


// Set whether we are at EOF.
inline void
Cyg_StdioStream::set_eof_state( cyg_bool eof_to_set )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return;     // not much we can do here
    
    flags.at_eof = eof_to_set;

    unlock_me();
} // set_eof_state()


// retrieve position
inline Cyg_ErrNo
Cyg_StdioStream::get_position( fpos_t *pos )
{
    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if (!lock_me())
        return EBADF; // assume file is now invalid

    *pos = position;

    unlock_me();

    return ENOERR;
} // get_position()


// set absolute position
inline Cyg_ErrNo
Cyg_StdioStream::set_position( fpos_t pos, int whence )
{
    // this is currently a workaround until we have real files
    // this will be corrected when we decide the true filesystem interface

    Cyg_ErrNo err;
    cyg_uint8 c;

    CYG_ASSERTCLASS( this, "Stream object is not a valid stream!" );
    
    if ((whence != SEEK_CUR) || pos < 0)
        return ENOSYS;

    if (!lock_me())
        return EBADF; // assume file is now invalid

    // Drain read buffer
    
    for ( ; whence > 0 ; whence-- ) {
        err = read_byte( &c );
        if (err == EAGAIN)
            err=refill_read_buffer();

        // if read_byte retured error, or refill_read_buffer returned error
        if (err) {
            unlock_me();
            return err;
        } // if
    } // for

    unlock_me();

    return ENOERR;
} // set_position()


#endif // if defined(CYGPKG_LIBC) && defined(CYGPKG_LIBC_STDIO)

#endif CYGONCE_LIBC_CLIBINCL_STREAM_INL multiple inclusion protection

// EOF stream.inl

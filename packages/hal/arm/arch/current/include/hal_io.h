#ifndef CYGONCE_HAL_IO_H
#define CYGONCE_HAL_IO_H

//=============================================================================
//
//      hal_io.h
//
//      HAL device IO register support.
//
//=============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.0 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://sourceware.cygnus.com/ecos                                        
//                                                                          
// Software distributed under the License is distributed on an       
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg, gthomas
// Contributors:        nickg, gthomas
// Date:        1998-09-11
// Purpose:     Define IO register support
// Description: The macros defined here provide the HAL APIs for handling
//              device IO control registers.
//              
// Usage:
//              #include <cyg/hal/hal_io.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/basetype.h>

//-----------------------------------------------------------------------------
// IO Register address.
// This type is for recording the address of an IO register.

typedef volatile CYG_ADDRWORD HAL_IO_REGISTER;


//-----------------------------------------------------------------------------
// BYTE Register access.
// Individual and vectorized access to 8 bit registers.

// Little-endian version
#if (CYG_BYTEORDER == CYG_LSBFIRST)

#define HAL_READ_UINT8( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_BYTE *)(_register_)))

#define HAL_WRITE_UINT8( _register_, _value_ ) \
        (*((volatile CYG_BYTE *)(_register_)) = (_value_))

#define HAL_READ_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )     \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = ((volatile CYG_BYTE *)(_register_))[_j_];        \
    CYG_MACRO_END

#define HAL_WRITE_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        ((volatile CYG_BYTE *)(_register_))[_j_] = (_buf_)[_i_];        \
    CYG_MACRO_END

#else // Big-endian version

#define HAL_READ_UINT8( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_BYTE *)((CYG_ADDRWORD)(_register_)^3)))

#define HAL_WRITE_UINT8( _register_, _value_ ) \
        (*((volatile CYG_BYTE *)((CYG_ADDRWORD)(_register_)^3)) = (_value_))

#define HAL_READ_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )     \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    volatile CYG_BYTE* _r_ = ((CYG_ADDRWORD)(_register_)^3);            \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = _r_[_j_];                                        \
    CYG_MACRO_END

#define HAL_WRITE_UINT8_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    volatile CYG_BYTE* _r_ = ((CYG_ADDRWORD)(_register_)^3);            \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        _r_[_j_] = (_buf_)[_i_];                                        \
    CYG_MACRO_END

#endif // Big-endian

//-----------------------------------------------------------------------------
// 16 bit access.
// Individual and vectorized access to 16 bit registers.
    
// Little-endian version
#if (CYG_BYTEORDER == CYG_LSBFIRST)

#define HAL_READ_UINT16( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_WORD16 *)(_register_)))

#define HAL_WRITE_UINT16( _register_, _value_ ) \
        (*((volatile CYG_WORD16 *)(_register_)) = (_value_))

#define HAL_READ_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = ((volatile CYG_WORD16 *)(_register_))[_j_];      \
    CYG_MACRO_END

#define HAL_WRITE_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        ((volatile CYG_WORD16 *)(_register_))[_j_] = (_buf_)[_i_];      \
    CYG_MACRO_END

#else // Big-endian version

#define HAL_READ_UINT16( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_WORD16 *)((CYG_ADDRWORD)(_register_)^3)))

#define HAL_WRITE_UINT16( _register_, _value_ ) \
        (*((volatile CYG_WORD16 *)((CYG_ADDRWORD)(_register_)^3)) = (_value_))

#define HAL_READ_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    volatile CYG_WORD16* _r_ = ((CYG_ADDRWORD)(_register_)^3);          \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = _r_[_j_];                                        \
    CYG_MACRO_END

#define HAL_WRITE_UINT16_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    volatile CYG_WORD16* _r_ = ((CYG_ADDRWORD)(_register_)^3);          \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        _r_[_j_] = (_buf_)[_i_];                                        \
    CYG_MACRO_END

#endif // Big-endian

//-----------------------------------------------------------------------------
// 32 bit access.
// Individual and vectorized access to 32 bit registers.

// Note: same macros for little- and big-endian systems.
    
#define HAL_READ_UINT32( _register_, _value_ ) \
        ((_value_) = *((volatile CYG_WORD32 *)(_register_)))

#define HAL_WRITE_UINT32( _register_, _value_ ) \
        (*((volatile CYG_WORD32 *)(_register_)) = (_value_))

#define HAL_READ_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )    \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        (_buf_)[_i_] = ((volatile CYG_WORD32 *)(_register_))[_j_];      \
    CYG_MACRO_END

#define HAL_WRITE_UINT32_VECTOR( _register_, _buf_, _count_, _step_ )   \
    CYG_MACRO_START                                                     \
    cyg_count32 _i_,_j_;                                                \
    for( _i_ = 0, _j_ = 0; _i_ < (_count_); _i_++, _j_ += (_step_))     \
        ((volatile CYG_WORD32 *)(_register_))[_j_] = (_buf_)[_i_];      \
    CYG_MACRO_END

// Enforce a flow "barrier" to prevent optimizing compiler from reordering 
// operations.
#define HAL_IO_BARRIER()

//-----------------------------------------------------------------------------
// Include plf_io.h for platforms that define it.
// FIXME: It should be possible to handle this better with CDL; define
//        CYGBLD_HAL_PLATFORM_IO or similar where appropriate.
#ifdef CYGPKG_HAL_ARM_EBSA285
#include <cyg/hal/plf_io.h>
#endif

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_IO_H
// End of hal_io.h

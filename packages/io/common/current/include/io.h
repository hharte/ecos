#ifndef CYGONCE_IO_H
#define CYGONCE_IO_H
// ====================================================================
//
//      io.h
//
//      Device I/O 
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   gthomas
// Contributors:        gthomas
// Date:        1999-02-04
// Purpose:     Describe low level I/O interfaces.
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================

// This file contains the user-level visible I/O interfaces

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#ifdef CYGPKG_ERROR
#include <cyg/error/codes.h>
#else
#error I/O subsystem requires 'error' package
#endif

// typedef int Cyg_ErrNo;

#ifdef __cplusplus
extern "C" {
#endif

typedef void *cyg_io_handle_t;

// Lookup a device and return it's handle
Cyg_ErrNo cyg_io_lookup(const char *name, 
                        cyg_io_handle_t *handle);
// Write data to a device
Cyg_ErrNo cyg_io_write(cyg_io_handle_t handle, 
                       const void *buf, 
                       cyg_uint32 *len);
// Read data from a device
Cyg_ErrNo cyg_io_read(cyg_io_handle_t handle, 
                      void *buf, 
                      cyg_uint32 *len);
// Write data to a block device
Cyg_ErrNo cyg_io_bwrite(cyg_io_handle_t handle, 
                       const void *buf, 
                       cyg_uint32 *len,
                       cyg_uint32 pos);
// Read data from a block device
Cyg_ErrNo cyg_io_bread(cyg_io_handle_t handle, 
                      void *buf, 
                      cyg_uint32 *len,
                      cyg_uint32 pos);
// Get the configuration of a device
Cyg_ErrNo cyg_io_get_config(cyg_io_handle_t handle, 
                            cyg_uint32 key,
                            void *buf, 
                            cyg_uint32 *len);
// Change the configuration of a device
Cyg_ErrNo cyg_io_set_config(cyg_io_handle_t handle, 
                            cyg_uint32 key,
                            const void *buf, 
                            cyg_uint32 *len);
// Test a device for readiness    
cyg_bool cyg_io_select(cyg_io_handle_t handle,
                       cyg_uint32 which,
                       CYG_ADDRWORD info);

#ifdef __cplusplus
}
#endif

#endif  /* CYGONCE_IO_H */
/* EOF io.h */

//==========================================================================
//
//      kinetis_gpio.c
//
//      Cortex-M Kinetis HAL functions
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2014 Free Software Foundation, Inc.                        
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    ilijak
// Date:         2014-02-05
// Description:
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <cyg/hal/hal_io.h>
#include <cyg/hal/var_io_gpio.h>

static cyghwr_hal_kinetis_gpio_t * const PortsGPIO[] = {
    CYGHWR_HAL_KINETIS_GPIO_PORTA_P, CYGHWR_HAL_KINETIS_GPIO_PORTB_P,
    CYGHWR_HAL_KINETIS_GPIO_PORTC_P, CYGHWR_HAL_KINETIS_GPIO_PORTD_P,
    CYGHWR_HAL_KINETIS_GPIO_PORTE_P, CYGHWR_HAL_KINETIS_GPIO_PORTF_P
};

void
hal_gpio_pin_ddr_out(cyg_uint32 pin){
    cyghwr_hal_kinetis_gpio_t *port_p;

    if(pin != CYGHWR_HAL_KINETIS_PIN_NONE) {
        port_p = PortsGPIO[CYGHWR_HAL_KINETIS_PIN_PORT(pin)];
        port_p->pddr |= BIT_(CYGHWR_HAL_KINETIS_PIN_BIT(pin));
    }    
}

void
hal_gpio_pin_ddr_in(cyg_uint32 pin){
    cyghwr_hal_kinetis_gpio_t *port_p;

    if(pin != CYGHWR_HAL_KINETIS_PIN_NONE) {
        port_p = PortsGPIO[CYGHWR_HAL_KINETIS_PIN_PORT(pin)];
        port_p->pddr &= ~BIT_(CYGHWR_HAL_KINETIS_PIN_BIT(pin));
    }    
}

cyg_uint32
hal_gpio_get_pin(cyg_uint32 pin){
    cyghwr_hal_kinetis_gpio_t *port_p;
    cyg_uint32 retval = 0xffffffff;
    
    if(pin != CYGHWR_HAL_KINETIS_PIN_NONE) {
        port_p = PortsGPIO[CYGHWR_HAL_KINETIS_PIN_PORT(pin)];
        retval = port_p->pdir & BIT_(CYGHWR_HAL_KINETIS_PIN_BIT(pin));
    }
    return retval;
}


// GPIO register on a given port (register name is lower case)
#define CYGHWR_HAL_KINETIS_GPIO_PORT_OPER(__port, __mask, __opreg) \
CYG_MACRO_START                                                    \
    cyghwr_hal_kinetis_gpio_t *port_p;                             \
    port_p = PortsGPIO[__port];                                    \
    port_p->__opreg = __mask;                                      \
CYG_MACRO_END

#define CYGHWR_HAL_KINETIS_GPIO_PIN_OPER(__pin, __opreg)                                 \
CYG_MACRO_START                                                                          \
if(__pin != CYGHWR_HAL_KINETIS_PIN_NONE)                                                 \
    CYGHWR_HAL_KINETIS_GPIO_PORT_OPER(CYGHWR_HAL_KINETIS_PIN_PORT(__pin),                \
                                      BIT_(CYGHWR_HAL_KINETIS_PIN_BIT(__pin)), __opreg); \
CYG_MACRO_END

// Port operations
// Bit operations (clear, set or toggle) are performed on port's bits
// that correspond to set bits in the mask parameter.

void
hal_gpio_port_clear(cyg_uint32 port_i, cyg_uint32 mask){
    CYGHWR_HAL_KINETIS_GPIO_PORT_OPER(port_i, mask, pcor);
}

void
hal_gpio_port_set(cyg_uint32 port_i, cyg_uint32 mask){
    CYGHWR_HAL_KINETIS_GPIO_PORT_OPER(port_i, mask, psor);
}

void
hal_gpio_port_toggle(cyg_uint32 port_i, cyg_uint32 mask){
    CYGHWR_HAL_KINETIS_GPIO_PORT_OPER(port_i, mask, ptor);
}

// Pin operations.
// Operation is performed to a given pin. Thin is addresses through
// a pin descriptor. Pin descriptor is typically created with CYGHWR_HAL_KINETIS_PIN_CFG()
// or CYGHWR_HAL_KINETIS_PIN() [ defined in var_io.h ]

void
hal_gpio_pin_clear(cyg_uint32 pin){
    CYGHWR_HAL_KINETIS_GPIO_PIN_OPER(pin, pcor);
}

void
hal_gpio_pin_set(cyg_uint32 pin){
    CYGHWR_HAL_KINETIS_GPIO_PIN_OPER(pin, psor);
}

void
hal_gpio_pin_toggle(cyg_uint32 pin){
    CYGHWR_HAL_KINETIS_GPIO_PIN_OPER(pin, ptor);
}

// EOF kinetis_gpio.c

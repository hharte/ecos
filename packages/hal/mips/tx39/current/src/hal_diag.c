/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-03-02
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_diag.h>

#include <cyg/hal/hal_intr.h>

#include <cyg/hal/hal_io.h>

/*---------------------------------------------------------------------------*/

//#define CYG_KERNEL_DIAG_LCD
#define CYG_KERNEL_DIAG_SERIAL0

#if defined(CYG_HAL_USE_ROM_MONITOR)

#define CYG_KERNEL_DIAG_GDB

#endif

/*---------------------------------------------------------------------------*/

static cyg_uint8 leds = 0;

void hal_diag_led(int x)
{
//    return;
    leds ^= x;
    
    HAL_WRITE_UINT8( 0xfffff504, leds);

#if 0
    {
        int i;

        for( i = 0; i < 0x00020000; i++ );
    }
#endif    
}

/*---------------------------------------------------------------------------*/

#ifdef CYG_KERNEL_DIAG_SERIAL0

#define DIAG_BASE       0xfffff300
#define DIAG_SLCR       (DIAG_BASE+0x00)
#define DIAG_SLSR       (DIAG_BASE+0x04)
#define DIAG_SLDICR     (DIAG_BASE+0x08)
#define DIAG_SLDISR     (DIAG_BASE+0x0C)
#define DIAG_SFCR       (DIAG_BASE+0x10)
#define DIAG_SBRG       (DIAG_BASE+0x14)
#define DIAG_TFIFO      (DIAG_BASE+0x20)
#define DIAG_RFIFO      (DIAG_BASE+0x30)

#define BRG_T0          0x0000
#define BRG_T2          0x0100
#define BRG_T4          0x0200
#define BRG_T5          0x0300


void hal_diag_init()
{
#if defined(CYG_HAL_USE_ROM_MONITOR)
// If we are using the ROM monitor, it has already
// initialized the serial line.
#else
//hal_diag_led(0x10);    
    HAL_WRITE_UINT16( DIAG_SLCR , 0x0020 );

    HAL_WRITE_UINT16( DIAG_SLDICR , 0x0000 );
    
    HAL_WRITE_UINT16( DIAG_SFCR , 0x0000 );

#if CYGHWR_HAL_MIPS_CPU_FREQ == 50
//    HAL_WRITE_UINT16( DIAG_SBRG , BRG_T2 | 20 );    // 9600 bps
//    HAL_WRITE_UINT16( DIAG_SBRG , BRG_T2 | 10 );    // 19200 bps
    HAL_WRITE_UINT16( DIAG_SBRG , BRG_T2 | 5 );     // 38400 bps
#elif CYGHWR_HAL_MIPS_CPU_FREQ == 66
//    HAL_WRITE_UINT16( DIAG_SBRG , BRG_T2 | 27 );    // 9600 bps
//    HAL_WRITE_UINT16( DIAG_SBRG , BRG_T0 | 54 );    // 19200 bps
    HAL_WRITE_UINT16( DIAG_SBRG , BRG_T0 | 27 );    // 38400 bps
#else
#error Unsupported CPU frequency
#endif
//hal_diag_led(0x10);
#endif    
}

void hal_diag_write_char_serial0( char c)
{
    CYG_WORD16 disr;
    
//hal_diag_led(0x20);

    for(;;)
    {
        HAL_READ_UINT16( DIAG_SLDISR , disr );

        if( disr & 0x0002 ) break;
    }

    disr = disr & ~0x0002;
    
    HAL_WRITE_UINT8( DIAG_TFIFO, c );

    HAL_WRITE_UINT16( DIAG_SLDISR , disr );    

//hal_diag_led(0x20);
}

void hal_diag_drain_serial0(void)
{
    CYG_WORD16 disr;
    
    for(;;)
    {
        HAL_READ_UINT16( DIAG_SLDISR , disr );

        if( disr & 0x0002 ) break;
    }

    disr = disr & ~0x0002;
    
    HAL_WRITE_UINT16( DIAG_SLDISR , disr );    
}

void hal_diag_read_char_serial0(char *c)
{
    CYG_WORD16 disr;
    
//hal_diag_led(0x40);        
    for(;;)
    {
        
        HAL_READ_UINT16( DIAG_SLDISR , disr );

        if( disr & 0x0001 ) break;
    }

    disr = disr & ~0x0001;
    
    HAL_READ_UINT8( DIAG_RFIFO, *c );
    
    HAL_WRITE_UINT16( DIAG_SLDISR , disr );    

//hal_diag_led(0x40);
}


void hal_diag_write_char(char c)
{
#ifdef CYG_KERNEL_DIAG_GDB    
#if 0 //defined(CYG_HAL_USE_ROM_MONITOR)

    typedef void rom_write_fn(char c);
    rom_write_fn *fn = ((rom_write_fn **)0x80000100)[63];

    fn(c);
    
#else    
    static char line[100];
    static int pos = 0;
//    register volatile cyg_uint16 *volatile tty_status = SERIAL1_SR;    

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.
        CYG_INTERRUPT_STATE oldstate;
        HAL_DISABLE_INTERRUPTS(oldstate);
        
        while(1)
        {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
            char c1;
        
            hal_diag_write_char_serial0('$');
            hal_diag_write_char_serial0('O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_diag_write_char_serial0(h);
                hal_diag_write_char_serial0(l);
                csum += h;
                csum += l;
            }
            hal_diag_write_char_serial0('#');
            hal_diag_write_char_serial0(hex[(csum>>4)&0xF]);
            hal_diag_write_char_serial0(hex[csum&0xF]);

            hal_diag_read_char_serial0( &c1 );

            if( c1 == '+' ) break;

            if( cyg_hal_is_break( &c1 , 1 ) )
                cyg_hal_user_break( NULL );    
            
            break;
        }
        
        pos = 0;

        // Wait for all data from serial line to drain
        // and clear ready-to-send indication.
        hal_diag_drain_serial0();
        
        // And re-enable interrupts
        HAL_RESTORE_INTERRUPTS( oldstate );
        
    }
#endif    
#else
    hal_diag_write_char_serial0(c);
#endif    
}

void hal_diag_read_char(char *c)
{
    for(;;)
    {
#if defined(CYG_KERNEL_DIAG_GDB) && defined(CYG_HAL_USE_ROM_MONITOR)

        typedef void rom_read_fn(char *c);
        rom_read_fn *fn = ((rom_read_fn **)0x80000100)[62];

        fn(c);
    
#else    
        hal_diag_read_char_serial0(c);

#endif    

#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
        if( *c == 3 )
        {
            // Ctrl-C: breakpoint.
            extern void breakpoint(void);
            breakpoint();
            continue;
        }
#elif defined(CYG_HAL_USE_ROM_MONITOR)
        if( *c == 3 )
        {
            // Ctrl-C: breakpoint.

//                HAL_BREAKPOINT(_breakinst);
            typedef void bpt_fn(void);
            bpt_fn *bfn = ((bpt_fn **)0x80000100)[61];

            bfn();
            continue;            
        }
#endif

        break;      
    }
}

#endif

/*---------------------------------------------------------------------------*/

#if defined(CYGPKG_HAL_MIPS_TX39_JMR3904) && defined(CYG_KERNEL_DIAG_LCD)

/* ----------------------------------------------------------- */
#define ISA_BASE 0xA0000000
#define LCD_DATA *(volatile unsigned char*)(0x13400000+ISA_BASE)
#define LCD_CMD *(volatile unsigned char*)(0x13000000+ISA_BASE)

#define DISPCLR 0x01   /* Display Clear */
#define ECURINC 0x06   /* Cursor Increment */
#define DISPCONT 0x08   /* Display Control */
#define BLINK 0x01   /* Blink */
#define CURON 0x02   /* Cursor ON */
#define DISPON 0x04   /* Display ON */
#define INITCMD 0x38   /* Initial Command */
#define DDRAM 0x80   /* DDRAM address */
#define LCDBUSY 0x80   /* Busy */

/* ----------------------------------------------------------- */

/*                                          */
/* JMZ-LCD202 LCD Display Unit              */
/*     - Sample Program (for JMR-TX3904) -  */
/*                                          */

static void readyLCD(){
    while(LCD_CMD & LCDBUSY);
}

static void outLCD(unsigned char d){
    readyLCD();
    LCD_DATA = d;
}

static void outLCD_CMD(unsigned char d){
    readyLCD();
    LCD_CMD = d;
}

static void INIT_LCD(){
    outLCD_CMD(INITCMD);
    outLCD_CMD(DISPCONT);
    outLCD_CMD(DISPCLR);
    outLCD_CMD(ECURINC);
    outLCD_CMD(DISPCONT|BLINK|CURON|DISPON);
}

#if 0
static void MAIN(){
    int     i;
    static  char   c[]="JMZ-LCD202 LCD UNIT";
    static  char   d[]="Display Test Sample";

    INIT_LCD();
    outLCD_CMD(DDRAM);
        for (i=0;i<20;i++) outLCD(c[i]);
    outLCD_CMD(DDRAM+0x40);
        for (i=0;i<20;i++) outLCD(d[i]);
}
#endif

#define LCD_LINE0       0x00
#define LCD_LINE1       0x40

#define LCD_LINE_LENGTH 20

static char lcd_line0[LCD_LINE_LENGTH+1];
static char lcd_line1[LCD_LINE_LENGTH+1];
static char *lcd_line[2] = { lcd_line0, lcd_line1 };
static int lcd_curline = 0;
static int lcd_linepos = 0;

static void lcd_dis(int add, char *string);

void hal_diag_init()
{
    int i;
//hal_diag_led(0x10);    

    INIT_LCD();

    lcd_curline = 0;
    lcd_linepos = 0;

    for( i = 0; i < LCD_LINE_LENGTH; i++ )
        lcd_line[0][i] = lcd_line[1][i] = ' ';

    lcd_line[0][LCD_LINE_LENGTH] = lcd_line[1][LCD_LINE_LENGTH] = 0;

    lcd_dis( LCD_LINE0, lcd_line[0] );
    lcd_dis( LCD_LINE1, lcd_line[1] );

#if 0    
    {
        int     i;
        static  char   c[]="JMZ-LCD202 LCD UNIT";
        static  char   d[]="Display Test Sample";

        outLCD_CMD(DDRAM);
        for (i=0;i<20;i++) outLCD(c[i]);
        outLCD_CMD(DDRAM+0x40);
        for (i=0;i<20;i++) outLCD(d[i]);
    }
#endif
    
//hal_diag_led(0x10);
}

/* this routine writes the string to the LCD */
/* display after setting the address to add */
static void lcd_dis(int add, char *string)
{
    int i;
    
    outLCD_CMD(DDRAM+add);

    for (i=0 ; i<LCD_LINE_LENGTH ; i++) outLCD(string[i]);
}

void hal_diag_write_char( char c)
{
    int i;

//hal_diag_led(0x20);
    
    // Truncate long lines
    if( lcd_linepos >= LCD_LINE_LENGTH ) return;

    // ignore CR
    if( c == '\r' ) return;
    
    if( c == '\n' )
    {
        lcd_dis( LCD_LINE0, &lcd_line[lcd_curline^1][0] );
        lcd_dis( LCD_LINE1, &lcd_line[lcd_curline][0] );            

        // Do a line feed
        lcd_curline ^= 1;
        lcd_linepos = 0;
        
        for( i = 0; i < LCD_LINE_LENGTH; i++ )
            lcd_line[lcd_curline][i] = ' ';

        return;
    }

    lcd_line[lcd_curline][lcd_linepos++] = c;
    
//hal_diag_led(0x20);
}

void hal_diag_read_char(char *c)
{
//hal_diag_led(0x40);        

//hal_diag_led(0x40);
}


#endif


/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */

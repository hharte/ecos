// eCos memory layout - Wed Nov 17 08:10:54 1999

// This is a generated file - do not edit

#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#define CYGMEM_REGION_ram (0)
#define CYGMEM_REGION_ram_SIZE (0x1c0000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#define CYGMEM_REGION_rom (0xe0000000)
#define CYGMEM_REGION_rom_SIZE (0x40000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)
extern char CYG_LABEL_NAME (_reserved) [];
#define CYGMEM_SECTION_reserved (CYG_LABEL_NAME (_reserved))
#define CYGMEM_SECTION_reserved_SIZE (0x1000)

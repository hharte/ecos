// eCos memory layout - Tue Feb 29 14:25:49 2000

// This is a generated file - do not edit

#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#define CYGMEM_REGION_ram (0)
#define CYGMEM_REGION_ram_SIZE (0x80000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#define CYGMEM_REGION_rom (0x4000000)
#define CYGMEM_REGION_rom_SIZE (0x80000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)
extern char CYG_LABEL_NAME (_reserved) [];
#define CYGMEM_SECTION_reserved (CYG_LABEL_NAME (_reserved))
#define CYGMEM_SECTION_reserved_SIZE (0x800)

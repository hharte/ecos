// eCos memory layout - Tue Nov 09 06:59:24 1999

// This is a generated file - changes will be lost if ConfigTool(MLT) is run

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

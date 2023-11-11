#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/hal/hal_arch.h>

typedef struct {
    void *m_net_pool;
    int	 len;			/* Length of the entire packet */
} M_PKT_HDR;


typedef struct mblk
{
    struct mblk *m_next;
    struct mblk *m_nextpkt;
    char        *m_data;
    int          m_len;
    cyg_uint8    m_type;
    cyg_uint8    m_flags;
    cyg_uint16   m_reserved;
    M_PKT_HDR    mBlkPktHdr;
    cyg_uint32   m_key;
} M_BLK;


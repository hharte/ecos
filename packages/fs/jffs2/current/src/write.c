/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001-2003 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@redhat.com>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id$
 *
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/crc32.h>
#include <linux/slab.h>
#include <linux/pagemap.h>
#include <linux/mtd/mtd.h>
#include "nodelist.h"


int jffs2_do_new_inode(struct jffs2_sb_info *c, struct jffs2_inode_info *f, uint32_t mode, struct jffs2_raw_inode *ri)
{
	struct jffs2_inode_cache *ic;

	ic = jffs2_alloc_inode_cache();
	if (!ic) {
		return -ENOMEM;
	}

	memset(ic, 0, sizeof(*ic));

	f->inocache = ic;
	f->inocache->nlink = 1;
	f->inocache->nodes = (struct jffs2_raw_node_ref *)f->inocache;
	f->inocache->ino = ++c->highest_ino;
	f->inocache->state = INO_STATE_PRESENT;

	ri->ino = cpu_to_je32(f->inocache->ino);

	D1(printk(KERN_DEBUG "jffs2_do_new_inode(): Assigned ino# %d\n", f->inocache->ino));
	jffs2_add_ino_cache(c, f->inocache);

	ri->magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	ri->nodetype = cpu_to_je16(JFFS2_NODETYPE_INODE);
	ri->totlen = cpu_to_je32(PAD(sizeof(*ri)));
	ri->hdr_crc = cpu_to_je32(crc32(0, ri, sizeof(struct jffs2_unknown_node)-4));
	ri->mode = cpu_to_jemode(mode);

	f->highest_version = 1;
	ri->version = cpu_to_je32(f->highest_version);

	return 0;
}

#if CONFIG_JFFS2_FS_DEBUG > 0
static void writecheck(struct jffs2_sb_info *c, uint32_t ofs)
{
	unsigned char buf[16];
	size_t retlen;
	int ret, i;

	ret = jffs2_flash_read(c, ofs, 16, &retlen, buf);
	if (ret || (retlen != 16)) {
		D1(printk(KERN_DEBUG "read failed or short in writecheck(). ret %d, retlen %zd\n", ret, retlen));
		return;
	}
	ret = 0;
	for (i=0; i<16; i++) {
		if (buf[i] != 0xff)
			ret = 1;
	}
	if (ret) {
		printk(KERN_WARNING "ARGH. About to write node to 0x%08x on flash, but there are data already there:\n", ofs);
		printk(KERN_WARNING "0x%08x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
		       ofs,
		       buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
		       buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
	}
}
#endif


/* jffs2_write_dnode - given a raw_inode, allocate a full_dnode for it, 
   write it to the flash, link it into the existing inode/fragment list */

struct jffs2_full_dnode *jffs2_write_dnode(struct jffs2_sb_info *c, struct jffs2_inode_info *f, struct jffs2_raw_inode *ri, const unsigned char *data, uint32_t datalen, uint32_t flash_ofs, int alloc_mode)

{
	struct jffs2_raw_node_ref *raw;
	struct jffs2_full_dnode *fn;
	size_t retlen;
	struct iovec vecs[2];
	int ret;
	int retried = 0;
	unsigned long cnt = 2;

	D1(if(je32_to_cpu(ri->hdr_crc) != crc32(0, ri, sizeof(struct jffs2_unknown_node)-4)) {
		printk(KERN_CRIT "Eep. CRC not correct in jffs2_write_dnode()\n");
		BUG();
	}
	   );
	vecs[0].iov_base = ri;
	vecs[0].iov_len = sizeof(*ri);
	vecs[1].iov_base = (unsigned char *)data;
	vecs[1].iov_len = datalen;

	D1(writecheck(c, flash_ofs));

	if (je32_to_cpu(ri->totlen) != sizeof(*ri) + datalen) {
		printk(KERN_WARNING "jffs2_write_dnode: ri->totlen (0x%08x) != sizeof(*ri) (0x%08zx) + datalen (0x%08x)\n", je32_to_cpu(ri->totlen), sizeof(*ri), datalen);
	}
	raw = jffs2_alloc_raw_node_ref();
	if (!raw)
		return ERR_PTR(-ENOMEM);
	
	fn = jffs2_alloc_full_dnode();
	if (!fn) {
		jffs2_free_raw_node_ref(raw);
		return ERR_PTR(-ENOMEM);
	}

	fn->ofs = je32_to_cpu(ri->offset);
	fn->size = je32_to_cpu(ri->dsize);
	fn->frags = 0;

	/* check number of valid vecs */
	if (!datalen || !data)
		cnt = 1;
 retry:
	fn->raw = raw;

	raw->flash_offset = flash_ofs;
	raw->__totlen = PAD(sizeof(*ri)+datalen);
	raw->next_phys = NULL;

	ret = jffs2_flash_writev(c, vecs, cnt, flash_ofs, &retlen,
				 (alloc_mode==ALLOC_GC)?0:f->inocache->ino);

	if (ret || (retlen != sizeof(*ri) + datalen)) {
		printk(KERN_NOTICE "Write of %zd bytes at 0x%08x failed. returned %d, retlen %zd\n", 
		       sizeof(*ri)+datalen, flash_ofs, ret, retlen);

		/* Mark the space as dirtied */
		if (retlen) {
			/* Doesn't belong to any inode */
			raw->next_in_ino = NULL;

			/* Don't change raw->size to match retlen. We may have 
			   written the node header already, and only the data will
			   seem corrupted, in which case the scan would skip over
			   any node we write before the original intended end of 
			   this node */
			raw->flash_offset |= REF_OBSOLETE;
			jffs2_add_physical_node_ref(c, raw);
			jffs2_mark_node_obsolete(c, raw);
		} else {
			printk(KERN_NOTICE "Not marking the space at 0x%08x as dirty because the flash driver returned retlen zero\n", raw->flash_offset);
			jffs2_free_raw_node_ref(raw);
		}
		if (!retried && alloc_mode != ALLOC_NORETRY && (raw = jffs2_alloc_raw_node_ref())) {
			/* Try to reallocate space and retry */
			uint32_t dummy;
			struct jffs2_eraseblock *jeb = &c->blocks[flash_ofs / c->sector_size];

			retried = 1;

			D1(printk(KERN_DEBUG "Retrying failed write.\n"));
			
			ACCT_SANITY_CHECK(c,jeb);
			D1(ACCT_PARANOIA_CHECK(jeb));

			if (alloc_mode == ALLOC_GC) {
				ret = jffs2_reserve_space_gc(c, sizeof(*ri) + datalen, &flash_ofs, &dummy);
			} else {
				/* Locking pain */
				up(&f->sem);
				jffs2_complete_reservation(c);
			
				ret = jffs2_reserve_space(c, sizeof(*ri) + datalen, &flash_ofs, &dummy, alloc_mode);
				down(&f->sem);
			}

			if (!ret) {
				D1(printk(KERN_DEBUG "Allocated space at 0x%08x to retry failed write.\n", flash_ofs));

				ACCT_SANITY_CHECK(c,jeb);
				D1(ACCT_PARANOIA_CHECK(jeb));

				goto retry;
			}
			D1(printk(KERN_DEBUG "Failed to allocate space to retry failed write: %d!\n", ret));
			jffs2_free_raw_node_ref(raw);
		}
		/* Release the full_dnode which is now useless, and return */
		jffs2_free_full_dnode(fn);
		return ERR_PTR(ret?ret:-EIO);
	}
	/* Mark the space used */
	/* If node covers at least a whole page, or if it starts at the 
	   beginning of a page and runs to the end of the file, or if 
	   it's a hole node, mark it REF_PRISTINE, else REF_NORMAL. 
	*/
	if ((je32_to_cpu(ri->dsize) >= PAGE_CACHE_SIZE) ||
	    ( ((je32_to_cpu(ri->offset)&(PAGE_CACHE_SIZE-1))==0) &&
	      (je32_to_cpu(ri->dsize)+je32_to_cpu(ri->offset) ==  je32_to_cpu(ri->isize)))) {
		raw->flash_offset |= REF_PRISTINE;
	} else {
		raw->flash_offset |= REF_NORMAL;
	}
	jffs2_add_physical_node_ref(c, raw);

	/* Link into per-inode list */
	raw->next_in_ino = f->inocache->nodes;
	f->inocache->nodes = raw;

	D1(printk(KERN_DEBUG "jffs2_write_dnode wrote node at 0x%08x(%d) with dsize 0x%x, csize 0x%x, node_crc 0x%08x, data_crc 0x%08x, totlen 0x%08x\n",
		  flash_ofs, ref_flags(raw), je32_to_cpu(ri->dsize), 
		  je32_to_cpu(ri->csize), je32_to_cpu(ri->node_crc),
		  je32_to_cpu(ri->data_crc), je32_to_cpu(ri->totlen)));

	if (retried) {
		ACCT_SANITY_CHECK(c,NULL);
	}

	return fn;
}

struct jffs2_full_dirent *jffs2_write_dirent(struct jffs2_sb_info *c, struct jffs2_inode_info *f, struct jffs2_raw_dirent *rd, const unsigned char *name, uint32_t namelen, uint32_t flash_ofs, int alloc_mode)
{
	struct jffs2_raw_node_ref *raw;
	struct jffs2_full_dirent *fd;
	size_t retlen;
	struct iovec vecs[2];
	int retried = 0;
	int ret;

	D1(printk(KERN_DEBUG "jffs2_write_dirent(ino #%u, name at *0x%p \"%s\"->ino #%u, name_crc 0x%08x)\n", 
		  je32_to_cpu(rd->pino), name, name, je32_to_cpu(rd->ino),
		  je32_to_cpu(rd->name_crc)));
	D1(writecheck(c, flash_ofs));

	D1(if(je32_to_cpu(rd->hdr_crc) != crc32(0, rd, sizeof(struct jffs2_unknown_node)-4)) {
		printk(KERN_CRIT "Eep. CRC not correct in jffs2_write_dirent()\n");
		BUG();
	}
	   );

	vecs[0].iov_base = rd;
	vecs[0].iov_len = sizeof(*rd);
	vecs[1].iov_base = (unsigned char *)name;
	vecs[1].iov_len = namelen;
	
	raw = jffs2_alloc_raw_node_ref();

	if (!raw)
		return ERR_PTR(-ENOMEM);

	fd = jffs2_alloc_full_dirent(namelen+1);
	if (!fd) {
		jffs2_free_raw_node_ref(raw);
		return ERR_PTR(-ENOMEM);
	}

	fd->version = je32_to_cpu(rd->version);
	fd->ino = je32_to_cpu(rd->ino);
	fd->nhash = full_name_hash(name, strlen(name));
	fd->type = rd->type;
	memcpy(fd->name, name, namelen);
	fd->name[namelen]=0;

 retry:
	fd->raw = raw;

	raw->flash_offset = flash_ofs;
	raw->__totlen = PAD(sizeof(*rd)+namelen);
	raw->next_phys = NULL;

	ret = jffs2_flash_writev(c, vecs, 2, flash_ofs, &retlen,
				 (alloc_mode==ALLOC_GC)?0:je32_to_cpu(rd->pino));
	if (ret || (retlen != sizeof(*rd) + namelen)) {
		printk(KERN_NOTICE "Write of %zd bytes at 0x%08x failed. returned %d, retlen %zd\n", 
			       sizeof(*rd)+namelen, flash_ofs, ret, retlen);
		/* Mark the space as dirtied */
		if (retlen) {
			raw->next_in_ino = NULL;
			raw->flash_offset |= REF_OBSOLETE;
			jffs2_add_physical_node_ref(c, raw);
			jffs2_mark_node_obsolete(c, raw);
		} else {
			printk(KERN_NOTICE "Not marking the space at 0x%08x as dirty because the flash driver returned retlen zero\n", raw->flash_offset);
			jffs2_free_raw_node_ref(raw);
		}
		if (!retried && (raw = jffs2_alloc_raw_node_ref())) {
			/* Try to reallocate space and retry */
			uint32_t dummy;
			struct jffs2_eraseblock *jeb = &c->blocks[flash_ofs / c->sector_size];

			retried = 1;

			D1(printk(KERN_DEBUG "Retrying failed write.\n"));

			ACCT_SANITY_CHECK(c,jeb);
			D1(ACCT_PARANOIA_CHECK(jeb));

			if (alloc_mode == ALLOC_GC) {
				ret = jffs2_reserve_space_gc(c, sizeof(*rd) + namelen, &flash_ofs, &dummy);
			} else {
				/* Locking pain */
				up(&f->sem);
				jffs2_complete_reservation(c);
			
				ret = jffs2_reserve_space(c, sizeof(*rd) + namelen, &flash_ofs, &dummy, alloc_mode);
				down(&f->sem);
			}

			if (!ret) {
				D1(printk(KERN_DEBUG "Allocated space at 0x%08x to retry failed write.\n", flash_ofs));
				ACCT_SANITY_CHECK(c,jeb);
				D1(ACCT_PARANOIA_CHECK(jeb));
				goto retry;
			}
			D1(printk(KERN_DEBUG "Failed to allocate space to retry failed write: %d!\n", ret));
			jffs2_free_raw_node_ref(raw);
		}
		/* Release the full_dnode which is now useless, and return */
		jffs2_free_full_dirent(fd);
		return ERR_PTR(ret?ret:-EIO);
	}
	/* Mark the space used */
	raw->flash_offset |= REF_PRISTINE;
	jffs2_add_physical_node_ref(c, raw);

	raw->next_in_ino = f->inocache->nodes;
	f->inocache->nodes = raw;

	if (retried) {
		ACCT_SANITY_CHECK(c,NULL);
	}

	return fd;
}

/* The OS-specific code fills in the metadata in the jffs2_raw_inode for us, so that
   we don't have to go digging in struct inode or its equivalent. It should set:
   mode, uid, gid, (starting)isize, atime, ctime, mtime */
int jffs2_write_inode_range(struct jffs2_sb_info *c, struct jffs2_inode_info *f,
			    struct jffs2_raw_inode *ri, unsigned char *buf, 
			    uint32_t offset, uint32_t writelen, uint32_t *retlen)
{
	int ret = 0;
	uint32_t writtenlen = 0;

       	D1(printk(KERN_DEBUG "jffs2_write_inode_range(): Ino #%u, ofs 0x%x, len 0x%x\n",
		  f->inocache->ino, offset, writelen));
		
	while(writelen) {
		struct jffs2_full_dnode *fn;
		unsigned char *comprbuf = NULL;
		unsigned char comprtype = JFFS2_COMPR_NONE;
		uint32_t phys_ofs, alloclen;
		uint32_t datalen, cdatalen;
		int retried = 0;

	retry:
		D2(printk(KERN_DEBUG "jffs2_commit_write() loop: 0x%x to write to 0x%x\n", writelen, offset));

		ret = jffs2_reserve_space(c, sizeof(*ri) + JFFS2_MIN_DATA_LEN, &phys_ofs, &alloclen, ALLOC_NORMAL);
		if (ret) {
			D1(printk(KERN_DEBUG "jffs2_reserve_space returned %d\n", ret));
			break;
		}
		down(&f->sem);
		datalen = min_t(uint32_t, writelen, PAGE_CACHE_SIZE - (offset & (PAGE_CACHE_SIZE-1)));
		cdatalen = min_t(uint32_t, alloclen - sizeof(*ri), datalen);

		comprtype = jffs2_compress(buf, &comprbuf, &datalen, &cdatalen);

		ri->magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
		ri->nodetype = cpu_to_je16(JFFS2_NODETYPE_INODE);
		ri->totlen = cpu_to_je32(sizeof(*ri) + cdatalen);
		ri->hdr_crc = cpu_to_je32(crc32(0, ri, sizeof(struct jffs2_unknown_node)-4));

		ri->ino = cpu_to_je32(f->inocache->ino);
		ri->version = cpu_to_je32(++f->highest_version);
		ri->isize = cpu_to_je32(max(je32_to_cpu(ri->isize), offset + datalen));
		ri->offset = cpu_to_je32(offset);
		ri->csize = cpu_to_je32(cdatalen);
		ri->dsize = cpu_to_je32(datalen);
		ri->compr = comprtype;
		ri->node_crc = cpu_to_je32(crc32(0, ri, sizeof(*ri)-8));
		ri->data_crc = cpu_to_je32(crc32(0, comprbuf, cdatalen));

		fn = jffs2_write_dnode(c, f, ri, comprbuf, cdatalen, phys_ofs, ALLOC_NORETRY);

		jffs2_free_comprbuf(comprbuf, buf);

		if (IS_ERR(fn)) {
			ret = PTR_ERR(fn);
			up(&f->sem);
			jffs2_complete_reservation(c);
			if (!retried) {
				/* Write error to be retried */
				retried = 1;
				D1(printk(KERN_DEBUG "Retrying node write in jffs2_write_inode_range()\n"));
				goto retry;
			}
			break;
		}
		ret = jffs2_add_full_dnode_to_inode(c, f, fn);
		if (f->metadata) {
			jffs2_mark_node_obsolete(c, f->metadata->raw);
			jffs2_free_full_dnode(f->metadata);
			f->metadata = NULL;
		}
		if (ret) {
			/* Eep */
			D1(printk(KERN_DEBUG "Eep. add_full_dnode_to_inode() failed in commit_write, returned %d\n", ret));
			jffs2_mark_node_obsolete(c, fn->raw);
			jffs2_free_full_dnode(fn);

			up(&f->sem);
			jffs2_complete_reservation(c);
			break;
		}
		up(&f->sem);
		jffs2_complete_reservation(c);
		if (!datalen) {
			printk(KERN_WARNING "Eep. We didn't actually write any data in jffs2_write_inode_range()\n");
			ret = -EIO;
			break;
		}
		D1(printk(KERN_DEBUG "increasing writtenlen by %d\n", datalen));
		writtenlen += datalen;
		offset += datalen;
		writelen -= datalen;
		buf += datalen;
	}
	*retlen = writtenlen;
	return ret;
}

int jffs2_do_create(struct jffs2_sb_info *c, struct jffs2_inode_info *dir_f, struct jffs2_inode_info *f, struct jffs2_raw_inode *ri, const char *name, int namelen)
{
	struct jffs2_raw_dirent *rd;
	struct jffs2_full_dnode *fn;
	struct jffs2_full_dirent *fd;
	uint32_t alloclen, phys_ofs;
	int ret;

	/* Try to reserve enough space for both node and dirent. 
	 * Just the node will do for now, though 
	 */
	ret = jffs2_reserve_space(c, sizeof(*ri), &phys_ofs, &alloclen, ALLOC_NORMAL);
	D1(printk(KERN_DEBUG "jffs2_do_create(): reserved 0x%x bytes\n", alloclen));
	if (ret) {
		up(&f->sem);
		return ret;
	}

	ri->data_crc = cpu_to_je32(0);
	ri->node_crc = cpu_to_je32(crc32(0, ri, sizeof(*ri)-8));

	fn = jffs2_write_dnode(c, f, ri, NULL, 0, phys_ofs, ALLOC_NORMAL);

	D1(printk(KERN_DEBUG "jffs2_do_create created file with mode 0x%x\n",
		  jemode_to_cpu(ri->mode)));

	if (IS_ERR(fn)) {
		D1(printk(KERN_DEBUG "jffs2_write_dnode() failed\n"));
		/* Eeek. Wave bye bye */
		up(&f->sem);
		jffs2_complete_reservation(c);
		return PTR_ERR(fn);
	}
	/* No data here. Only a metadata node, which will be 
	   obsoleted by the first data write
	*/
	f->metadata = fn;

	up(&f->sem);
	jffs2_complete_reservation(c);
	ret = jffs2_reserve_space(c, sizeof(*rd)+namelen, &phys_ofs, &alloclen, ALLOC_NORMAL);
		
	if (ret) {
		/* Eep. */
		D1(printk(KERN_DEBUG "jffs2_reserve_space() for dirent failed\n"));
		return ret;
	}

	rd = jffs2_alloc_raw_dirent();
	if (!rd) {
		/* Argh. Now we treat it like a normal delete */
		jffs2_complete_reservation(c);
		return -ENOMEM;
	}

	down(&dir_f->sem);

	rd->magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	rd->nodetype = cpu_to_je16(JFFS2_NODETYPE_DIRENT);
	rd->totlen = cpu_to_je32(sizeof(*rd) + namelen);
	rd->hdr_crc = cpu_to_je32(crc32(0, rd, sizeof(struct jffs2_unknown_node)-4));

	rd->pino = cpu_to_je32(dir_f->inocache->ino);
	rd->version = cpu_to_je32(++dir_f->highest_version);
	rd->ino = ri->ino;
	rd->mctime = ri->ctime;
	rd->nsize = namelen;
	rd->type = DT_REG;
	rd->node_crc = cpu_to_je32(crc32(0, rd, sizeof(*rd)-8));
	rd->name_crc = cpu_to_je32(crc32(0, name, namelen));

	fd = jffs2_write_dirent(c, dir_f, rd, name, namelen, phys_ofs, ALLOC_NORMAL);

	jffs2_free_raw_dirent(rd);
	
	if (IS_ERR(fd)) {
		/* dirent failed to write. Delete the inode normally 
		   as if it were the final unlink() */
		jffs2_complete_reservation(c);
		up(&dir_f->sem);
		return PTR_ERR(fd);
	}

	/* Link the fd into the inode's list, obsoleting an old
	   one if necessary. */
	jffs2_add_fd_to_list(c, fd, &dir_f->dents);

	jffs2_complete_reservation(c);
	up(&dir_f->sem);

	return 0;
}


int jffs2_do_unlink(struct jffs2_sb_info *c, struct jffs2_inode_info *dir_f,
		    const char *name, int namelen, struct jffs2_inode_info *dead_f)
{
	struct jffs2_raw_dirent *rd;
	struct jffs2_full_dirent *fd;
	uint32_t alloclen, phys_ofs;
	int ret;

	rd = jffs2_alloc_raw_dirent();
	if (!rd)
		return -ENOMEM;

	ret = jffs2_reserve_space(c, sizeof(*rd)+namelen, &phys_ofs, &alloclen, ALLOC_DELETION);
	if (ret) {
		jffs2_free_raw_dirent(rd);
		return ret;
	}

	down(&dir_f->sem);

	/* Build a deletion node */
	rd->magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	rd->nodetype = cpu_to_je16(JFFS2_NODETYPE_DIRENT);
	rd->totlen = cpu_to_je32(sizeof(*rd) + namelen);
	rd->hdr_crc = cpu_to_je32(crc32(0, rd, sizeof(struct jffs2_unknown_node)-4));

	rd->pino = cpu_to_je32(dir_f->inocache->ino);
	rd->version = cpu_to_je32(++dir_f->highest_version);
	rd->ino = cpu_to_je32(0);
	rd->mctime = cpu_to_je32(get_seconds());
	rd->nsize = namelen;
	rd->type = DT_UNKNOWN;
	rd->node_crc = cpu_to_je32(crc32(0, rd, sizeof(*rd)-8));
	rd->name_crc = cpu_to_je32(crc32(0, name, namelen));

	fd = jffs2_write_dirent(c, dir_f, rd, name, namelen, phys_ofs, ALLOC_DELETION);
	
	jffs2_free_raw_dirent(rd);

	if (IS_ERR(fd)) {
		jffs2_complete_reservation(c);
		up(&dir_f->sem);
		return PTR_ERR(fd);
	}

	/* File it. This will mark the old one obsolete. */
	jffs2_add_fd_to_list(c, fd, &dir_f->dents);

	up(&dir_f->sem);
	
	/* dead_f is NULL if this was a rename not a real unlink */
	/* Also catch the !f->inocache case, where there was a dirent
	   pointing to an inode which didn't exist. */
	if (dead_f && dead_f->inocache) { 

		down(&dead_f->sem);

		while (dead_f->dents) {
			/* There can be only deleted ones */
			fd = dead_f->dents;
			
			dead_f->dents = fd->next;
			
			if (fd->ino) {
				printk(KERN_WARNING "Deleting inode #%u with active dentry \"%s\"->ino #%u\n",
				       dead_f->inocache->ino, fd->name, fd->ino);
			} else {
				D1(printk(KERN_DEBUG "Removing deletion dirent for \"%s\" from dir ino #%u\n", fd->name, dead_f->inocache->ino));
			}
			jffs2_mark_node_obsolete(c, fd->raw);
			jffs2_free_full_dirent(fd);
		}

		dead_f->inocache->nlink--;
		/* NB: Caller must set inode nlink if appropriate */
		up(&dead_f->sem);
	}

	jffs2_complete_reservation(c);

	return 0;
}


int jffs2_do_link (struct jffs2_sb_info *c, struct jffs2_inode_info *dir_f, uint32_t ino, uint8_t type, const char *name, int namelen)
{
	struct jffs2_raw_dirent *rd;
	struct jffs2_full_dirent *fd;
	uint32_t alloclen, phys_ofs;
	int ret;

	rd = jffs2_alloc_raw_dirent();
	if (!rd)
		return -ENOMEM;

	ret = jffs2_reserve_space(c, sizeof(*rd)+namelen, &phys_ofs, &alloclen, ALLOC_NORMAL);
	if (ret) {
		jffs2_free_raw_dirent(rd);
		return ret;
	}
	
	down(&dir_f->sem);

	/* Build a deletion node */
	rd->magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	rd->nodetype = cpu_to_je16(JFFS2_NODETYPE_DIRENT);
	rd->totlen = cpu_to_je32(sizeof(*rd) + namelen);
	rd->hdr_crc = cpu_to_je32(crc32(0, rd, sizeof(struct jffs2_unknown_node)-4));

	rd->pino = cpu_to_je32(dir_f->inocache->ino);
	rd->version = cpu_to_je32(++dir_f->highest_version);
	rd->ino = cpu_to_je32(ino);
	rd->mctime = cpu_to_je32(get_seconds());
	rd->nsize = namelen;

	rd->type = type;

	rd->node_crc = cpu_to_je32(crc32(0, rd, sizeof(*rd)-8));
	rd->name_crc = cpu_to_je32(crc32(0, name, namelen));

	fd = jffs2_write_dirent(c, dir_f, rd, name, namelen, phys_ofs, ALLOC_NORMAL);
	
	jffs2_free_raw_dirent(rd);

	if (IS_ERR(fd)) {
		jffs2_complete_reservation(c);
		up(&dir_f->sem);
		return PTR_ERR(fd);
	}

	/* File it. This will mark the old one obsolete. */
	jffs2_add_fd_to_list(c, fd, &dir_f->dents);

	jffs2_complete_reservation(c);
	up(&dir_f->sem);

	return 0;
}

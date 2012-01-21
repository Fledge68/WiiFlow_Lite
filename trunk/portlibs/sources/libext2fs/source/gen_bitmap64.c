/*
 * gen_bitmap64.c --- routines to read, write, and manipulate the new qinode and
 * block bitmaps.
 *
 * Copyright (C) 2007, 2008 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include <stdio.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "ext2_fs.h"
#include "ext2fsP.h"
#include "bmap64.h"

/*
 * Design of 64-bit bitmaps
 *
 * In order maintain ABI compatibility with programs that don't
 * understand about 64-bit blocks/inodes,
 * ext2fs_allocate_inode_bitmap() and ext2fs_allocate_block_bitmap()
 * will create old-style bitmaps unless the application passes the
 * flag EXT2_FLAG_64BITS to ext2fs_open().  If this flag is
 * passed, then we know the application has been recompiled, so we can
 * use the new-style bitmaps.  If it is not passed, we have to return
 * an error if trying to open a filesystem which needs 64-bit bitmaps.
 *
 * The new bitmaps use a new set of structure magic numbers, so that
 * both the old-style and new-style interfaces can identify which
 * version of the data structure was used.  Both the old-style and
 * new-style interfaces will support either type of bitmap, although
 * of course 64-bit operation will only be possible when both the
 * new-style interface and the new-style bitmap are used.
 *
 * For example, the new bitmap interfaces will check the structure
 * magic numbers and so will be able to detect old-stype bitmap.  If
 * they see an old-style bitmap, they will pass it to the gen_bitmap.c
 * functions for handling.  The same will be true for the old
 * interfaces as well.
 *
 * The new-style interfaces will have several different back-end
 * implementations, so we can support different encodings that are
 * appropriate for different applications.  In general the default
 * should be whatever makes sense, and what the application/library
 * will use.  However, e2fsck may need specialized implementations for
 * its own uses.  For example, when doing parent directory pointer
 * loop detections in pass 3, the bitmap will *always* be sparse, so
 * e2fsck can request an encoding which is optimized for that.
 */

static void warn_bitmap(ext2fs_generic_bitmap bitmap,
			int code, __u64 arg)
{
#ifndef OMIT_COM_ERR
	if (bitmap->description)
		com_err(0, bitmap->base_error_code+code,
			"#%llu for %s", arg, bitmap->description);
	else
		com_err(0, bitmap->base_error_code + code, "#%llu", arg);
#endif
}


errcode_t ext2fs_alloc_generic_bmap(ext2_filsys fs, errcode_t magic,
				    int type, __u64 start, __u64 end,
				    __u64 real_end,
				    const char *descr,
				    ext2fs_generic_bitmap *ret)
{
	ext2fs_generic_bitmap	bitmap;
	struct ext2_bitmap_ops	*ops;
	errcode_t retval;

	switch (type) {
	case EXT2FS_BMAP64_BITARRAY:
		ops = &ext2fs_blkmap64_bitarray;
		break;
	default:
		return EINVAL;
	}

	retval = ext2fs_get_mem(sizeof(struct ext2fs_struct_generic_bitmap),
				&bitmap);
	if (retval)
		return retval;

	/* XXX factor out, repeated in copy_bmap */
	bitmap->magic = magic;
	bitmap->fs = fs;
	bitmap->start = start;
	bitmap->end = end;
	bitmap->real_end = real_end;
	bitmap->bitmap_ops = ops;
	switch (magic) {
	case EXT2_ET_MAGIC_INODE_BITMAP64:
		bitmap->base_error_code = EXT2_ET_BAD_INODE_MARK;
		break;
	case EXT2_ET_MAGIC_BLOCK_BITMAP64:
		bitmap->base_error_code = EXT2_ET_BAD_BLOCK_MARK;
		break;
	default:
		bitmap->base_error_code = EXT2_ET_BAD_GENERIC_MARK;
	}
	if (descr) {
		retval = ext2fs_get_mem(strlen(descr)+1, &bitmap->description);
		if (retval) {
			ext2fs_free_mem(&bitmap);
			return retval;
		}
		strcpy(bitmap->description, descr);
	} else
		bitmap->description = 0;

	retval = bitmap->bitmap_ops->new_bmap(fs, bitmap);
	if (retval) {
		ext2fs_free_mem(&bitmap->description);
		ext2fs_free_mem(&bitmap);
		return retval;
	}

	*ret = bitmap;
	return 0;
}

void ext2fs_free_generic_bmap(ext2fs_generic_bitmap bmap)
{
	if (!bmap)
		return;

	if (EXT2FS_IS_32_BITMAP(bmap)) {
		ext2fs_free_generic_bitmap(bmap);
		return;
	}

	if (!EXT2FS_IS_64_BITMAP(bmap))
		return;

	bmap->bitmap_ops->free_bmap(bmap);

	if (bmap->description) {
		ext2fs_free_mem(&bmap->description);
		bmap->description = 0;
	}
	bmap->magic = 0;
	ext2fs_free_mem(&bmap);
}

errcode_t ext2fs_copy_generic_bmap(ext2fs_generic_bitmap src,
				   ext2fs_generic_bitmap *dest)
{
	char *descr, *new_descr;
	ext2fs_generic_bitmap	new_bmap;
	errcode_t retval;

	if (!src)
		return EINVAL;

	if (EXT2FS_IS_32_BITMAP(src))
		return ext2fs_copy_generic_bitmap(src, dest);

	if (!EXT2FS_IS_64_BITMAP(src))
		return EINVAL;

	/* Allocate a new bitmap struct */
	retval = ext2fs_get_mem(sizeof(struct ext2fs_struct_generic_bitmap),
				&new_bmap);
	if (retval)
		return retval;

	/* Copy all the high-level parts over */
	new_bmap->magic = src->magic;
	new_bmap->fs = src->fs;
	new_bmap->start = src->start;
	new_bmap->end = src->end;
	new_bmap->real_end = src->real_end;
	new_bmap->bitmap_ops = src->bitmap_ops;
	new_bmap->base_error_code = src->base_error_code;

	descr = src->description;
	if (descr) {
		retval = ext2fs_get_mem(strlen(descr)+1, &new_descr);
		if (retval) {
			ext2fs_free_mem(&new_bmap);
			return retval;
		}
		strcpy(new_descr, descr);
		new_bmap->description = new_descr;
	}

	retval = src->bitmap_ops->copy_bmap(src, new_bmap);
	if (retval) {
		ext2fs_free_mem(&new_bmap->description);
		ext2fs_free_mem(&new_bmap);
		return retval;
	}

	*dest = new_bmap;

	return 0;
}

errcode_t ext2fs_resize_generic_bmap(ext2fs_generic_bitmap bmap,
				     __u64 new_end,
				     __u64 new_real_end)
{
	if (!bmap)
		return EINVAL;

	if (EXT2FS_IS_32_BITMAP(bmap))
		return ext2fs_resize_generic_bitmap(bmap->magic, new_end,
						    new_real_end, bmap);

	if (!EXT2FS_IS_64_BITMAP(bmap))
		return EINVAL;

	return bmap->bitmap_ops->resize_bmap(bmap, new_end, new_real_end);
}

errcode_t ext2fs_fudge_generic_bmap_end(ext2fs_generic_bitmap bitmap,
					errcode_t neq,
					__u64 end, __u64 *oend)
{
	if (!bitmap)
		return EINVAL;

	if (EXT2FS_IS_32_BITMAP(bitmap)) {
		ext2_ino_t tmp_oend;
		int retval;

		retval = ext2fs_fudge_generic_bitmap_end(bitmap, bitmap->magic,
							 neq, end, &tmp_oend);
		if (oend)
			*oend = tmp_oend;
		return retval;
	}

	if (!EXT2FS_IS_64_BITMAP(bitmap))
		return EINVAL;

	if (end > bitmap->real_end)
		return neq;
	if (oend)
		*oend = bitmap->end;
	bitmap->end = end;
	return 0;
}

__u64 ext2fs_get_generic_bmap_start(ext2fs_generic_bitmap bitmap)
{
	if (!bitmap)
		return EINVAL;

	if (EXT2FS_IS_32_BITMAP(bitmap))
		return ext2fs_get_generic_bitmap_start(bitmap);

	if (!EXT2FS_IS_64_BITMAP(bitmap))
		return EINVAL;

	return bitmap->start;
}

__u64 ext2fs_get_generic_bmap_end(ext2fs_generic_bitmap bitmap)
{
	if (!bitmap)
		return EINVAL;

	if (EXT2FS_IS_32_BITMAP(bitmap))
		return ext2fs_get_generic_bitmap_end(bitmap);

	if (!EXT2FS_IS_64_BITMAP(bitmap))
		return EINVAL;

	return bitmap->end;
}

void ext2fs_clear_generic_bmap(ext2fs_generic_bitmap bitmap)
{
	if (EXT2FS_IS_32_BITMAP(bitmap))
		ext2fs_clear_generic_bitmap(bitmap);

	bitmap->bitmap_ops->clear_bmap (bitmap);
}

int ext2fs_mark_generic_bmap(ext2fs_generic_bitmap bitmap,
			     __u64 arg)
{
	if (!bitmap)
		return 0;

	if (EXT2FS_IS_32_BITMAP(bitmap)) {
		if (arg & ~0xffffffffULL) {
			ext2fs_warn_bitmap2(bitmap,
					    EXT2FS_MARK_ERROR, 0xffffffff);
			return 0;
		}
		return ext2fs_mark_generic_bitmap(bitmap, arg);
	}

	if (!EXT2FS_IS_64_BITMAP(bitmap))
		return 0;

	if ((arg < bitmap->start) || (arg > bitmap->end)) {
		warn_bitmap(bitmap, EXT2FS_MARK_ERROR, arg);
		return 0;
	}

	return bitmap->bitmap_ops->mark_bmap(bitmap, arg);
}

int ext2fs_unmark_generic_bmap(ext2fs_generic_bitmap bitmap,
			       __u64 arg)
{
	if (!bitmap)
		return 0;

	if (EXT2FS_IS_32_BITMAP(bitmap)) {
		if (arg & ~0xffffffffULL) {
			ext2fs_warn_bitmap2(bitmap, EXT2FS_UNMARK_ERROR,
					    0xffffffff);
			return 0;
		}
		return ext2fs_unmark_generic_bitmap(bitmap, arg);
	}

	if (!EXT2FS_IS_64_BITMAP(bitmap))
		return 0;

	if ((arg < bitmap->start) || (arg > bitmap->end)) {
		warn_bitmap(bitmap, EXT2FS_UNMARK_ERROR, arg);
		return 0;
	}

	return bitmap->bitmap_ops->unmark_bmap(bitmap, arg);
}

int ext2fs_test_generic_bmap(ext2fs_generic_bitmap bitmap,
			     __u64 arg)
{
	if (!bitmap)
		return 0;

	if (EXT2FS_IS_32_BITMAP(bitmap)) {
		if (arg & ~0xffffffffULL) {
			ext2fs_warn_bitmap2(bitmap, EXT2FS_TEST_ERROR,
					    0xffffffff);
			return 0;
		}
		return ext2fs_test_generic_bitmap(bitmap, arg);
	}

	if (!EXT2FS_IS_64_BITMAP(bitmap))
		return 0;

	if ((arg < bitmap->start) || (arg > bitmap->end)) {
		warn_bitmap(bitmap, EXT2FS_TEST_ERROR, arg);
		return 0;
	}

	return bitmap->bitmap_ops->test_bmap(bitmap, arg);
}

errcode_t ext2fs_set_generic_bmap_range(ext2fs_generic_bitmap bmap,
					__u64 start, unsigned int num,
					void *in)
{
	if (!bmap)
		return EINVAL;

	if (EXT2FS_IS_32_BITMAP(bmap)) {
		if ((start+num) & ~0xffffffffULL) {
			ext2fs_warn_bitmap2(bmap, EXT2FS_UNMARK_ERROR,
					    0xffffffff);
			return EINVAL;
		}
		return ext2fs_set_generic_bitmap_range(bmap, bmap->magic,
						       start, num, in);
	}

	if (!EXT2FS_IS_64_BITMAP(bmap))
		return EINVAL;

	return bmap->bitmap_ops->set_bmap_range(bmap, start, num, in);
}

errcode_t ext2fs_get_generic_bmap_range(ext2fs_generic_bitmap bmap,
					__u64 start, unsigned int num,
					void *out)
{
	if (!bmap)
		return EINVAL;

	if (EXT2FS_IS_32_BITMAP(bmap)) {
		if ((start+num) & ~0xffffffffULL) {
			ext2fs_warn_bitmap2(bmap,
					    EXT2FS_UNMARK_ERROR, 0xffffffff);
			return EINVAL;
		}
		return ext2fs_get_generic_bitmap_range(bmap, bmap->magic,
						       start, num, out);
	}

	if (!EXT2FS_IS_64_BITMAP(bmap))
		return EINVAL;

	return bmap->bitmap_ops->get_bmap_range(bmap, start, num, out);
}

errcode_t ext2fs_compare_generic_bmap(errcode_t neq,
				      ext2fs_generic_bitmap bm1,
				      ext2fs_generic_bitmap bm2)
{
	blk64_t	i;

	if (!bm1 || !bm2)
		return EINVAL;
	if (bm1->magic != bm2->magic)
		return EINVAL;

	/* Now we know both bitmaps have the same magic */
	if (EXT2FS_IS_32_BITMAP(bm1))
		return ext2fs_compare_generic_bitmap(bm1->magic, neq, bm1, bm2);

	if (!EXT2FS_IS_64_BITMAP(bm1))
		return EINVAL;

	if ((bm1->start != bm2->start) ||
	    (bm1->end != bm2->end))
		return neq;

	for (i = bm1->end - ((bm1->end - bm1->start) % 8); i <= bm1->end; i++)
		if (ext2fs_test_generic_bmap(bm1, i) !=
		    ext2fs_test_generic_bmap(bm2, i))
			return neq;

	return 0;
}

void ext2fs_set_generic_bmap_padding(ext2fs_generic_bitmap bmap)
{
	__u64	start, num;

	if (EXT2FS_IS_32_BITMAP(bmap)) {
		ext2fs_set_generic_bitmap_padding(bmap);
		return;
	}

	start = bmap->end + 1;
	num = bmap->real_end - bmap->end;
	bmap->bitmap_ops->mark_bmap_extent(bmap, start, num);
	/* XXX ought to warn on error */
}

int ext2fs_test_block_bitmap_range2(ext2fs_block_bitmap bmap,
				    blk64_t block, unsigned int num)
{
	if (!bmap)
		return EINVAL;

	if (num == 1)
		return !ext2fs_test_generic_bmap((ext2fs_generic_bitmap)
						 bmap, block);

	if (EXT2FS_IS_32_BITMAP(bmap)) {
		if ((block+num) & ~0xffffffffULL) {
			ext2fs_warn_bitmap2((ext2fs_generic_bitmap) bmap,
					    EXT2FS_UNMARK_ERROR, 0xffffffff);
			return EINVAL;
		}
		return ext2fs_test_block_bitmap_range(
			(ext2fs_generic_bitmap) bmap, block, num);
	}

	if (!EXT2FS_IS_64_BITMAP(bmap))
		return EINVAL;

	return bmap->bitmap_ops->test_clear_bmap_extent(bmap, block, num);
}

void ext2fs_mark_block_bitmap_range2(ext2fs_block_bitmap bmap,
				     blk64_t block, unsigned int num)
{
	if (!bmap)
		return;

	if (EXT2FS_IS_32_BITMAP(bmap)) {
		if ((block+num) & ~0xffffffffULL) {
			ext2fs_warn_bitmap2((ext2fs_generic_bitmap) bmap,
					    EXT2FS_UNMARK_ERROR, 0xffffffff);
			return;
		}
		ext2fs_mark_block_bitmap_range((ext2fs_generic_bitmap) bmap,
					       block, num);
	}

	if (!EXT2FS_IS_64_BITMAP(bmap))
		return;

	if ((block < bmap->start) || (block+num-1 > bmap->end)) {
		ext2fs_warn_bitmap(EXT2_ET_BAD_BLOCK_MARK, block,
				   bmap->description);
		return;
	}

	bmap->bitmap_ops->mark_bmap_extent(bmap, block, num);
}

void ext2fs_unmark_block_bitmap_range2(ext2fs_block_bitmap bmap,
				       blk64_t block, unsigned int num)
{
	if (!bmap)
		return;

	if (EXT2FS_IS_32_BITMAP(bmap)) {
		if ((block+num) & ~0xffffffffULL) {
			ext2fs_warn_bitmap2((ext2fs_generic_bitmap) bmap,
					    EXT2FS_UNMARK_ERROR, 0xffffffff);
			return;
		}
		ext2fs_unmark_block_bitmap_range((ext2fs_generic_bitmap) bmap,
						 block, num);
	}

	if (!EXT2FS_IS_64_BITMAP(bmap))
		return;

	if ((block < bmap->start) || (block+num-1 > bmap->end)) {
		ext2fs_warn_bitmap(EXT2_ET_BAD_BLOCK_UNMARK, block,
				   bmap->description);
		return;
	}

	bmap->bitmap_ops->unmark_bmap_extent(bmap, block, num);
}

int ext2fs_warn_bitmap32(ext2fs_generic_bitmap bitmap, const char *func)
{
#ifndef OMIT_COM_ERR
	if (bitmap && bitmap->description)
		com_err(0, EXT2_ET_MAGIC_GENERIC_BITMAP,
			"called %s with 64-bit bitmap for %s", func,
			bitmap->description);
	else
		com_err(0, EXT2_ET_MAGIC_GENERIC_BITMAP,
			"called %s with 64-bit bitmap", func);
#endif
    return 0;
}

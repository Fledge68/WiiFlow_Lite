/*
 * bmap64.h --- 64-bit bitmap structure
 *
 * Copyright (C) 2007, 2008 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

struct ext2fs_struct_generic_bitmap {
	errcode_t		magic;
	ext2_filsys 		fs;
	struct ext2_bitmap_ops	*bitmap_ops;
	int			flags;
	__u64			start, end;
	__u64			real_end;
	int			cluster_bits;
	char			*description;
	void			*private;
	errcode_t		base_error_code;
};

#define EXT2FS_IS_32_BITMAP(bmap) \
	(((bmap)->magic == EXT2_ET_MAGIC_GENERIC_BITMAP) || \
	 ((bmap)->magic == EXT2_ET_MAGIC_BLOCK_BITMAP) || \
	 ((bmap)->magic == EXT2_ET_MAGIC_INODE_BITMAP))

#define EXT2FS_IS_64_BITMAP(bmap) \
	(((bmap)->magic == EXT2_ET_MAGIC_GENERIC_BITMAP64) || \
	 ((bmap)->magic == EXT2_ET_MAGIC_BLOCK_BITMAP64) || \
	 ((bmap)->magic == EXT2_ET_MAGIC_INODE_BITMAP64))

struct ext2_bitmap_ops {
	int	type;
	/* Generic bmap operators */
	errcode_t (*new_bmap)(ext2_filsys fs, ext2fs_generic_bitmap bmap);
	void	(*free_bmap)(ext2fs_generic_bitmap bitmap);
	errcode_t (*copy_bmap)(ext2fs_generic_bitmap src,
			     ext2fs_generic_bitmap dest);
	errcode_t (*resize_bmap)(ext2fs_generic_bitmap bitmap,
			       __u64 new_end,
			       __u64 new_real_end);
	/* bit set/test operators */
	int	(*mark_bmap)(ext2fs_generic_bitmap bitmap, __u64 arg);
	int	(*unmark_bmap)(ext2fs_generic_bitmap bitmap, __u64 arg);
	int	(*test_bmap)(ext2fs_generic_bitmap bitmap, __u64 arg);
	void	(*mark_bmap_extent)(ext2fs_generic_bitmap bitmap, __u64 arg,
				    unsigned int num);
	void	(*unmark_bmap_extent)(ext2fs_generic_bitmap bitmap, __u64 arg,
				      unsigned int num);
	int	(*test_clear_bmap_extent)(ext2fs_generic_bitmap bitmap,
					  __u64 arg, unsigned int num);
	errcode_t (*set_bmap_range)(ext2fs_generic_bitmap bitmap,
				    __u64 start, size_t num, void *in);
	errcode_t (*get_bmap_range)(ext2fs_generic_bitmap bitmap,
				    __u64 start, size_t num, void *out);
	void (*clear_bmap)(ext2fs_generic_bitmap bitmap);
};

extern struct ext2_bitmap_ops ext2fs_blkmap64_bitarray;

/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall fusexmp.c `pkg-config fuse --cflags --libs` -o fusexmp
*/

#define FUSE_USE_VERSION 30

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include <tsk3/libtsk.h>
#include "tsk3/tsk_tools_i.h"

#include "upartsfs.h"

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;

	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;

	res = access(path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}

static struct UPARTS_DE_INFO * last_de_info(const struct UPARTS_DE_INFO * ude, uint8_t * list_position)
{
	struct UPARTS_DE_INFO * tail;
	uint8_t lp;

	//Debug fprintf(stderr, "last_de_info: ude = %p\n", ude);
	//Debug fprintf(stderr, "last_de_info: ude->next = %p\n", ude->next);

	if (ude == NULL) {
		return NULL;
	}

	lp = 0;

	for (tail = ude; tail->next != NULL; tail = tail->next) {
		//Debug fprintf(stderr, "last_de_info: lp = %" PRIu8 "\n", lp);
		//Debug fprintf(stderr, "last_de_info: tail = %p\n", tail);
		//Debug fprintf(stderr, "last_de_info: tail->next = %p\n", tail->next);
		lp++;
	}

	if (list_position != NULL) {
		*list_position = lp;
	}

	return tail;
}

static TSK_WALK_RET_ENUM populate_uparts_by_index(TSK_VS_INFO * vs, const TSK_VS_PART_INFO * part, void *ptr)
{
	struct UPARTS_EXTRA * uparts_extra = (struct UPARTS_EXTRA *) ptr;
	struct UPARTS_DE_INFO * sbi_tail;
	struct UPARTS_DE_INFO * new_tail;
	uint8_t old_tail_position = 0;
	int rc;

	/* Guarantee list is initialized */
	if (uparts_extra->stats_by_index == NULL) {
		uparts_extra->stats_by_index = (struct UPARTS_DE_INFO *) tsk_malloc(sizeof(struct UPARTS_DE_INFO));
		if (uparts_extra->stats_by_index == NULL) {
			tsk_error_print(stderr);
			return TSK_WALK_ERROR;
		}
	}
	
	/* Fetch list tail */
	sbi_tail = last_de_info(uparts_extra->stats_by_index, &old_tail_position);
	if (sbi_tail == NULL) {
		/* TODO Describe error */
		return TSK_WALK_ERROR;
	}

	/* Allocate new list tail */
	new_tail = (struct UPARTS_DE_INFO *) tsk_malloc(sizeof(struct UPARTS_DE_INFO));
	if (new_tail == NULL) {
		tsk_error_print(stderr);
		return TSK_WALK_ERROR;
	}

	/* Populate new list tail data */
	new_tail->encounter_order = old_tail_position + 1;
	rc = snprintf(new_tail->name, UPARTS_NAME_LENGTH, "%" PRIu8, new_tail->encounter_order);
	if (rc < 0) {
		/* TODO Describe error */
		fprintf(stderr, "populate_uparts_by_index: snprintf error");
		free(new_tail);
		return TSK_WALK_ERROR;
	}
	/* TODO Fill stat struct of new tail*/
	/* TODO Get ssize from vs or img_info */
	uint64_t ssize = 512;
	new_tail->offset = part->start * ssize; 
	new_tail->st.st_size = part->len * ssize;
	new_tail->st.st_ino = 22; /* TODO Compact partition offset into ino_t */

	/* Append new tail to list */
	new_tail->prev = sbi_tail;
	sbi_tail->next = new_tail;

	return TSK_WALK_CONT;
}

static int uparts_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_UTIMENSAT
static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int uparts_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	return -EOPNOTSUPP;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int xmp_fallocate(const char *path, int mode,
			off_t offset, off_t length, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	if (mode)
		return -EOPNOTSUPP;

	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = -posix_fallocate(fd, offset, length);

	close(fd);
	return res;
}
#endif

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations upartsfs_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= uparts_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
#ifdef HAVE_UTIMENSAT
	.utimens	= xmp_utimens,
#endif
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= uparts_write,
	.statfs		= xmp_statfs,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_POSIX_FALLOCATE
	.fallocate	= xmp_fallocate,
#endif
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

/**
 * Clean up uparts_extra struct
 */
int free_uparts_extra(struct UPARTS_EXTRA * uparts_extra)
{
	/* Base case: Received null pointer */
	if (uparts_extra == NULL) return 0;

	/* Walk by-index list, freeing tail-forward.  This frees all partitions. */
	/* TODO */

	uparts_extra->stats_by_offset = NULL;

	free(uparts_extra);
	return 0;
}

int main(int argc, char *argv[])
{
	TSK_VS_INFO *vs;
	int ch;
	TSK_OFF_T imgaddr = 0;
	TSK_VS_PART_FLAG_ENUM flags = 0;
	TSK_IMG_TYPE_ENUM imgtype = TSK_IMG_TYPE_DETECT;
	TSK_VS_TYPE_ENUM vstype = TSK_VS_TYPE_DETECT;
	TSK_IMG_INFO *img;
	unsigned int ssize = 0; /* AJN TODO Wait, is this the same type as elsewhere? */
	struct UPARTS_EXTRA *uparts_extra;
	struct UPARTS_DE_INFO *ude;

	while ((ch = GETOPT(argc, argv, _TSK_T("aAb:Bi:mMo:rt:vV"))) > 0) {
		/* AJN TODO Nop for now. Just want OPTIND to be set.*/
	}

	/* Open image */
	img = tsk_img_open(argc - OPTIND - 1, &argv[OPTIND], imgtype, ssize);
	if (img == NULL) {
		tsk_error_print(stderr);
		exit(1);
	}

	/* Open partition table */
	vs = tsk_vs_open(img, imgaddr * img->sector_size, vstype);
	if (vs == NULL) {
		tsk_error_print(stderr);
		if (tsk_error_get_errno() == TSK_ERR_VS_UNSUPTYPE)
			tsk_vs_type_print(stderr);
		tsk_img_close(img);
		exit(1);
	}

	/* Initialize extra-data struct */
	uparts_extra = (struct UPARTS_EXTRA *) tsk_malloc(sizeof(struct UPARTS_EXTRA));
	if (uparts_extra == NULL) {
		tsk_error_print(stderr);
		tsk_vs_close(vs);
		tsk_img_close(img);
		exit(1);
	}

	/* Walk partition table to populate extra-data struct */
	if (tsk_vs_part_walk(vs, 0, vs->part_count - 1, flags, populate_uparts_by_index, uparts_extra)) {
		tsk_error_print(stderr);
		free_uparts_extra(uparts_extra);
		tsk_vs_close(vs);
		tsk_img_close(img);
		exit(1);
	}

	/* Debug */
	fprintf(stderr, "main: Debug: Partitions by index:\n");
	for (ude = uparts_extra->stats_by_index;
	     ude != NULL && ude->next != NULL;
	     ude = ude->next) {
		fprintf(stderr, "\t%" PRIu8 "\t%s\n", ude->encounter_order, ude->name);
	}

	umask(0);
	return fuse_main(argc, argv, &upartsfs_oper, uparts_extra);
}

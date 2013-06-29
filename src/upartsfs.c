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

#include <tsk/libtsk.h>
#include "tsk/tsk_tools_i.h"

#include "upartsfs.h"

static int uparts_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strncmp(path, "/", 2) == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 4;
	} else if (strncmp(path, "/in_order", 10) == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strncmp(path, "/by_offset", 11) == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (0) {
		/* TODO Handle actual partition files */
		return -ENOTSUP;
	} else {
		return -ENOENT;
	}

	return res;
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
	struct UPARTS_DE_INFO * tail = NULL;
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
	uint64_t ssize = vs->block_size;
	struct UPARTS_EXTRA * uparts_extra = (struct UPARTS_EXTRA *) ptr;
	struct UPARTS_DE_INFO * sbi_tail = NULL;
	struct UPARTS_DE_INFO * new_tail;
	uint8_t old_tail_position = 0;
	uint8_t new_tail_position = 0;
	int rc;

	/* Get tail of list, creating list if need be */
	if (uparts_extra->stats_by_index == NULL) {
		uparts_extra->stats_by_index = (struct UPARTS_DE_INFO *) tsk_malloc(sizeof(struct UPARTS_DE_INFO));
		if (uparts_extra->stats_by_index == NULL) {
			tsk_error_print(stderr);
			return TSK_WALK_ERROR;
		}
		new_tail_position = 0;
	} else {
		/* Fetch list tail */
		sbi_tail = last_de_info(uparts_extra->stats_by_index, &old_tail_position);
		if (sbi_tail == NULL) {
			/* TODO Describe error */
			return TSK_WALK_ERROR;
		}
		new_tail_position = old_tail_position + 1;
	}

	/* Allocate new list tail */
	new_tail = (struct UPARTS_DE_INFO *) tsk_malloc(sizeof(struct UPARTS_DE_INFO));
	if (new_tail == NULL) {
		tsk_error_print(stderr);
		return TSK_WALK_ERROR;
	}

	/* Populate new list tail data */
	new_tail->encounter_order = new_tail_position;
	rc = snprintf(new_tail->name, UPARTS_NAME_LENGTH, "%" PRIu8, new_tail_position);
	if (rc < 0) {
		/* TODO Describe error */
		fprintf(stderr, "populate_uparts_by_index: snprintf error");
		free(new_tail);
		return TSK_WALK_ERROR;
	}
	/* TODO Fill stat struct of new tail*/
	new_tail->offset = part->start * ssize; 
	new_tail->st.st_size = part->len * ssize;
	new_tail->st.st_ino = 22; /* TODO Compact partition offset into ino_t */

	/* Append new tail to list */
	if (sbi_tail) {
		new_tail->prev = sbi_tail;
		sbi_tail->next = new_tail;
	} else {
		sbi_tail = new_tail;
	}

	return TSK_WALK_CONT;
}

static int uparts_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	struct UPARTS_DE_INFO *ude = NULL;
	if (uparts_extra == NULL) {
		fprintf(stderr, "uparts_readdir: uparts_extra is null.\n");
		return -EIO;
	}

	/* Select list for populating directory */
	if (strncmp(path, "/", 2) == 0) {
		/* (The root directory isn't terribly dynamic) */
	       	filler(buf, ".", NULL, 0);
	       	filler(buf, "..", NULL, 0);
		filler(buf, "by_offset", NULL, 0); /*TODO Specify these are directories */
		filler(buf, "in_order", NULL, 0);
		return 0;
	} else if (strncmp(path, "/by_offset", 11) == 0) {
		ude = uparts_extra->stats_by_offset;
	} else if (strncmp(path, "/in_order", 10) == 0) {
		ude = uparts_extra->stats_by_index;
	} else {
		return -ENOENT;
	}

	/* Populate directory */
       	filler(buf, ".", NULL, 0);
       	filler(buf, "..", NULL, 0);
	for (ude;
	     ude != NULL && ude->next != NULL;
	     ude = ude->next) {
		if (filler(buf, ude->name, &(ude->st), 0))
			break;
	}

	return 0;

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

static int uparts_mknod(const char *path, mode_t mode, dev_t rdev)
{
	return -EOPNOTSUPP;
}

static int uparts_mkdir(const char *path, mode_t mode)
{
	return -EOPNOTSUPP;
}

static int uparts_unlink(const char *path)
{
	return -EOPNOTSUPP;
}

static int uparts_rmdir(const char *path)
{
	return -EOPNOTSUPP;
}

static int uparts_symlink(const char *from, const char *to)
{
	return -EOPNOTSUPP;
}

static int uparts_rename(const char *from, const char *to)
{
	return -EOPNOTSUPP;
}

static int uparts_link(const char *from, const char *to)
{
	return -EOPNOTSUPP;
}

static int uparts_chmod(const char *path, mode_t mode)
{
	return -EOPNOTSUPP;
}

static int uparts_chown(const char *path, uid_t uid, gid_t gid)
{
	return -EOPNOTSUPP;
}

static int uparts_truncate(const char *path, off_t size)
{
	return -EOPNOTSUPP;
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

/* NB a FUSE-ism: What this function returns becomes context->private_data.  Hence, returning context->private_data is the closest to a nop-implementation. */
static void * uparts_init(struct fuse_conn_info *conn)
{
	//Debug fprintf(stderr, "Debug: uparts_init(...)\n");

	//Debug fprintf(stderr, "Debug: uparts_init: Getting context...\n");
	struct fuse_context * context = fuse_get_context();
	//Debug fprintf(stderr, "Debug: uparts_init: Got context.\n");
	//Debug fprintf(stderr, "Debug: uparts_init: context @ %p\n", context);
	//Debug fprintf(stderr, "Debug: uparts_init: context->private_data = %p\n", context->private_data);

	return context->private_data;
}

static void uparts_destroy(void * private_data)
{
	//Debug fprintf(stderr, "Debug: uparts_destroy(...)\n");
	//Debug fprintf(stderr, "Debug: uparts_destroy: private_data @ %p\n", private_data);
	struct fuse_context * context = fuse_get_context();
	//Debug fprintf(stderr, "Debug: uparts_destroy: Got context.\n");
	//Debug fprintf(stderr, "Debug: uparts_destroy: context @ %p\n", context);
	//Debug fprintf(stderr, "Debug: uparts_destroy: context->private_data = %p\n", context->private_data);
	free_uparts_extra(context->private_data);
	return;
}

static struct fuse_operations upartsfs_oper = {
	.init		= uparts_init,
	.destroy	= uparts_destroy,
	.getattr	= uparts_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= uparts_readdir,
	.mknod		= uparts_mknod,
	.mkdir		= uparts_mkdir,
	.symlink	= uparts_symlink,
	.unlink		= uparts_unlink,
	.rmdir		= uparts_rmdir,
	.rename		= uparts_rename,
	.link		= uparts_link,
	.chmod		= uparts_chmod,
	.chown		= uparts_chown,
	.truncate	= uparts_truncate,
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
	struct UPARTS_DE_INFO *ude_head;
	struct UPARTS_DE_INFO *ude_next;

	/* Base case: Received null pointer */
	if (uparts_extra == NULL) return 0;
	if (uparts_extra->stats_by_index == NULL) return 0;

	/* Walk by-index list, freeing head-forward.  This frees all partitions. */
	ude_head = uparts_extra->stats_by_index;
	while (ude_head != NULL) {
		ude_next = ude_head->next;
		free(ude_head);
		ude_head = ude_next;
	}

	uparts_extra->stats_by_offset = NULL;

	tsk_vs_close(uparts_extra->tsk_vs);
	tsk_img_close(uparts_extra->tsk_img);

	free(uparts_extra);
	return 0;
}

int main(int main_argc, char *main_argv[])
{
	int tsk_argc;
	TSK_TCHAR **tsk_argv;
	TSK_TCHAR **tsk_argv_tmp;
	char * tsk_opts;
	TSK_VS_INFO *vs;
	int ch;
	TSK_OFF_T imgaddr = 0;
	TSK_VS_PART_FLAG_ENUM flags = 0;
	TSK_IMG_TYPE_ENUM imgtype = TSK_IMG_TYPE_DETECT;
	TSK_VS_TYPE_ENUM vstype = TSK_VS_TYPE_DETECT;
	TSK_IMG_INFO *img;
	unsigned int ssize = 0; /* AJN TODO Wait, is this the same type as elsewhere? */
	struct UPARTS_DE_INFO *ude;

	/* Pointer acrobatics ensue to split FUSE command line parameters from image file list for TSK */
#ifdef TSK_WIN32
	// On Windows, get the wide arguments (mingw doesn't support wmain)
	tsk_argv_tmp = CommandLineToArgvW(GetCommandLineW(), &main_argc);
	if (tsk_argv_tmp == NULL) {
		fprintf(stderr, "Error getting wide arguments\n");
		exit(1);
	}
#else
	tsk_argv_tmp = (TSK_TCHAR **) main_argv;
#endif
	/* Just pass the last argument to TSK for now; some disk images are multi-file */
	/* TODO This would be better handled with a "--", or getopt parsing out options and the FUSE mount point */
	tsk_argc = 2;
	tsk_argv = (TSK_TCHAR **) tsk_malloc(tsk_argc * sizeof(TSK_TCHAR *));
	if (tsk_argv == NULL) {
		tsk_error_print(stderr);
		exit(1);
	}
	tsk_argv[0] = tsk_argv_tmp[0];
	tsk_argv[1] = tsk_argv_tmp[main_argc - 1];
	//Debug fprintf(stderr, "Debug: image file arguments:\n");
	//Debug fprintf(stderr, "\t%s\n", tsk_argv[1]);

	/* AJN Nothing interesting about these options specified yet... */
	tsk_opts = "d:";
	while ((ch = GETOPT(tsk_argc, tsk_argv, _TSK_T(tsk_opts))) > 0) {
		/* AJN TODO Nop for now. Just want OPTIND to be set.*/
	}

	/* Open image */
	//Debug fprintf(stderr, "Debug: OPTIND = %" PRIu32 "\n", OPTIND);
	img = tsk_img_open(tsk_argc - OPTIND, &tsk_argv[OPTIND], imgtype, ssize);
	if (img == NULL) {
		tsk_error_print(stderr);
		free(tsk_argv);
		exit(1);
	}

	free(tsk_argv);

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
	//Debug fprintf(stderr, "Debug: main: uparts_extra @ %p\n", uparts_extra);
	/* From here forward, free_uparts_extra handles cleaning up TSK structures */
	uparts_extra->tsk_img = img;
	uparts_extra->tsk_vs = vs;

	/* Walk partition table to populate extra-data struct */
	if (tsk_vs_part_walk(vs, 0, vs->part_count - 1, flags, populate_uparts_by_index, uparts_extra)) {
		tsk_error_print(stderr);
		free_uparts_extra(uparts_extra);
		exit(1);
	}

	/* Walk in-table-order partition list to generate by-offset partition list */
	/* TODO */

	/* Debug */
	fprintf(stderr, "main: Debug: Partitions by index:\n");
	for (ude = uparts_extra->stats_by_index;
	     ude != NULL && ude->next != NULL;
	     ude = ude->next) {
		fprintf(stderr, "\t%" PRIu8 "\t%s\n", ude->encounter_order, ude->name);
	}

	umask(0);
	/* Just pass the arguments less the last to fuse_main for now. */
	int fuse_argc = main_argc - 1;
	char **fuse_argv = main_argv;

	return fuse_main(fuse_argc, fuse_argv, &upartsfs_oper, uparts_extra);
}

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

static int logged_getattr(const char *path, struct stat *stbuf)
{
	fprintf(stderr, "logged_getattr(\"%s\", %p)\n", path, stbuf);
	int res;

	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_access(const char *path, int mask)
{
	fprintf(stderr, "logged_access(\"%s\", %d)\n", path, mask);
	int res;

	res = access(path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_readlink(const char *path, char *buf, size_t size)
{
	fprintf(stderr, "logged_readlink(\"%s\", %p, %zd)\n", path, buf, size);
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int logged_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	fprintf(stderr, "logged_readdir(\"%s\", %p, ..., %ld, %p)\n", path, buf, offset, fi);
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

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

static int logged_mknod(const char *path, mode_t mode, dev_t rdev)
{
	fprintf(stderr, "logged_mknod(\"%s\", %d, ...)\n", path, mode);
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

static int logged_mkdir(const char *path, mode_t mode)
{
	fprintf(stderr, "logged_mkdir(\"%s\", %d)\n", path, mode);
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_unlink(const char *path)
{
	fprintf(stderr, "logged_unlink(\"%s\")\n", path);
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_rmdir(const char *path)
{
	fprintf(stderr, "logged_rmdir(\"%s\")\n", path);
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_symlink(const char *from, const char *to)
{
	fprintf(stderr, "logged_symlink(\"%s\", \"%s\")\n", from, to);
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_rename(const char *from, const char *to)
{
	fprintf(stderr, "logged_rename(\"%s\", \"%s\")\n", from, to);
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_link(const char *from, const char *to)
{
	fprintf(stderr, "logged_link(\"%s\", \"%s\")\n", from, to);
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_chmod(const char *path, mode_t mode)
{
	fprintf(stderr, "logged_chmod(\"%s\", %d)\n", path, mode);
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_chown(const char *path, uid_t uid, gid_t gid)
{
	fprintf(stderr, "logged_chown(\"%s\", %d, %d)\n", path, uid, gid);
	int res;

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_truncate(const char *path, off_t size)
{
	fprintf(stderr, "logged_truncate(\"%s\", %ld)\n", path, size);
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_UTIMENSAT
static int logged_utimens(const char *path, const struct timespec ts[2])
{
	int res;

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

static int logged_open(const char *path, struct fuse_file_info *fi)
{
	fprintf(stderr, "logged_open(\"%s\", %p)\n", path, fi);
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int logged_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	fprintf(stderr, "logged_read(\"%s\", %p, %zd, %ld, %p)\n", path, buf, size, offset, fi);
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

static int logged_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int logged_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int logged_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int logged_fsync(const char *path, int isdatasync,
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
static int logged_fallocate(const char *path, int mode,
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
static int logged_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int logged_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int logged_listxattr(const char *path, char *list, size_t size)
{
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int logged_removexattr(const char *path, const char *name)
{
	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations logged_oper = {
	.getattr	= logged_getattr,
	.access		= logged_access,
	.readlink	= logged_readlink,
	.readdir	= logged_readdir,
	.mknod		= logged_mknod,
	.mkdir		= logged_mkdir,
	.symlink	= logged_symlink,
	.unlink		= logged_unlink,
	.rmdir		= logged_rmdir,
	.rename		= logged_rename,
	.link		= logged_link,
	.chmod		= logged_chmod,
	.chown		= logged_chown,
	.truncate	= logged_truncate,
#ifdef HAVE_UTIMENSAT
	.utimens	= logged_utimens,
#endif
	.open		= logged_open,
	.read		= logged_read,
	.write		= logged_write,
	.statfs		= logged_statfs,
	.release	= logged_release,
	.fsync		= logged_fsync,
#ifdef HAVE_POSIX_FALLOCATE
	.fallocate	= logged_fallocate,
#endif
#ifdef HAVE_SETXATTR
	.setxattr	= logged_setxattr,
	.getxattr	= logged_getxattr,
	.listxattr	= logged_listxattr,
	.removexattr	= logged_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &logged_oper, NULL);
}

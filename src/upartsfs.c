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


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi)
{
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
    /* Just a stub.     This method is optional and can safely be left
       unimplemented */

    (void) path;
    (void) fi;
    return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
             struct fuse_file_info *fi)
{
    /* Just a stub.     This method is optional and can safely be left
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
    .getattr        = xmp_getattr,
    .access         = xmp_access,
    .readlink       = xmp_readlink,
    .readdir        = xmp_readdir,
    .mknod          = uparts_mknod,
    .mkdir          = uparts_mkdir,
    .symlink        = uparts_symlink,
    .unlink         = uparts_unlink,
    .rmdir          = uparts_rmdir,
    .rename         = uparts_rename,
    .link           = uparts_link,
    .chmod          = uparts_chmod,
    .chown          = uparts_chown,
    .truncate       = uparts_truncate,
#ifdef HAVE_UTIMENSAT
    .utimens        = xmp_utimens,
#endif
    .open           = xmp_open,
    .read           = xmp_read,
    .write          = uparts_write,
    .statfs         = xmp_statfs,
    .release        = xmp_release,
    .fsync          = xmp_fsync,
#ifdef HAVE_POSIX_FALLOCATE
    .fallocate      = xmp_fallocate,
#endif
#ifdef HAVE_SETXATTR
    .setxattr       = xmp_setxattr,
    .getxattr       = xmp_getxattr,
    .listxattr      = xmp_listxattr,
    .removexattr    = xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &upartsfs_oper, NULL);
}

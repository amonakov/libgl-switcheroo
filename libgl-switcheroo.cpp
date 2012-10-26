#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>
#include <cstring>
#include <cstdio>
#include <cassert>

#define LIBNAME "/libGL.so.1"

static int sw_getattr(const char *path, struct stat *stbuf)
{
  memset(stbuf, 0, sizeof(struct stat));

  if (!strcmp(path, "/lib")
#ifdef LIB64PATH
      || !strcmp(path, LIB64PATH)
#endif
#ifdef LIB32PATH
      || !strcmp(path, LIB32PATH)
#endif
      )
  {
    stbuf->st_mode = S_IFDIR | 0500;
    return 0;
  }
  if (!strcmp(path + strlen(path) - strlen(LIBNAME), LIBNAME))
  {
    stbuf->st_mode = S_IFLNK | 0777;
    stbuf->st_size = strlen(LIBPATH) + strlen(ALTPATH) + strlen(path);
    return 0;
  }
  return -ENOENT;
}

static bool need_switch()
{
  return true;
}

static int sw_readlink(const char *path, char *buf, size_t bufsize)
{
  assert(strlen(path) >= strlen(LIBNAME));
  snprintf(buf, bufsize, LIBPATH "%.*s%s", strlen(path) - strlen(LIBNAME), path,
           need_switch() ? ALTPATH LIBNAME : LIBNAME);
  return 0;
}

static struct fuse_operations ops = {
  .getattr  = sw_getattr,
  .readlink = sw_readlink
};

int main(int argc, char *argv[])
{
  return fuse_main(argc, argv, &ops, NULL);
}

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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

static pthread_mutex_t mutex;

static struct sockaddr_un addr;

static int need_switch(pid_t p)
{
  int do_switch = 0;
  pthread_mutex_lock(&mutex);
  int sock = socket(PF_UNIX, SOCK_STREAM, 0);
  if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("connect");
    goto unlock;
  }
  if (send(sock, &p, sizeof(p), 0) != sizeof(p))
  {
    perror("send");
    goto unlock;
  }
  if (recv(sock, &p, sizeof(p), 0) != sizeof(p))
  {
    perror("send");
    goto unlock;
  }
  do_switch = p;
unlock:
  pthread_mutex_unlock(&mutex);
  return do_switch;
}

static int sw_readlink(const char *path, char *buf, size_t bufsize)
{
  assert(strlen(path) >= strlen(LIBNAME));
  snprintf(buf, bufsize, LIBPATH "%.*s%s", (int)(strlen(path) - strlen(LIBNAME)), path,
           need_switch(fuse_get_context()->pid) ? ALTPATH LIBNAME : LIBNAME);
  return 0;
}

static struct fuse_operations ops = {
  .getattr  = sw_getattr,
  .readlink = sw_readlink
};

int main(int argc, char *argv[])
{
  const char *runtimedir;
  pthread_mutex_init(&mutex, NULL);
  addr.sun_family = AF_UNIX;
  if ((runtimedir = getenv("XDG_RUNTIME_DIR")))
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/libgl-switcheroo/socket", runtimedir);
  else
    snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/libgl-switcheroo-%s/socket", getenv("USER"));
  return fuse_main(argc, argv, &ops, NULL);
}

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <map>
#include <string>

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

static bool ask_user(const char procname[], bool &remember)
{
  char xdlgbuf[256];
  snprintf(xdlgbuf, 256, "Xdialog --stdout --check \"Remember my choice\" --no-tags --no-cancel "
           "--radiolist \"Use OpenGL offloading for %s?\" 0x0 0 d \"Yes, use discrete GPU\" on "
	   "i \"No, use integrated GPU\" off" , procname);
  FILE *f = popen(xdlgbuf, "r");
  char tag, check;
  int n = fscanf(f, "%c\n%c", &tag, &check);
  int r = pclose(f);
  if (n != 2 || r)
    return false;
  remember = check == 'c';
  return tag == 'd';
}

static std::map<std::string, bool> memos;

static bool lookup_memo(const char procname[], bool &choice)
{
  std::map<std::string, bool>::iterator i = memos.find(std::string(procname));
  if (i != memos.end())
    choice = i->second;
  return i != memos.end();
}

static void add_memo(const char procname[], bool choice)
{
  memos[std::string(procname)] = choice;
}

static bool need_switch()
{
  pid_t pid = fuse_get_context()->pid;
  char pathbuf[32];
  snprintf(pathbuf, 32, "/proc/%d/status", pid);
  int fd = open(pathbuf, O_RDONLY);
  char namebuf[32];
  read(fd, namebuf, 32);
  close(fd);
  *strchr(namebuf, '\n') = 0;
  char *procname = namebuf + 6;
  bool switch_yes;
  if (lookup_memo(procname, switch_yes))
    return switch_yes;
  bool remember = false;
  switch_yes = ask_user(procname, remember);
  if (remember)
    add_memo(procname, switch_yes);
  return switch_yes;
}

static int sw_readlink(const char *path, char *buf, size_t bufsize)
{
  assert(strlen(path) >= strlen(LIBNAME));
  snprintf(buf, bufsize, LIBPATH "%.*s%s", (int)(strlen(path) - strlen(LIBNAME)), path,
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

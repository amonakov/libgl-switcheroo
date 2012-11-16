#define _GNU_SOURCE
#include <link.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#define _stringify(S) #S
#define  stringify(S) _stringify(S)

static int need_switch()
{
  int sock, do_switch = 0;
  if ((sock = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
    return 0;
  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/libgl-switcheroo-%s/socket", getenv("USER"));
  if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    goto cleanup;
  int p = getpid();
  if (send(sock, &p, sizeof(p), 0) != sizeof(p))
    goto cleanup;
  if (recv(sock, &p, sizeof(p), 0) != sizeof(p))
    goto cleanup;
  do_switch = p;

cleanup:
  close(sock);
  return do_switch;
}

char *la_objsearch(const char *name, uintptr_t *cookie, unsigned flag)
{
  if (flag != LA_SER_ORIG || strcmp(name, "libGL.so.1"))
    return (char *) name; // Not the droid we're looking for
  return need_switch() ? stringify(ALTPATH) : (char *)name;
}

unsigned la_version(unsigned v)
{
  return v;
}

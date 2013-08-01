#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>

#include <map>
#include <string>

#include <gtk/gtk.h>

#define die(msg, ...) \
({fprintf(stderr, "gtkglswitch: fatal: " msg "\n", ##__VA_ARGS__); exit(1);})

static bool ask_user(const char procname[], bool &remember)
{
  GtkWidget *dialog = gtk_message_dialog_new (NULL,
      GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_QUESTION,
      GTK_BUTTONS_OK,
      "Use OpenGL offloading for %s?", procname);
  GtkWidget *content = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG(dialog));

  GtkWidget *radioy = gtk_radio_button_new_with_label(NULL, "Yes, use the discrete GPU");
  gtk_container_add(GTK_CONTAINER(content), radioy);
  GtkWidget *radion = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radioy), "No, use integrated GPU");
  gtk_container_add(GTK_CONTAINER(content), radion);

  GtkWidget *check = gtk_check_button_new_with_label("Remember my choice");
  gtk_container_add(GTK_CONTAINER(content), check);
  gtk_widget_show_all(dialog);
  gtk_dialog_run(GTK_DIALOG (dialog));
  gboolean yes = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radioy));
  remember = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check));
  gtk_widget_destroy(dialog);
  return yes;
}

static char *conffile;

static std::map<std::string, bool> read_memos()
{
  const char *configdir, *homedir;
  int r;
  if ((configdir = getenv("XDG_CONFIG_HOME")))
    r = asprintf(&conffile, "%s/libgl_switcheroo.conf", configdir);
  else if ((homedir = getenv("HOME")))
    r = asprintf(&conffile, "%s/.config/libgl_switcheroo.conf", homedir);
  else
    die("XDG_CONFIG_HOME and HOME are not set");
  if (r < 0)
    die("asprintf failed");
  FILE *f = fopen(conffile, "r");
  std::map<std::string, bool> memos;
  if (!f)
    return memos;
  char c, procname[17];
  while (fscanf(f, "%c%16[^\n]\n", &c, procname) == 2)
    memos[std::string(procname)] = c == '+';
  fclose(f);
  return memos;
}

static std::map<std::string, bool> memos(read_memos());

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
  FILE *f = fopen(conffile, "a");
  if (!f)
    die("failed to open %s for writing", conffile);
  fprintf(f, "%c%s\n", choice ? '+' : '-', procname);
  fclose(f);
}

static enum
{
  SWITCH_DEFAULT_ASK, SWITCH_DEFAULT_NO, SWITCH_DEFAULT_YES
} switch_default;

static bool need_switch(pid_t pid)
{
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
  if (switch_default != SWITCH_DEFAULT_ASK)
    return switch_default == SWITCH_DEFAULT_YES;
  bool remember = false;
  switch_yes = ask_user(procname, remember);
  if (remember)
    add_memo(procname, switch_yes);
  return switch_yes;
}

static void gdk_input_cb(void *data, int sock, GdkInputCondition cond)
{
  int fd = accept(sock, 0, 0);
  int pid;
  recv(fd, &pid, sizeof(pid), 0);
  pid = need_switch(pid);
  send(fd, &pid, sizeof(pid), 0);
  close(fd);
}

int main(int argc, char *argv[])
{
  char *opt_default = NULL;
  GError *error = NULL;
  static GOptionEntry entries [] = {
         {"default", 'D', 0, G_OPTION_ARG_STRING, &opt_default, "assume default answer", ""},
	 {NULL} 
  };

  if (!gtk_init_with_args(&argc, &argv, NULL, entries, NULL, &error))
    die("GTK initialization failed: %s", error->message);
  if (opt_default) {
    if (!strcmp(opt_default, "yes"))
      switch_default = SWITCH_DEFAULT_YES;
    else if (!strcmp(opt_default, "no"))
      switch_default = SWITCH_DEFAULT_NO;
    else
      die("invalid default answer: %s", opt_default);
  }

  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  snprintf(addr.sun_path, sizeof(addr.sun_path), "/tmp/libgl-switcheroo-%s/socket", getenv("USER"));
  unlink(addr.sun_path);
  int sock = socket(PF_UNIX, SOCK_STREAM, 0);
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    die("failed to bind socket %s: %s", addr.sun_path, strerror(errno));
  listen(sock, 16);
  gdk_input_add(sock, GDK_INPUT_READ, gdk_input_cb, NULL);
  gtk_main();

  return 0;
}

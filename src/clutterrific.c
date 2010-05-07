#include <errno.h>

#include <clutter/clutter.h>
#include <clutter/x11/clutter-x11.h>



void
clutterrific_init (void)
{
  const gchar *id = g_getenv ("XSCREENSAVER_WINDOW");
  gchar       *end;

  if (id != NULL)
  {
    Window window = (Window) g_ascii_strtoull (id, &end, 0);

    if (window && end != NULL && (!*end || *end == ' ') && (window < G_MAXULONG && errno != ERANGE))
      clutter_x11_set_stage_foreign (CLUTTER_STAGE (clutter_stage_get_default ()), window);
  }
}

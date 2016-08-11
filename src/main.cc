#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "xmj3ts.hh"
#include "gmjts.hh"
#include "kmjts.hh"
#include "kdets.hh"
#include "board.hh"
#include "panel.hh"
#include "game.hh"
#include "hint.hh"
#include "alarm.hh"
#include "button.hh"
#include "matches.hh"
#include "solution.hh"
#include "counter.hh"
#include <lcdf/permstr.hh>
#include <lcdf/string.hh>
#include <X11/Xutil.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cctype>
#include <dirent.h>
#include <cerrno>
#include <ctime>
#include <lcdf/clp.h>

const char *program_name;
bool solvable_boards = true;
Moment last_new_board;

static Gif_XContext *gifx;
static Gif_Stream *gifbuttons;
static MatchCount *matches;
extern Gif_Record buttons_gif;

#define DISPLAY_OPT		300
#define HELP_OPT		301
#define VERSION_OPT		302
#define NAME_OPT		303
#define GEOMETRY_OPT		304
#define SOLVABLE_OPT		305
#define ANY_BOARD_OPT		306
#define TILESET_OPT		307
#define CONFIG_DIR_OPT		308
#define BOARD_NUMBER_OPT	309
#define LIST_OPT		310
#define BACKGROUND_OPT		311
#define LAYOUT_OPT		312
#define OBSOLETE_OPT		313
static Clp_Option options[] = {
  { "any-boards", 'a', ANY_BOARD_OPT, 0, Clp_Negate },
  { "background", 0, BACKGROUND_OPT, Clp_ArgStringNotOption, 0 },
  { "bg", 0, BACKGROUND_OPT, Clp_ArgStringNotOption, 0 },
  { "board", 'b', OBSOLETE_OPT, Clp_ArgStringNotOption, 0 },
  { "config-dir", 'B', CONFIG_DIR_OPT, Clp_ArgStringNotOption, 0 },
  { "display", 'd', DISPLAY_OPT, Clp_ArgStringNotOption, 0 },
  { "geometry", 'g', GEOMETRY_OPT, Clp_ArgString, 0 },
  { "help", 0, HELP_OPT, 0, 0 },
  { "layout", 'l', LAYOUT_OPT, Clp_ArgStringNotOption, 0 },
  { "list", 0, LIST_OPT, 0, 0 },
  { "name", 0, NAME_OPT, Clp_ArgString, 0 },
  { "number", 'n', BOARD_NUMBER_OPT, Clp_ArgInt, Clp_Negate },
  { "solvable-boards", 's', SOLVABLE_OPT, 0, Clp_Negate },
  { "tileset", 't', TILESET_OPT, Clp_ArgStringNotOption, 0 },
  { "version", 0, VERSION_OPT, 0, 0 },
};


void
fatal_error(const char *message, ...)
{
  va_list val;
  va_start(val, message);
  fprintf(stderr, "%s: ", program_name);
  vfprintf(stderr, message, val);
  fputc('\n', stderr);
  exit(1);
}

void
config_error(const char *kind)
{
  fatal_error("%s\n\
  (This probably means I was installed incorrectly. Wizards can try the\n\
  `-B CONFIGDIR' option.)", kind);
}

void
error(const char *message, ...)
{
  va_list val;
  va_start(val, message);
  fprintf(stderr, "%s: ", program_name);
  vfprintf(stderr, message, val);
  fputc('\n', stderr);
}

void
warning(const char *message, ...)
{
  va_list val;
  va_start(val, message);
  fprintf(stderr, "%s: warning: ", program_name);
  vfprintf(stderr, message, val);
  fputc('\n', stderr);
}

void
short_usage()
{
  fprintf(stderr, "Usage: %s [OPTION]... [LAYOUT]\n\
Try `%s --help' for more information.\n",
	  program_name, program_name);
  exit(1);
}

void
usage()
{
  printf("\
`Xmahjongg' is a colorful X version of the venerable computer solitaire\n\
Mah Jongg game.\n\
\n\
Usage: %s [OPTION]... [LAYOUT]\n\
\n\
Options are:\n\
  -s, --solvable-boards          Use only solvable boards (default).\n\
  -a, --any-boards               Allow any board (opposite of -s).\n\
  -n, --number N                 Start with board number N.\n\
  -l, --layout LAYOUT            Use the specified layout.\n\
  -t, --tileset TILESET          Use the specified tileset.\n\
  --bg, --background IMAGE       Use the specified image for the background.\n\
      --list                     List known layouts, tilesets & backgrounds.\n\
  -d, --display DISPLAY          Set display to DISPLAY.\n\
      --name NAME                Set application resource name to NAME.\n\
  -g, --geometry GEOM            Set window geometry.\n\
  -B, --config-dir DIR           Look for shared configuration data in DIR.\n\
      --help                     Print this message and exit.\n\
      --version                  Print version number and exit.\n\
\n\
Report bugs to <eddietwo@lcs.mit.edu>.\n", program_name);
}

static int
permstring_compare(const void *v1, const void *v2)
{
  const PermString *p1 = (const PermString *)v1;
  const PermString *p2 = (const PermString *)v2;
  return strcmp(p1->c_str(), p2->c_str());
}

static void
print_config_dir(const char *format, const char *dir_name,
		 const char *kill_suffix, DIR *dir)
{
  Vector<PermString> entries;
  int max_length = 0;
  int kill_suffix_len = (kill_suffix ? strlen(kill_suffix) : 0);
  
  for (struct dirent *dp = readdir(dir); dp; dp = readdir(dir))
    if (dp->d_name[0] != '.') {
      int len = strlen(dp->d_name); // d_namlen isn't portable.
      if (kill_suffix_len && kill_suffix_len < len
	  && strcmp(kill_suffix, dp->d_name + len - kill_suffix_len) == 0)
	len -= kill_suffix_len;
      
      entries.push_back(PermString(dp->d_name, len));
      if (len > max_length) max_length = len;
    }

  qsort(&entries[0], entries.size(), sizeof(PermString), permstring_compare);

  int width = ((max_length + 5) / 4) * 4;
  int ncolumns = 75 / width;
  if (ncolumns == 0) ncolumns = 1;
  if (ncolumns > entries.size()) ncolumns = entries.size();

  int height;
  while (1) {
    height = ((entries.size() - 1) / ncolumns) + 1;
    int size = ncolumns * height;
    if (size < height + entries.size()) break;
    ncolumns--;
  }

  printf(format, dir_name);
  printf(":\n");
  for (int j = 0; j < height; j++) {
    printf("    ");
    for (int i = 0; i < ncolumns - 1; i++)
      printf("%-*s", width, entries[i*height + j].c_str());
    if ((ncolumns-1)*height + j < entries.size())
      printf("%s\n", entries[(ncolumns-1)*height + j].c_str());
    else
      printf("\n");
  }
}

void
print_config_list(const char *config_dir)
{
  int len = strlen(config_dir) + 13;
  char *buf = new char[len];
  
  sprintf(buf, "%s/layouts", config_dir);
  DIR *dir = opendir(buf);
  if (dir == 0) config_error("can't find any layouts!");
  print_config_dir("Layouts in `%s/layouts'", config_dir,
		   0, dir);
  closedir(dir);
  
  sprintf(buf, "%s/tiles", config_dir);
  dir = opendir(buf);
  if (dir == 0) config_error("can't find any tilesets!");
  print_config_dir("Tilesets in `%s/tiles'", config_dir,
		   ".gif", dir);
  closedir(dir);

  sprintf(buf, "%s/backgrounds", config_dir);
  dir = opendir(buf);
  if (dir == 0) config_error("can't find any background images!");
  print_config_dir("Background images in `%s/backgrounds'",
		   config_dir, ".gif", dir);
  closedir(dir);

  delete[] buf;
}


Button *
new_button(Panel *panel, char *name)
{
  char buf[100];
  Button *but = new Button(panel);
  but->set_normal(gifbuttons, name);
  sprintf(buf, "%s-lit", name);
  but->set_lit(gifbuttons, buf);
  return but;
}


static void
make_panel_images(Panel *p)
{
  gifbuttons = Gif_ReadRecord(&buttons_gif);

  matches = new MatchCount(p, gifbuttons, "rock");
  p->set_match_count(matches);
  
  p->new_but = new_button(p, "new");
  p->undo_but = new_button(p, "undo");
  p->quit_but = new_button(p, "quit");
  p->hint_but = new_button(p, "hint");
  p->clean_but = new_button(p, "clean");
}


void
panel_loop(Game *game, Panel *panel)
{
  Display *d = panel->display();
  XEvent event;
  
  while (1) {
    while (XPending(d)) {
      XNextEvent(d, &event);
      panel->handle(game, &event);
    }
    panel->redraw();
    Alarm::x_wait(d);
  }
}



int
parse_geometry(const char *const_g, XSizeHints *sh, int screen_width,
	       int screen_height)
{
  char *g = (char *)const_g;
  sh->flags = 0;
  
  if (isdigit(*g)) {
    sh->flags |= USSize;
    sh->width = strtol(g, &g, 10);
    if (g[0] == 'x' && isdigit(g[1]))
      sh->height = strtol(g + 1, &g, 10);
    else
      goto error;
  } else if (!*g)
    goto error;
  
  if (*g == '+' || *g == '-') {
    int x_minus, y_minus;
    sh->flags |= USPosition | PWinGravity;
    x_minus = *g == '-';
    sh->x = strtol(g + 1, &g, 10);
    if (x_minus) sh->x = screen_width - sh->x - sh->width;
    
    y_minus = *g == '-';
    if (*g == '-' || *g == '+')
      sh->y = strtol(g + 1, &g, 10);
    else
      goto error;
    if (y_minus) sh->y = screen_height - sh->y - sh->height;
    
    if (x_minus)
      sh->win_gravity = y_minus ? SouthEastGravity : NorthEastGravity;
    else
      sh->win_gravity = y_minus ? SouthWestGravity : NorthWestGravity;
    
  } else if (*g)
    goto error;
  
  return 1;
  
 error:
  warning("bad geometry specification");
  sh->flags = 0;
  return 0;
}


static Visual *visual;
static int depth;
static Colormap colormap;

static void
choose_visual(Display *display, int screen_number)
{
  int nv;
  unsigned int default_visualid =
    DefaultVisual(display, screen_number)->visualid;
  Window root_window = RootWindow(display, screen_number);
  XVisualInfo visi_template;
  visi_template.screen = screen_number;
  
  XVisualInfo *v =
    XGetVisualInfo(display, VisualScreenMask, &visi_template, &nv);
  XVisualInfo *best_v = 0;
  for (int i = 0; i < nv && !best_v; i++)
    if (v[i].visualid == default_visualid)
      best_v = &v[i];
  
  if (!best_v) {
    visual = DefaultVisual(display, screen_number);
    depth = DefaultDepth(display, screen_number);
    colormap = DefaultColormap(display, screen_number);
  } else {
  
    /* Which visual to choose? This isn't exactly a simple decision, since
       we want to avoid colormap flashing while choosing a nice visual. So
       here's the algorithm: Prefer the default visual, or take a TrueColor
       visual with strictly greater depth. */
    for (int i = 0; i < nv; i++)
      if (v[i].depth > best_v->depth && v[i].c_class == TrueColor)
	best_v = &v[i];
    
    visual = best_v->visual;
    depth = best_v->depth;
    if (best_v->visualid != default_visualid)
      colormap = XCreateColormap(display, root_window, visual, AllocNone);
    else
      colormap = DefaultColormap(display, screen_number);
    
  }
  
  if (v) XFree(v);

  gifx = Gif_NewXContextFromVisual(display, screen_number, visual, depth,
				   colormap);
}


static Window
create_window(Display *display, int screen_number, Pixmap background)
{
  XSetWindowAttributes x_set_attr;
  unsigned long x_set_attr_mask;
  x_set_attr.colormap = colormap;
  x_set_attr.backing_store = NotUseful;
  x_set_attr.save_under = False;
  x_set_attr.border_pixel = 0;
  x_set_attr.background_pixel = 0;
  x_set_attr.background_pixmap = background;
  x_set_attr_mask = CWColormap | CWBorderPixel | CWBackingStore
    | CWSaveUnder | (background ? CWBackPixmap : CWBackPixel);
  
  /* Now make the window */
  return XCreateWindow
    (display, RootWindow(display, screen_number),
     0, 0, 100, 100, 0,
     depth, InputOutput, visual,
     x_set_attr_mask, &x_set_attr);
}


static Tileset *
load_tileset(const char *tileset_name, const char *config_dir)
{
  int len = strlen(tileset_name) + strlen(config_dir) + 15;
  char *buf = new char[len];
  Gif_Stream *gfs;
  
  sprintf(buf, "%s/tiles/%s.gif", config_dir, tileset_name);
  FILE *f = fopen(buf, "rb");
  if (!f)
    f = fopen(tileset_name, "rb");
  if (!f) {
    gfs = 0;
    error("bad tileset `%s': %s", tileset_name, strerror(errno));
  } else {
    gfs = Gif_FullReadFile(f, GIF_READ_COMPRESSED, 0, 0);
    fclose(f);
  }
  
  Tileset *tileset = 0;

  // Xmahjongg tileset?
  if (gfs && !tileset && Gif_ImageCount(gfs) > 1)
    tileset = new Xmj3Tileset(gfs, gifx);

  // Otherwise, check dimensions.
  int width = 0, height = 0;
  if (gfs && !tileset) {
    Gif_Image *gfi = Gif_GetImage(gfs, 0);
    width = (gfi ? Gif_ImageWidth(gfi) : 0);
    height = (gfi ? Gif_ImageHeight(gfi) : 0);
  }

  // Gnome tileset?
  if (gfs && !tileset && width > 2*height)
    tileset = new GnomeMjTileset(gfs, gifx);

  // KDE tileset?
  if (gfs && !tileset && width == 360 && height == 280) {
    tileset = new KDETileset(gfs, gifx);
    if (!tileset->ok()) {
      delete tileset;
      tileset = 0;
    }
  }

  // Kyodai tileset?
  if (gfs && !tileset)
    tileset = new KyodaiTileset(gfs, gifx);

  // Delete tileset if bad
  if (tileset && !tileset->ok()) {
    delete tileset;
    tileset = 0;
  }
  
  delete[] buf;
  if (gfs && !gfs->refcount)
    Gif_DeleteStream(gfs);
  
  // What if that's not a valid tileset?
  if (!tileset) {
    if (strcmp(tileset_name, "thick") == 0)
      config_error("can't load `thick' tileset!");
    else if (f != 0)
      error("tileset `%s' is invalid", tileset_name);
    error("using default tileset `thick'");
    return load_tileset("thick", config_dir);
  }
  
  return tileset;
}


static Pixmap
load_background(const char *background_name, const char *config_dir,
		Gif_XContext *gfx)
{
  int len = strlen(background_name) + strlen(config_dir) + 21;
  char *buf = new char[len];
  Gif_Stream *gfs;
  
  sprintf(buf, "%s/backgrounds/%s.gif", config_dir, background_name);
  FILE *normal_f = fopen(buf, "rb");
  if (!normal_f)
    normal_f = fopen(background_name, "rb");
  if (!normal_f) {
    gfs = 0;
    error("bad background `%s': %s", background_name, strerror(errno));
  } else {
    gfs = Gif_ReadFile(normal_f);
    fclose(normal_f);
  }
  
  delete[] buf;
  
  // What if that's not a valid background?
  if (gfs && Gif_ImageCount(gfs) == 0) {
    Gif_DeleteStream(gfs);
    gfs = 0;
  }
  if (!gfs) {
    if (strcmp(background_name, "default") == 0)
      config_error("can't load default background!");
    else if (normal_f != 0)
      error("background `%s' is invalid", background_name);
    error("using default background");
    return load_background("default", config_dir, gfx);
  }
  
  Pixmap background = Gif_XImage(gfx, gfs, 0);
  Gif_DeleteStream(gfs);
  return background;
}


int
main(int argc, char *argv[])
{
  // First, parse command-line options.
  const char *display_name = 0;
  const char *layout_name = 0;
  const char *tileset_name = "thick";
  const char *background_name = "default";
  const char *x_name = 0;
  const char *geometry = 0;
  const char *config_dir = PKGDATADIR;
  bool board_number_given = false;
  uint32_t board_number = 0;
  bool do_config_list = false;
  
  Clp_Parser *clp =
    Clp_NewParser(argc, (const char * const *)argv, sizeof(options) / sizeof(options[0]), options);
  
  program_name = Clp_ProgramName(clp);
  
  while (1) {
    int opt = Clp_Next(clp);
    switch (opt) {

     case SOLVABLE_OPT:
      solvable_boards = clp->negated ? 0 : 1;
      break;

     case ANY_BOARD_OPT:
      solvable_boards = clp->negated ? 1 : 0;
      break;
      
     case LAYOUT_OPT:
     case Clp_NotOption:
      if (layout_name) fatal_error("only one layout name allowed");
      layout_name = clp->arg;
      break;
      
     case BOARD_NUMBER_OPT:
      board_number = clp->val.i;
      board_number_given = clp->negated ? 0 : 1;
      break;
      
     case TILESET_OPT:
      tileset_name = clp->arg;
      break;
      
     case BACKGROUND_OPT:
      background_name = clp->arg;
      break;
      
     case DISPLAY_OPT:
      if (display_name) fatal_error("only one --display allowed");
      display_name = clp->arg;
      break;
      
     case NAME_OPT:
      if (x_name) fatal_error("only one --name allowed");
      x_name = clp->arg;
      break;
      
     case GEOMETRY_OPT:
      if (geometry) fatal_error("only one --geometry allowed");
      geometry = clp->arg;
      break;
      
     case CONFIG_DIR_OPT:
      config_dir = clp->arg;
      break;
      
     case VERSION_OPT:
      printf("LCDF Xmahjongg %s\n", VERSION);
      printf("\
Copyright (C) 1993-2005 Eddie Kohler and others\n\
This is free software; see the source for copying conditions.\n\
There is NO warranty, not even for merchantability or fitness for a\n\
particular purpose.\n");
      exit(0);
      break;
      
     case HELP_OPT:
      usage();
      exit(0);
      break;
      
     case LIST_OPT:
      do_config_list = true;
      break;
      
     case OBSOLETE_OPT:
      fatal_error("The `--board' option is obsolete. Use `--layout' instead.");
      break;
      
     case Clp_Done:
      goto done;
      
     case Clp_BadOption:
      short_usage();
      break;
      
     default:
      break;
      
    }
  }
  
 done:
  
  if (do_config_list) {
    print_config_list(config_dir);
    exit(0);
  }
  
  // First, choose the visual and create the tile set.
  zrand_seed(0x97891);
  
  Display *display = XOpenDisplay(display_name);
  if (!display) fatal_error("could not open display");
  int screen_number = DefaultScreen(display);
  choose_visual(display, screen_number);
  
  Tileset *tileset = load_tileset(tileset_name, config_dir);
  Pixmap background = load_background(background_name, config_dir, gifx);
  
  Game *game = new Game(tileset);
  
  // Make the window.
  
  Window window = create_window(display, screen_number, background);
  if (window == None) fatal_error("could not create window");
  XSelectInput(display, window, ExposureMask | ButtonPressMask
	       | ButtonReleaseMask | KeyPressMask | KeyReleaseMask
	       | StructureNotifyMask | Button2MotionMask);
  
  Panel *panel = new Panel(display, window);
  make_panel_images(panel);
  matches->set_game(game);
  
  Board *board = new Board(panel, game, tileset);
  board->set_position(0, 65);
  
  panel->set_board(board);
  panel->set_background(background);
  panel->set_solution(new SolutionDisplay(game, board));
  
  FancyTileCounter *tile_counter = new FancyTileCounter(panel);
  game->add_hook(tile_counter);
  panel->set_tile_count(tile_counter);

  // Lay out game.
  
  if (!layout_name)
    game->layout_default();
  else {
    int ok = game->layout_file(layout_name);
    if (ok < 0) {
      int len = strlen(layout_name) + strlen(config_dir) + 10;
      char *buf = new char[len];
      sprintf(buf, "%s/layouts/%s", config_dir, layout_name);
      ok = game->layout_file(buf);
      delete[] buf;
    }
    if (ok < 0)
      fatal_error("layout %s: %s", layout_name, strerror(errno));
    else if (ok == 0)
      fatal_error("layout %s corrupted", layout_name);
  }

  // now we know how big and wide the game is; set up WM hints
  {
    XSizeHints *size_hint = XAllocSizeHints();
    int wid, hgt;
    board->tile_layout_size(&wid, &hgt);
    
    size_hint->flags = PSize;
    size_hint->x = size_hint->y = 0;
    size_hint->width = wid + 38;
    size_hint->height = hgt + 38 + 56;
    
    if (geometry)
      parse_geometry(geometry, size_hint, DisplayWidth(display, screen_number),
		     DisplayHeight(display, screen_number));

    String window_name = (x_name ? x_name : program_name);
    if (layout_name)
	window_name += String(" - ") + layout_name;
    
    XClassHint class_hint;
    char *woog[2];
    XTextProperty window_name_prop, icon_name_prop;
    woog[0] = window_name.mutable_c_str();
    woog[1] = NULL;
    XStringListToTextProperty(woog, 1, &window_name_prop);
    XStringListToTextProperty(woog, 1, &icon_name_prop);
    class_hint.res_name = (char *)(x_name ? x_name : program_name);
    class_hint.res_class = "XMahjongg";

    XResizeWindow(display, window, size_hint->width, size_hint->height);
    XSetWMProperties(display, window, &window_name_prop, &icon_name_prop,
		     NULL, 0, size_hint, NULL, &class_hint);
    
    XFree(window_name_prop.value);
    XFree(icon_name_prop.value);

    board->set_size(size_hint->width, size_hint->height - board->y_pos());
    board->center_layout();
  }
  
  // start game
  if (board_number_given) {
    game->start_specific(board_number);
    solvable_boards = game->solution().size() != 0;
  } else
    game->start(getpid() * time(0), solvable_boards);
  last_new_board = Moment::now();
  
  XMapRaised(display, window);
  panel_loop(game, panel);
  exit(0);
}

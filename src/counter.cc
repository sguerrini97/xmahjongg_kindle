#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "counter.hh"

extern Gif_Record digits_gif;
static Pixmap digits[10];
static Pixmap masks[10];
static int digit_width, digit_height;

static void
setup_digits(Gif_XContext *gifx)
{
  if (digits[0]) return;
  Gif_Stream *gfs = Gif_ReadRecord(&digits_gif);
  assert(gfs->nimages == 10);
  digit_width = Gif_ImageWidth(Gif_GetImage(gfs, 0));
  digit_height = Gif_ImageHeight(Gif_GetImage(gfs, 0));
  for (int i = 0; i < 10; i++) {
    Gif_Image *gfi = Gif_GetImage(gfs, i);
    digits[i] = Gif_XImage(gifx, gfs, gfi);
    masks[i] = Gif_XMask(gifx, gfs, gfi);
  }
  Gif_DeleteStream(gfs);
}


FancyCounter::FancyCounter(SwWindow *parent, int num_digits)
  : SwWidget(parent), _digits(num_digits), _value(0)
{
  setup_digits(get_gif_x_context());
  set_size(digit_width * num_digits, digit_height);
}


void
FancyCounter::draw()
{
  clear_area(0, 0, width(), height());
  int v = _value;
  for (int d = _digits - 1; d >= 0; d--) {
    int digit = v % 10;
    draw_image(digits[digit], masks[digit], digit_width,
	       digit_height, d * digit_width, 0);
    v /= 10;
    if (v == 0) break;
  }
}



FancyTileCounter::FancyTileCounter(SwWindow *window)
  : FancyCounter(window, 3)
{
}

void
FancyTileCounter::start_hook(Game *g)
{
  set_value(g->nremaining());
}

void
FancyTileCounter::move_made_hook(Game *g)
{
  set_value(g->nremaining());
}

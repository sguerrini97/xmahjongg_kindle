#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "xmj3ts.hh"
#include "tile.hh"
#include "swgeneral.hh"
#include <cstring>

#define NPICTURES	(Tileset::IVORY_NPICTURES)

Xmj3Tileset::Xmj3Tileset(Gif_Stream *gfs, Gif_XContext *gifx)
  : Tileset("ivory"), _gfs(gfs),
    _gifx(gifx), _colormap(0), _image_error(ieNone)
{
  initialize_images();
  if (check()) {
    initialize();
    _gfs->refcount++;
  } else
    _gfs = 0;
}

Xmj3Tileset::~Xmj3Tileset()
{
  Gif_DeleteColormap(_colormap);
  
  Display *display = _gifx->display;
  if (_gfs && _images.size())
    for (int i = 0; i < Gif_ImageCount(_gfs); i++) {
      if (_images[i]) XFreePixmap(display, _images[i]);
      if (_masks[i]) XFreePixmap(display, _masks[i]);
    }
  
  Gif_DeleteStream(_gfs);
}


/*****
 * code to create & use the map of tile names to picture numbers
 **/

static int nsearch_pictures;
static const char **search_picture_names;
static int *search_picture_numbers;

static int
sort_picture_names(const void *v1, const void *v2)
{
  int i1 = *(int *)v1, i2 = *(int *)v2;
  return strcmp(search_picture_names[i1], search_picture_names[i2]);
}

#define RANGE_ALL	(NPICTURES + 7)

static const char *special_picture_names[] = {
  "season",	"flower",	"dragon",	"wind",
  "dot",	"bamboo",	"character",	"all"
};

static void
initialize_picture_names()
{
  if (search_picture_names) return;
  
  assert(Tileset::IVORY_NPICTURES == NPICTURES);
  int npictures = NPICTURES + 8;
  search_picture_names = new const char *[npictures];
  search_picture_numbers = new int[npictures];
  for (int i = 0; i < NPICTURES; i++) {
    search_picture_names[i] = Tileset::ivory_picture_name(i);
    search_picture_numbers[i] = i;
  }
  for (int i = 0; i < 8; i++) {
    int j = NPICTURES + i;
    search_picture_names[j] = special_picture_names[i];
    search_picture_numbers[j] = j;
  }
  
  nsearch_pictures = npictures;
  qsort(search_picture_numbers, npictures, sizeof(int), &sort_picture_names);
  
  const char **x = search_picture_names;
  search_picture_names = new const char *[npictures];
  for (int i = 0; i < npictures; i++)
    search_picture_names[i] = x[search_picture_numbers[i]];
  delete[] x;
}

static int
find_picture_name(const char *name)
{
  if (name == 0) return -1;
  int l = 0, r = nsearch_pictures - 1;
  while (l <= r) {
    int m = (l + r) / 2;
    int comparison = strcmp(name, search_picture_names[m]);
    if (comparison < 0)
      r = m - 1;
    else if (comparison > 0)
      l = m + 1;
    else
      return search_picture_numbers[m];
  }
  return -1;
}

/******/

void
Xmj3Tileset::map_one_image(const char *name_rest, int image_index,
			   ImageType which, Vector<short> &genericity)
{
  if (*name_rest == '-' || *name_rest == ' ' || *name_rest == '.')
    name_rest++;
  
  int range;
  if (*name_rest == 0)
    range = RANGE_ALL;
  else
    range = find_picture_name(name_rest);
  if (range < 0)
    return;
  
  int left, right;
  switch (range) {
   case NPICTURES: /* seasons  */	left = 0; right = 3; break;
   case NPICTURES+1: /* flowers */	left = 4; right = 7; break;
   case NPICTURES+2: /* dragons */	left = 8; right = 10; break;
   case NPICTURES+3: /* winds */	left = 11; right = 14; break;
   case NPICTURES+4: /* dots */		left = 15; right = 23; break;
   case NPICTURES+5: /* bamboos */	left = 24; right = 32; break;
   case NPICTURES+6: /* characters */	left = 33; right = 41; break;
   case RANGE_ALL:			left = 0; right = 41; break;
   default:				left = right = range; break;
  }
  
  Vector<short> *references = 0;
  if (which == itFace)
    references = &_face_ref;
  else if (which == itBase)
    references = &_base_ref;
  else if (which == itSelected)
    references = &_selected_ref;
  else if (which == itObscured)
    references = &_obscured_ref;
  
  short new_genericity = right - left;
  genericity[image_index] = new_genericity;
  
  for (int i = left; i <= right; i++) {
    short old_image = (*references)[i];
    if (old_image == -1 || genericity[old_image] > new_genericity)
      (*references)[i] = image_index;
  }
}

void
Xmj3Tileset::initialize_images()
{
  if (_gfs == 0) {
    _image_error = ieBadGif;
    return;
  }
  
  initialize_picture_names();
  _base_ref.assign(NPICTURES, -1);
  _selected_ref.assign(NPICTURES, -1);
  _obscured_ref.assign(NPICTURES, -1);
  _face_ref.assign(NPICTURES, -1);
  
  Vector<short> genericity(Gif_ImageCount(_gfs), NPICTURES + 1);
  
  for (int imagei = 0; imagei < Gif_ImageCount(_gfs); imagei++) {
    Gif_Image *gfi = Gif_GetImage(_gfs, imagei);
    const char *name = gfi->identifier;
    if (!name || name[0] == 0)
      ;
    else if (strncmp(name, "base", 4) == 0)
      map_one_image(name + 4, imagei, itBase, genericity);
    else if (strncmp(name, "selected", 8) == 0)
      map_one_image(name + 8, imagei, itSelected, genericity);
    else if (strncmp(name, "obscured", 8) == 0)
      map_one_image(name + 8, imagei, itObscured, genericity);
    else
      map_one_image(name, imagei, itFace, genericity);
  }
  
  _images.assign(Gif_ImageCount(_gfs), None);
  _masks.assign(Gif_ImageCount(_gfs), None);
  
  check_images();
}

void
Xmj3Tileset::check_images()
{
  _image_error = ieNone;
  
  // All four kinds of image must exist for every tile
  for (int i = 0; i < NPICTURES; i++)
    if (_base_ref[i] < 0 || _selected_ref[i] < 0 || _obscured_ref[i] < 0) {
      _image_error = ieNoBase;
      return;
    }
  
  // Check the sizes of all the images
  // Assume it's broken.
  _image_error = ieBadSize;
  Gif_Image *gfi = Gif_GetImage(_gfs, _obscured_ref[0]);
  int w = gfi->width;
  int h = gfi->height;
  for (int i = 0; i < NPICTURES; i++) {
    Gif_Image *gfi = Gif_GetImage(_gfs, _base_ref[i]);
    if (gfi->width != w || gfi->height != h)
      return;
    gfi = Gif_GetImage(_gfs, _selected_ref[i]);
    if (gfi->width != w || gfi->height != h)
      return;
    gfi = Gif_GetImage(_gfs, _obscured_ref[i]);
    if (gfi->width != w || gfi->height != h)
      return;
    gfi = Gif_GetImage(_gfs, _face_ref[i]);
    if (gfi->width > w || gfi->height > h)
      return;
  }
  
  // If we get here, all is hunky-dory!
  _image_error = ieNone;
}

bool
Xmj3Tileset::check() const
{
  // Stream must exist & have global colormap
  if (!_gfs->global || _image_error != ieNone)
    return false;
  
  // OK
  return true;
}

void
Xmj3Tileset::initialize()
{
  _colormap = Gif_CopyColormap(_gfs->global);
  
  int border = 4, shadow = 1;
  for (Gif_Extension *gfex = _gfs->extensions; gfex; gfex = gfex->next)
    if (gfex->application && strcmp(gfex->application, "xmahjongg") == 0) {
      sscanf((char *)gfex->data, "border=%u", &border);
      sscanf((char *)gfex->data, "shadow=%d", &shadow);
    }
  _xborder = _yborder = border;
  _shadow = shadow;
  
  Gif_Image *gfi = Gif_GetImage(_gfs, _base_ref[0]);
  _width = gfi->width - _xborder;
  _height = gfi->height - _yborder;
}

void
Xmj3Tileset::draw(SwDrawable *drawable, short x, short y, short base,
		  short face)
{
  if (base >= 0) {
    if (!_images[base]) {
      Gif_Image *gfi = Gif_GetImage(_gfs, base);
      _images[base] = Gif_XImageColormap(_gifx, _gfs, _colormap, gfi);
      _masks[base] = Gif_XMask(_gifx, _gfs, gfi);
    }
    drawable->draw_image(_images[base], _masks[base],
			 _width + _xborder, _height + _yborder, x, y);
  }
  
  if (face >= 0) {
    Gif_Image *facei = Gif_GetImage(_gfs, face);
    if (!_images[face]) {
      _images[face] = Gif_XImageColormap(_gifx, _gfs, _colormap, facei);
      _masks[face] = Gif_XMask(_gifx, _gfs, facei);
    }
    
    int dx = (_shadow & 1 ? _xborder : 0) + facei->left;
    int dy = (_shadow & 2 ? _yborder : 0) + facei->top;
    drawable->draw_image(_images[face], _masks[face],
			 facei->width, facei->height, x + dx, y + dy);
  }
}

void
Xmj3Tileset::draw_normal(const Tile *t, SwDrawable *drawable, short x, short y)
{
  int which = picture(t->number());
  assert(which >= 0 && which < NPICTURES);
  draw(drawable, x, y, _base_ref[which], _face_ref[which]);
}

void
Xmj3Tileset::draw_lit(const Tile *t, SwDrawable *drawable, short x, short y)
{
  int which = picture(t->number());
  assert(which >= 0 && which < NPICTURES);
  draw(drawable, x, y, _selected_ref[which], _face_ref[which]);
}

void
Xmj3Tileset::draw_obscured(const Tile *t, SwDrawable *drawable,
			   short x, short y)
{
  int which = picture(t->number());
  assert(which >= 0 && which < NPICTURES);
  draw(drawable, x, y, _obscured_ref[which], -1);
}

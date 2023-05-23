#include "display.h"

Display::Display(int width, int height)
    : gfxFont{}
    , textsize_x{1}
    , textsize_y{1}
    , wrap{false}
    , _width(width)
    , _height(height)
    , cursor_x{0}
    , cursor_y{0}
    , textcolor(1)
    , textbgcolor(0)
    , buffer{QSize(_width, _height), QImage::Format_Indexed8}
{
    buffer.setColor(0, 0xffeeeeee);
    buffer.setColor(1, 0xff000000);
    buffer.setColor(2, 0xffff0000);
    buffer.fill(0);
}

void Display::save()
{
    buffer.save("/tmp/screenshot.png");
}

void Display::charBounds(unsigned char c,
                         int16_t *x,
                         int16_t *y,
                         int16_t *minx,
                         int16_t *miny,
                         int16_t *maxx,
                         int16_t *maxy)
{
    if (gfxFont) {
        if (c == '\n') { // Newline?
            *x = 0;      // Reset x to zero, advance y by one line
            *y += textsize_y * (uint8_t) gfxFont->yAdvance;
        } else if (c != '\r') { // Not a carriage return; is normal char
            uint8_t first = gfxFont->first, last = gfxFont->last;
            if ((c >= first) && (c <= last)) { // Char present in this font?
                GFXglyph *glyph = &gfxFont->glyph[c - first];
                uint8_t gw = glyph->width, gh = glyph->height, xa = glyph->xAdvance;
                int8_t xo = glyph->xOffset, yo = glyph->yOffset;
                if (wrap && ((*x + (((int16_t) xo + gw) * textsize_x)) > _width)) {
                    *x = 0; // Reset x to zero, advance y by one line
                    *y += textsize_y * (uint8_t) gfxFont->yAdvance;
                }
                int16_t tsx = (int16_t) textsize_x, tsy = (int16_t) textsize_y, x1 = *x + xo * tsx,
                        y1 = *y + yo * tsy, x2 = x1 + gw * tsx - 1, y2 = y1 + gh * tsy - 1;
                if (x1 < *minx)
                    *minx = x1;
                if (y1 < *miny)
                    *miny = y1;
                if (x2 > *maxx)
                    *maxx = x2;
                if (y2 > *maxy)
                    *maxy = y2;
                *x += xa * tsx;
            }
        }

    } else { // Default font

        if (c == '\n') {          // Newline?
            *x = 0;               // Reset x to zero,
            *y += textsize_y * 8; // advance y one line
            // min/max x/y unchaged -- that waits for next 'normal' character
        } else if (c != '\r') {                             // Normal char; ignore carriage returns
            if (wrap && ((*x + textsize_x * 6) > _width)) { // Off right?
                *x = 0;                                     // Reset x to zero,
                *y += textsize_y * 8;                       // advance y one line
            }
            int x2 = *x + textsize_x * 6 - 1, // Lower-right pixel of char
                y2 = *y + textsize_y * 8 - 1;
            if (x2 > *maxx)
                *maxx = x2; // Track max x, y
            if (y2 > *maxy)
                *maxy = y2;
            if (*x < *minx)
                *minx = *x; // Track min x, y
            if (*y < *miny)
                *miny = *y;
            *x += textsize_x * 6; // Advance x one char
        }
    }
}

void Display::getTextBounds(
    const char *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
    Q_ASSERT(str);

    uint8_t c;                                                  // Current character
    int16_t minx = 0x7FFF, miny = 0x7FFF, maxx = -1, maxy = -1; // Bound rect
    // Bound rect is intentionally initialized inverted, so 1st char sets it

    *x1 = x; // Initial position is value passed in
    *y1 = y;
    *w = *h = 0; // Initial size is zero

    while ((c = *str++)) {
        // charBounds() modifies x/y to advance for each character,
        // and min/max x/y are updated to incrementally build bounding rect.
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);
    }

    if (maxx >= minx) {       // If legit string bounds were found...
        *x1 = minx;           // Update x1 to least X coord,
        *w = maxx - minx + 1; // And w to bound rect width
    }
    if (maxy >= miny) { // Same for height
        *y1 = miny;
        *h = maxy - miny + 1;
    }
}

void Display::getTextBounds(
    const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
    if (str.length() != 0) {
        getTextBounds(str.c_str(), x, y, x1, y1, w, h);
    }
}

void Display::drawInvertedBitmap(
    int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
    Q_ASSERT(bitmap);

    // taken from Adafruit_GFX.cpp, modified
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7)
                byte <<= 1;
            else {
                byte = bitmap[j * byteWidth + i / 8];
            }
            if (!(byte & 0x80)) {
                buffer.setPixel(x + i, y + j, color);
            }
        }
    }
}

void Display::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    buffer.setPixel(x, y, color);
}

size_t Display::write(uint8_t c)
{
    if (!gfxFont) { // 'Classic' built-in font

        if (c == '\n') {                                          // Newline?
            cursor_x = 0;                                         // Reset x to zero,
            cursor_y += textsize_y * 8;                           // advance y one line
        } else if (c != '\r') {                                   // Ignore carriage returns
            if (wrap && ((cursor_x + textsize_x * 6) > _width)) { // Off right?
                cursor_x = 0;                                     // Reset x to zero,
                cursor_y += textsize_y * 8;                       // advance y one line
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
            cursor_x += textsize_x * 6; // Advance x one char
        }

    } else { // Custom font

        if (c == '\n') {
            cursor_x = 0;
            cursor_y += (int16_t) textsize_y * gfxFont->yAdvance;
        } else if (c != '\r') {
            uint8_t first = gfxFont->first;
            if ((c >= first) && (c <= (uint8_t) gfxFont->last)) {
                GFXglyph *glyph = &gfxFont->glyph[c - first];
                uint8_t w = glyph->width, h = glyph->height;
                if ((w > 0) && (h > 0)) {                 // Is there an associated bitmap?
                    int16_t xo = (int8_t) glyph->xOffset; // sic
                    if (wrap && ((cursor_x + textsize_x * (xo + w)) > _width)) {
                        cursor_x = 0;
                        cursor_y += (int16_t) textsize_y * gfxFont->yAdvance;
                    }
                    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
                }
                cursor_x += (uint8_t) glyph->xAdvance * (int16_t) textsize_x;
            }
        }
    }
    return 1;
}

size_t Display::write(const char *str)
{
    if (str == NULL) {
        return 0;
    }
    return write((const uint8_t *) str, strlen(str));
}

size_t Display::write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    while (size--) {
        n += write(*buffer++);
    }
    return n;
}

void Display::fillRect(int x0, int y0, int w, int h, int color)
{
    for (int x = x0; x < x0 + w; ++x) {
        for (int y = y0; y < y0 + h; ++y) {
            buffer.setPixel(x, y, color);
        }
    }
}

void Display::drawVLine(int x0, int y0, int h, int color)
{
    for (int y = y0; y < y0 + h; ++y)
        buffer.setPixel(x0, y, color);
}

void Display::drawHLine(int x0, int y0, int w, int color)
{
    for (int x = x0; x < x0 + w; ++x)
        buffer.setPixel(x, y0, color);
}

void Display::drawChar(int16_t x,
                       int16_t y,
                       unsigned char c,
                       uint16_t color,
                       uint16_t bg,
                       uint8_t size_x,
                       uint8_t size_y)
{
    if (!gfxFont) { // 'Classic' built-in font

        if ((x >= _width) ||              // Clip right
            (y >= _height) ||             // Clip bottom
            ((x + 6 * size_x - 1) < 0) || // Clip left
            ((y + 8 * size_y - 1) < 0))   // Clip top
            return;

        if (c >= 176)
            c++; // Handle 'classic' charset behavior

        for (int8_t i = 0; i < 5; i++) { // Char bitmap = 5 columns
            uint8_t line = AdafruitDefaultFont[c * 5 + i];
            for (int8_t j = 0; j < 8; j++, line >>= 1) {
                if (line & 1) {
                    if (size_x == 1 && size_y == 1)
                        buffer.setPixel(x + i, y + j, color);
                    else
                        fillRect(x + i * size_x, y + j * size_y, size_x, size_y, color);
                } else if (bg != color) {
                    if (size_x == 1 && size_y == 1)
                        buffer.setPixel(x + i, y + j, bg);
                    else
                        fillRect(x + i * size_x, y + j * size_y, size_x, size_y, bg);
                }
            }
        }
        if (bg != color) { // If opaque, draw vertical line for last column
            if (size_x == 1 && size_y == 1)
                drawVLine(x + 5, y, 8, bg);
            else
                fillRect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
        }

    } else { // Custom font

        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling
        // drawChar() directly with 'bad' characters of font may cause mayhem!

        c -= (uint8_t) gfxFont->first;
        GFXglyph *glyph = &gfxFont->glyph[c];
        uint8_t *bitmap = gfxFont->bitmap;

        uint16_t bo = glyph->bitmapOffset;
        uint8_t w = glyph->width, h = glyph->height;
        int8_t xo = glyph->xOffset, yo = glyph->yOffset;
        uint8_t xx, yy, bits = 0, bit = 0;
        int16_t xo16 = 0, yo16 = 0;

        if (size_x > 1 || size_y > 1) {
            xo16 = xo;
            yo16 = yo;
        }

        // Todo: Add character clipping here

        // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
        // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
        // has typically been used with the 'classic' font to overwrite old
        // screen contents with new data.  This ONLY works because the
        // characters are a uniform size; it's not a sensible thing to do with
        // proportionally-spaced fonts with glyphs of varying sizes (and that
        // may overlap).  To replace previously-drawn text when using a custom
        // font, use the getTextBounds() function to determine the smallest
        // rectangle encompassing a string, erase the area with fillRect(),
        // then draw new text.  This WILL infortunately 'blink' the text, but
        // is unavoidable.  Drawing 'background' pixels will NOT fix this,
        // only creates a new set of problems.  Have an idea to work around
        // this (a canvas object type for MCUs that can afford the RAM and
        // displays supporting setAddrWindow() and pushColors()), but haven't
        // implemented this yet.

        for (yy = 0; yy < h; yy++) {
            for (xx = 0; xx < w; xx++) {
                if (!(bit++ & 7)) {
                    bits = bitmap[bo++];
                }
                if (bits & 0x80) {
                    if (size_x == 1 && size_y == 1) {
                        buffer.setPixel(x + xo + xx, y + yo + yy, color);
                    } else {
                        fillRect(x + (xo16 + xx) * size_x,
                                 y + (yo16 + yy) * size_y,
                                 size_x,
                                 size_y,
                                 color);
                    }
                }
                bits <<= 1;
            }
        }
    } // End classic vs custom font
}

void Display::drawChar(
    int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size)
{
    drawChar(x, y, c, color, bg, size, size);
}

size_t Display::write(const char *buffer, size_t size)
{
    Q_ASSERT(buffer);
    return write((const uint8_t *) buffer, size);
}

size_t Display::print(const String &s)
{
    return write(s.c_str(), s.length());
}

void Display::setFont(const GFXfont *f)
{
    if (f) {            // Font struct pointer passed in?
        if (!gfxFont) { // And no current font struct?
            // Switching from classic to new font behavior.
            // Move cursor pos down 6 pixels so it's on baseline.
            cursor_y += 6;
        }
    } else if (gfxFont) { // NULL passed.  Current font struct defined?
        // Switching from new to classic font behavior.
        // Move cursor pos up 6 pixels so it's at top-left of char.
        cursor_y -= 6;
    }
    gfxFont = const_cast<GFXfont *>(f);
}

void Display::setCursor(int x, int y)
{
    cursor_x = x;
    cursor_y = y;
}

void Display::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    if (x0 == x1) {
        if (y0 > y1)
            std::swap(y0, y1);
        drawVLine(x0, y0, y1 - y0 + 1, color);
    } else if (y0 == y1) {
        if (x0 > x1)
            std::swap(x0, x1);
        drawHLine(x0, y0, x1 - x0 + 1, color);
    } else {
        writeLine(x0, y0, x1, y1, color);
    }
}

void Display::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            buffer.setPixel(y0, x0, color);
        } else {
            buffer.setPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void Display::setTextColor(int color)
{
    textcolor = color;
}

int16_t Display::getCursorX() const
{
    return cursor_x;
}

QImage Display::image() const
{
    return buffer;
}

void Display::clear()
{
    buffer.fill(0);
}

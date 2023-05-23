#pragma once

#include <QImage>
#include <QSize>
#include <cstdint>

#include <Arduino.h>

extern const unsigned char AdafruitDefaultFont[];

static constexpr auto GxEPD_BLACK = 1;
static constexpr auto GxEPD_RED = 2;

/// Font data stored PER GLYPH
typedef struct
{
    uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
    uint8_t width;         ///< Bitmap dimensions in pixels
    uint8_t height;        ///< Bitmap dimensions in pixels
    uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
    int8_t xOffset;        ///< X dist from cursor pos to UL corner
    int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct
{
    uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
    GFXglyph *glyph;  ///< Glyph array
    uint16_t first;   ///< ASCII extents (first char)
    uint16_t last;    ///< ASCII extents (last char)
    uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;

class Display
{
public:
    Display(int width, int height);
    ~Display() {}
    void save();
    void charBounds(unsigned char c,
                    int16_t *x,
                    int16_t *y,
                    int16_t *minx,
                    int16_t *miny,
                    int16_t *maxx,
                    int16_t *maxy);
    void getTextBounds(
        const char *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void getTextBounds(
        const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void drawInvertedBitmap(
        int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    size_t write(uint8_t c);
    size_t write(const char *str);
    size_t write(const uint8_t *buffer, size_t size);
    void fillRect(int x0, int y0, int w, int h, int color);
    void drawVLine(int x0, int y0, int h, int color);
    void drawHLine(int x0, int y0, int w, int color);
    void drawChar(int16_t x,
                  int16_t y,
                  unsigned char c,
                  uint16_t color,
                  uint16_t bg,
                  uint8_t size_x,
                  uint8_t size_y);
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
    size_t write(const char *buffer, size_t size);
    size_t print(const String &s);
    void setFont(const GFXfont *f);
    void setCursor(int x, int y);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void setTextColor(int color);
    int16_t getCursorX(void) const;
    QImage image() const;
    void clear();

private:
    GFXfont *gfxFont;
    uint8_t textsize_x; ///< Desired magnification in X-axis of text to print()
    uint8_t textsize_y; ///< Desired magnification in Y-axis of text to print()
    bool wrap;
    int16_t _width;       ///< Display width as modified by current rotation
    int16_t _height;      ///< Display height as modified by current rotation
    int16_t cursor_x;     ///< x location to start print()ing text
    int16_t cursor_y;     ///< y location to start print()ing text
    uint16_t textcolor;   ///< 16-bit background color for print()
    uint16_t textbgcolor; ///< 16-bit text color for print()
    QImage buffer;
};

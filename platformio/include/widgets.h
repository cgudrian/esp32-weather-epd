#pragma once

#include <vector>

#include <Arduino.h>
#include <gfxfont.h>

namespace W {

struct Font
{
    const GFXfont &gfxFont;
    int ascent;
    int descent;
};

extern const Font DEFAULT_FONT;

class Painter
{
public:
    virtual void write(int x, int y, const String &s) = 0;
    virtual void setFont(const Font &f) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
};

template<typename Display>
class DisplayPainter : public Painter
{
public:
    DisplayPainter(Display &display)
        : _d(display)
    {}

    void write(int x, int y, const String &s) override
    {
        _d.setCursor(x, y);
        _d.print(s);
    }

    void setFont(const Font &f) override { _d.setFont(&f.gfxFont); }

    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override
    {
        _d.drawRect(x, y, w, h, color);
    }

private:
    Display &_d;
};

class Item;

class Window
{
public:
    Window(int width, int height);

    void paint(Painter &painter);
    Window &operator<<(Item &item);

private:
    std::vector<Item *> _children;
};

class Item
{
    friend class Window;

public:
    Item &operator<<(Item &item);

protected:
    virtual void paint(Painter &painter);

private:
    std::vector<Item *> _children;
};

class Text : public Item
{
public:
    Text &text(const String &t);
    Text &font(const Font &f);

protected:
    void paint(Painter &painter) override;

private:
    String _text{};
    std::reference_wrapper<const Font> _font{DEFAULT_FONT};
};

struct Size
{
    int width;
    int height;
};

struct Point
{
    int x;
    int y;
};

struct Bounds
{
    Point origin;
    Size size;
};

Bounds computeTextBounds(const Font &font, const char *text);

} // namespace W

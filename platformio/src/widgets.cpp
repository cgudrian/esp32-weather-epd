#include "widgets.h"
#include "FreeSans.h"

#ifdef QT_VERSION
#include <QDebug>
#endif

namespace W {

const Font DEFAULT_FONT{.gfxFont = FONT_8pt8b, .ascent=8, .descent=8};

W::Window::Window(int width, int height) {}

void Window::paint(Painter &painter)
{
    for (auto child : _children)
        child->paint(painter);
}

Window &Window::operator<<(Item &item)
{
    _children.emplace_back(&item);
    return *this;
}

Item &Item::operator<<(Item &item)
{
    _children.emplace_back(&item);
    return *this;
}

void Item::paint(Painter &painter)
{
    for (auto child : _children)
        child->paint(painter);
}

Text &Text::text(const String &t)
{
    _text = t;
    return *this;
}

Text &Text::font(const Font &f)
{
    _font = f;
    return *this;
}

void Text::paint(Painter &painter)
{
    auto bounds = computeTextBounds(_font, _text.c_str());
    Item::paint(painter);
    painter.setFont(_font);
    painter.write(0, -bounds.origin.y, _text);
    painter.drawRect(0, 0, bounds.size.width, bounds.size.height, 1);
}

Bounds computeTextBounds(const Font &font, const char *text)
{
    if (!text || !*text)
        // passed in text is either missing or has zero length
        return {};

    int lineWidth = 0;
    Bounds bounds{.origin = {.y = -font.gfxFont.yAdvance}, .size = {.height = font.gfxFont.yAdvance + 1}};

    while (auto c = *text++) {
        if (c == '\n') {
            // newline
            bounds.size.height += font.gfxFont.yAdvance;
            bounds.size.width = std::max(bounds.size.width, lineWidth);
            lineWidth = 0;
            continue;
        }

        if (c < font.gfxFont.first || c > font.gfxFont.last)
            continue;

        auto glyph = font.gfxFont.glyph[c - font.gfxFont.first];
        lineWidth += glyph.xAdvance;
    }

    bounds.size.width = std::max(bounds.size.width, lineWidth);

    return bounds;
}

} // namespace W

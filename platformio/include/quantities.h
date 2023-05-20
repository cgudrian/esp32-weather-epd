#pragma once

#include <cmath>

struct Kelvin
{
    static const char *const symbol() { return "K"; }
    static constexpr float factor{1};
    static constexpr float offset{0};
};

struct Celsius
{
    static const char *const symbol() { return "°C"; }
    static constexpr float factor{1};
    static constexpr float offset{-273.15};
};

struct Fahrenheit
{
    static const char *const symbol() { return "°F"; }
    static constexpr float factor{9.0 / 5.0};
    static constexpr float offset{-459.67};
};

struct Meter
{
    static const char *const symbol() { return "m"; }
    static constexpr float factor{1};
    static constexpr float offset{0};
};

struct Kilometer
{
    static const char *const symbol() { return "km"; }
    static constexpr float factor{0.001};
    static constexpr float offset{0};
};

struct MetersPerSecond
{
    static const char *const symbol() { return "m/s"; }
    static constexpr float factor{1};
    static constexpr float offset{0};
};

struct KilometersPerHour
{
    static const char *const symbol() { return "km/h"; }
    static constexpr float factor{3.6};
    static constexpr float offset{0};
};

struct Beaufort;

namespace C {
template<class FromUnit, class ToUnit>
struct Converter
{
    float operator()(float value)
    {
        return (value - FromUnit::offset) / FromUnit::factor * ToUnit::factor + ToUnit::offset;
    }
};

template<class Unit>
struct Converter<Unit, Unit>
{
    float operator()(float value) { return value; }
};

template<class FromUnit>
struct Converter<FromUnit, Beaufort>
{
    float operator()(float value)
    {
        float metersPerSec = (value - FromUnit::offset) / FromUnit::factor;
        int beaufort = static_cast<int>(
            (powf(1 / 0.836f, 2.f / 3.f) * powf(metersPerSec, 2.f / 3.f)) + .5f);
        return beaufort > 12 ? 12 : beaufort;
    }
};
} // namespace C

template<class U>
struct Quantity
{
    Quantity()
        : _val{}
    {}
    Quantity(float v)
        : _val(v)
    {}

    Quantity &operator=(float v)
    {
        _val = v;
        return *this;
    }

    float val() const { return _val; }

    template<class ToUnit>
    Quantity<ToUnit> to() const
    {
        C::Converter<U, ToUnit> convert;
        return Quantity<ToUnit>(convert(this->_val));
    }

    template<class ToUnit>
    float in() const
    {
        return to<ToUnit>().val();
    }

    template<class ToUnit>
    operator Quantity<ToUnit>() const
    {
        return to<ToUnit>();
    }

protected:
    float _val;
};

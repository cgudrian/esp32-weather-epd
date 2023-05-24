#pragma once

#include <cmath>

#include "config.h"

#define DEG "\xB0"

struct Kelvin
{
    static constexpr const char *const symbol{"K"};
    static constexpr const char *const shortSym{"K"};
    static constexpr float factor{1};
    static constexpr float offset{0};
};

struct Celsius
{
    static constexpr const char * const symbol{DEG "C"};
    static constexpr const char *const shortSym{DEG};
    static constexpr float factor{1};
    static constexpr float offset{-273.15};
};

struct Fahrenheit
{
    static constexpr const char * const symbol{DEG "F"};
    static constexpr const char *const shortSym{DEG};
    static constexpr float factor{9.0 / 5.0};
    static constexpr float offset{-459.67};
};

struct Meters
{
    static constexpr const char * const symbol{"m"};
    static constexpr float factor{1};
    static constexpr float offset{0};
    static constexpr float maxVisibility{10000.0};
};

struct Kilometers
{
    static constexpr const char * const symbol{"km"};
    static constexpr float factor{1 / 1000.0};
    static constexpr float offset{0};
    static constexpr float maxVisibility{10.0};
};

struct Miles
{
    static constexpr const char * const symbol{"mi"};
    static constexpr float factor{1 / 1609.344};
    static constexpr float offset{0};
    static constexpr float maxVisibility{6.0};
};

struct MetersPerSecond
{
    static constexpr const char * const symbol{"m/s"};
    static constexpr float factor{1};
    static constexpr float offset{0};
};

struct KilometersPerHour
{
    static constexpr const char * const symbol{"km/h"};
    static constexpr float factor{3.6};
    static constexpr float offset{0};
};

struct MilesPerHour
{
    static constexpr const char * const symbol{"mph"};
    static constexpr float factor{3.6 / 1.609344};
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

using TemperatureUnit = UNITS_TEMP;
using DistanceUnit = UNITS_DIST;
using SpeedUnit = UNITS_SPEED;

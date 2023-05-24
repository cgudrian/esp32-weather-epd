/* Unit conversion functions for esp32-weather-epd.
 * Copyright (C) 2023  Luke Marzen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "conversions.h"

#include <cmath>

float hectopascals_to_pascals(float hectopascals)
{
  return hectopascals * 100.f;
} // end hectopascals_to_pascals

float hectopascals_to_millimetersofmercury(float hectopascals)
{
  return hectopascals * 0.7501f;
} // end hectopascals_to_millimetersofmercury

float hectopascals_to_inchesofmercury(float hectopascals)
{
  return hectopascals * 0.02953f;
} // end hectopascals_to_inchesofmercury

float hectopascals_to_millibars(float hectopascals)
{
  return hectopascals * 1.f;
} // end hectopascals_to_millibars

float hectopascals_to_atmospheres(float hectopascals)
{
  return hectopascals * 9.869e-4f;
} // end hectopascals_to_atmospheres

float hectopascals_to_gramspersquarecentimeter(float hectopascals)
{
  return hectopascals * 1.02f;
} // end hectopascals_to_gramspersquarecentimeter

float hectopascals_to_poundspersquareinch(float hectopascals)
{
  return hectopascals * 0.0145f;
} // end hectopascals_to_poundspersquareinch

/* Renderer for esp32-weather-epd.
 * Copyright (C) 2022-2023  Luke Marzen
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

#include <owa-icons.h>

#include "_locale.h"
#include "_strftime.h"
#include "renderer.h"
#include "api_response.h"
#include "config.h"
#include "conversions.h"
#include "display_utils.h"

#ifdef SIMULATION
#include <QDebug>
#endif

// fonts
#include FONT_HEADER

#ifdef ARDUINO
#ifdef DISP_BW
GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(
  GxEPD2_750_T7(PIN_EPD_CS,
                PIN_EPD_DC,
                PIN_EPD_RST,
                PIN_EPD_BUSY));
#endif
#ifdef DISP_3C
GxEPD2_3C<GxEPD2_750c_Z08, GxEPD2_750c_Z08::HEIGHT / 2> display(
  GxEPD2_750c_Z08(PIN_EPD_CS,
                  PIN_EPD_DC,
                  PIN_EPD_RST,
                  PIN_EPD_BUSY));
#endif
#else
Display display(DISP_WIDTH, DISP_HEIGHT);
#endif

#ifndef ACCENT_COLOR
  #define ACCENT_COLOR GxEPD_BLACK
#endif

/* Returns the string width in pixels
 */
uint16_t getStringWidth(String text)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return w;
}

/* Returns the string height in pixels
 */
uint16_t getStringHeight(String text)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return h;
}

/* Draws a string with alignment
 */
void drawString(int16_t x, int16_t y, String text, alignment_t alignment,
                uint16_t color)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextColor(color);
  display.getTextBounds(text, x, y, &x1, &y1, &w, &h);
  if (alignment == RIGHT)
    x = x - w;
  if (alignment == CENTER)
    x = x - w / 2;
  display.setCursor(x, y);
  display.print(text);
} // end drawString

/* Draws a string that will flow into the next line when max_width is reached.
 * If a string exceeds max_lines an ellipsis (...) will terminate the last word.
 * Lines will break at spaces(' ') and dashes('-').
 * 
 * Note: max_width should be big enough to accommodate the largest word that
 *       will be displayed. If an unbroken string of characters longer than
 *       max_width exist in text, then the string will be printed beyond 
 *       max_width.
 */
void drawMultiLnString(int16_t x, int16_t y, String text, alignment_t alignment,
                       uint16_t max_width, uint16_t max_lines,
                       int16_t line_spacing, uint16_t color)
{

  uint16_t current_line = 0;
  String textRemaining = text;
  // print until we reach max_lines or no more text remains
  while (current_line < max_lines && !textRemaining.isEmpty())
  {
    int16_t  x1, y1;
    uint16_t w, h;

    display.getTextBounds(textRemaining, 0, 0, &x1, &y1, &w, &h);

    int endIndex = textRemaining.length();
    // check if remaining text is to wide, if it is then print what we can
    String subStr = textRemaining;
    int splitAt = 0;
    int keepLastChar = 0;
    while (w > max_width && splitAt != -1)
    {
      if (keepLastChar)
      {
        // if we kept the last character during the last iteration of this while
        // loop, remove it now so we don't get stuck in an infinite loop.
        subStr.remove(subStr.length() - 1);
      }

      // find the last place in the string that we can break it.
      if (current_line < max_lines - 1)
      {
        splitAt = max(subStr.lastIndexOf(" "),
                      subStr.lastIndexOf("-"));
      }
      else
      {
        // this is the last line, only break at spaces so we can add ellipsis
        splitAt = subStr.lastIndexOf(" ");
      }

      // if splitAt == -1 then there is an unbroken set of characters that is
      // longer than max_width. Otherwise if splitAt != -1 then we can continue
      // the loop until the string is <= max_width
      if (splitAt != -1)
      {
        endIndex = splitAt;
        subStr = subStr.substring(0, endIndex + 1);

        char lastChar = subStr.charAt(endIndex);
        if (lastChar == ' ')
        {
          // remove this char now so it is not counted towards line width
          keepLastChar = 0;
          subStr.remove(endIndex);
          --endIndex;
        }
        else if (lastChar == '-')
        {
          // this char will be printed on this line and removed next iteration
          keepLastChar = 1;
        }

        if (current_line < max_lines - 1)
        {
          // this is not the last line
          display.getTextBounds(subStr, 0, 0, &x1, &y1, &w, &h);
        }
        else
        {
          // this is the last line, we need to make sure there is space for
          // ellipsis
          display.getTextBounds(subStr + "...", 0, 0, &x1, &y1, &w, &h);
          if (w <= max_width)
          {
            // ellipsis fit, add them to subStr
            subStr = subStr + "...";
          }
        }

      } // end if (splitAt != -1)
    } // end inner while

    drawString(x, y + (current_line * line_spacing), subStr, alignment, color);

    // update textRemaining to no longer include what was printed
    // +1 for exclusive bounds, +1 to get passed space/dash
    textRemaining = textRemaining.substring(endIndex + 2 - keepLastChar);

    ++current_line;
  } // end outer while

  return;
} // end drawMultiLnString

/* Initialize e-paper display
 */
void initDisplay()
{
#ifdef ARDUINO
  display.init(115200, true, 2, false);
  // display.init(); for older Waveshare HAT's
  SPI.begin(PIN_EPD_SCK,
            PIN_EPD_MISO,
            PIN_EPD_MOSI,
            PIN_EPD_CS);

  display.setRotation(0);
  display.setTextSize(1);
  display.setTextColor(GxEPD_BLACK);
  display.setTextWrap(false);
  display.fillScreen(GxEPD_WHITE);
  display.setFullWindow();
#endif
} // end initDisplay

void drawSunrise(int x, int y, const owm_current_t &current)
{
  time_t ts = current.sunrise;
  tm *timeInfo = localtime(&ts);
  char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
  _strftime(timeBuffer, sizeof(timeBuffer), TIME_FORMAT, timeInfo);
  display.drawInvertedBitmap(x, y, wi_sunrise_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_SUNRISE, LEFT);
  display.setFont(&FONT_12pt8b);
  drawString(x + 48, y + 17 / 2 + 48 / 2, timeBuffer, LEFT);
}

void drawWind(int x, int y, const owm_current_t &current)
{
  display.drawInvertedBitmap(x, y, wi_strong_wind_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_WIND, LEFT);
  display.drawInvertedBitmap(x + 48,
                             y + 24 / 2,
                             getWindBitmap24(current.wind_deg),
                             24,
                             24,
                             GxEPD_BLACK);
  auto dataStr = String(static_cast<int>(round(current.wind_speed.in<SpeedUnit>())));
  drawString(x + 48 + 24, y + 17 / 2 + 48 / 2, dataStr, LEFT);
  display.setFont(&FONT_8pt8b);
  drawString(display.getCursorX(), y + 17 / 2 + 48 / 2, SpeedUnit::symbol, LEFT);
}

void drawUVIndex(int x, int y, const owm_current_t &current, int sp)
{
  display.drawInvertedBitmap(x, y, wi_day_sunny_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_UV_INDEX, LEFT);
  display.setFont(&FONT_12pt8b);
  uint uvi = static_cast<uint>(max(round(current.uvi), 0.0f));
  drawString(x + 48, y + 17 / 2 + 48 / 2, String(uvi), LEFT);
  display.setFont(&FONT_7pt8b);
  auto dataStr = String(getUVIdesc(uvi));
  int max_w = 170 - (display.getCursorX() + sp);
  if (getStringWidth(dataStr) <= max_w) { // Fits on a single line, draw along bottom
    drawString(display.getCursorX() + sp, y + 17 / 2 + 48 / 2, dataStr, LEFT);
  } else { // use smaller font
    display.setFont(&FONT_5pt8b);
    if (getStringWidth(dataStr)
        <= max_w) { // Fits on a single line with smaller font, draw along bottom
      drawString(display.getCursorX() + sp, y + 17 / 2 + 48 / 2, dataStr, LEFT);
    } else { // Does not fit on a single line, draw higher to allow room for 2nd line
      drawMultiLnString(display.getCursorX() + sp,
                        y + 17 / 2 + 48 / 2 - 10,
                        dataStr,
                        LEFT,
                        max_w,
                        2,
                        10);
    }
  }
}

void drawAQI(int x, int y, const owm_resp_air_pollution_t &owm_air_pollution, int sp)
{
  display.drawInvertedBitmap(x, y, air_filter_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_AIR_QUALITY_INDEX, LEFT);
  display.setFont(&FONT_12pt8b);
  int aqi = getAQI(owm_air_pollution);
  drawString(x + 48, y + 17 / 2 + 48 / 2, String(aqi), LEFT);
  display.setFont(&FONT_7pt8b);
  auto dataStr = String(getAQIdesc(aqi));
  auto max_w = 170 - (display.getCursorX() + sp);
  if (getStringWidth(dataStr) <= max_w) { // Fits on a single line, draw along bottom
    drawString(display.getCursorX() + sp, y + 17 / 2 + 48 / 2, dataStr, LEFT);
  } else { // use smaller font
    display.setFont(&FONT_5pt8b);
    if (getStringWidth(dataStr)
        <= max_w) { // Fits on a single line with smaller font, draw along bottom
      drawString(display.getCursorX() + sp, y + 17 / 2 + 48 / 2, dataStr, LEFT);
    } else { // Does not fit on a single line, draw higher to allow room for 2nd line
      drawMultiLnString(display.getCursorX() + sp,
                        y + 17 / 2 + 48 / 2 - 10,
                        dataStr,
                        LEFT,
                        max_w,
                        2,
                        10);
    }
  }
}

void drawIndoorTemperature(int x, int y, const std::optional<Quantity<TemperatureUnit>> &inTemp)
{
  display.drawInvertedBitmap(x, y, house_thermometer_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_INDOOR_TEMPERATURE, LEFT);
  display.setFont(&FONT_12pt8b);
  String dataStr = inTemp ? String(static_cast<int>(round(inTemp->val()))) : String("--");
  dataStr += TemperatureUnit::shortSym;
  drawString(x + 48, y + 17 / 2 + 48 / 2, dataStr, LEFT);
}

void drawSunset(int x, int y, const owm_current_t &current)
{
  time_t ts = current.sunset;
  tm *timeInfo = localtime(&ts);
  char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
  _strftime(timeBuffer, sizeof(timeBuffer), TIME_FORMAT, timeInfo);
  display.drawInvertedBitmap(x, y, wi_sunset_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_SUNSET, LEFT);
  display.setFont(&FONT_12pt8b);
  drawString(x + 48, y + 17 / 2 + 48 / 2, timeBuffer, LEFT);
}

void drawHumidity(int x, int y, const owm_current_t &current)
{
  display.drawInvertedBitmap(x, y, wi_humidity_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_HUMIDITY, LEFT);
  display.setFont(&FONT_12pt8b);
  drawString(x + 48, y + 17 / 2 + 48 / 2, String(current.humidity), LEFT);
  display.setFont(&FONT_8pt8b);
  drawString(display.getCursorX(), y + 17 / 2 + 48 / 2, "%", LEFT);
}

void drawPressure(int x, int y, const owm_current_t &current)
{
  display.drawInvertedBitmap(x, y, wi_barometer_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_PRESSURE, LEFT);
#ifdef UNITS_PRES_HECTOPASCALS
  dataStr = String(current.pressure);
  unitStr = TXT_UNITS_PRES_HECTOPASCALS;
#endif
#ifdef UNITS_PRES_PASCALS
  dataStr = String(static_cast<int>(round(hectopascals_to_pascals(current.pressure))));
  unitStr = TXT_UNITS_PRES_PASCALS;
#endif
#ifdef UNITS_PRES_MILLIMETERSOFMERCURY
  dataStr = String(static_cast<int>(round(hectopascals_to_millimetersofmercury(current.pressure))));
  unitStr = TXT_UNITS_PRES_MILLIMETERSOFMERCURY;
#endif
#ifdef UNITS_PRES_INCHESOFMERCURY
  auto dataStr = String(round(1e1f * hectopascals_to_inchesofmercury(current.pressure)) / 1e1f, 1);
  auto unitStr = TXT_UNITS_PRES_INCHESOFMERCURY;
#endif
#ifdef UNITS_PRES_MILLIBARS
  dataStr = String(static_cast<int>(round(hectopascals_to_millibars(current.pressure))));
  unitStr = TXT_UNITS_PRES_MILLIBARS;
#endif
#ifdef UNITS_PRES_ATMOSPHERES
  dataStr = String(round(1e3f * hectopascals_to_atmospheres(current.pressure)) / 1e3f, 3);
  unitStr = TXT_UNITS_PRES_ATMOSPHERES;
#endif
#ifdef UNITS_PRES_GRAMSPERSQUARECENTIMETER
  dataStr = String(
      static_cast<int>(round(hectopascals_to_gramspersquarecentimeter(current.pressure))));
  unitStr = TXT_UNITS_PRES_GRAMSPERSQUARECENTIMETER;
#endif
#ifdef UNITS_PRES_POUNDSPERSQUAREINCH
  dataStr = String(round(1e2f * hectopascals_to_poundspersquareinch(current.pressure)) / 1e2f, 2);
  unitStr = TXT_UNITS_PRES_POUNDSPERSQUAREINCH;
#endif
  display.setFont(&FONT_12pt8b);
  drawString(x + 48, y + 17 / 2 + 48 / 2, dataStr, LEFT);
  display.setFont(&FONT_8pt8b);
  drawString(display.getCursorX(), y + 17 / 2 + 48 / 2, unitStr, LEFT);
}

void drawVisibility(int x, int y, const owm_current_t &current)
{
  display.drawInvertedBitmap(x, y, visibility_icon_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_VISIBILITY, LEFT);
  display.setFont(&FONT_12pt8b);
  float vis = current.visibility.in<DistanceUnit>();
  auto unitStr = DistanceUnit::symbol;
  String dataStr;
  // if visibility is less than 1.95, round to 1 decimal place
  // else round to int
  if (vis < 1.95) {
    dataStr = String(round(10 * vis) / 10.0, 1);
  } else {
    dataStr = String(static_cast<int>(round(vis)));
  }
  if (vis >= DistanceUnit::maxVisibility) {
    dataStr = "> " + dataStr;
  }
  drawString(x + 48, y + 17 / 2 + 48 / 2, dataStr, LEFT);
  display.setFont(&FONT_8pt8b);
  drawString(display.getCursorX(), y + 17 / 2 + 48 / 2, unitStr, LEFT);
}

void drawIndoorHumidity(int x, int y, const std::optional<float> &inHumidity)
{
  display.drawInvertedBitmap(x, y, house_humidity_48x48, 48, 48, GxEPD_BLACK);
  display.setFont(&FONT_7pt8b);
  drawString(x + 48, y + 10, TXT_INDOOR_HUMIDITY, LEFT);
  display.setFont(&FONT_12pt8b);
  auto dataStr = inHumidity ? String(static_cast<int>(round(*inHumidity))) : String("--");
  drawString(x + 48, y + 17 / 2 + 48 / 2, dataStr, LEFT);
  display.setFont(&FONT_8pt8b);
  drawString(display.getCursorX(), y + 17 / 2 + 48 / 2, "%", LEFT);
}

namespace G {
class Sunrise
{
  public:
  Sunrise(time_t time)
      : x(0)
      , y(0)
      , time(time)
  {}

  void draw()
  {
    tm *timeInfo = localtime(&time);
    char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
    _strftime(timeBuffer, sizeof(timeBuffer), TIME_FORMAT, timeInfo);
    display.drawInvertedBitmap(x, y, wi_sunrise_48x48, 48, 48, GxEPD_BLACK);
    display.setFont(&FONT_7pt8b);
    drawString(x + 48, y + 10, TXT_SUNRISE, LEFT);
    display.setFont(&FONT_12pt8b);
    drawString(x + 48, y + 17 / 2 + 48 / 2, timeBuffer, LEFT);
  }

  private:
  int x;
  int y;
  time_t time;
};
} // namespace G

template<int dx, int dy>
void drawDataGrid(int x,
                  int y,
                  const owm_current_t &current,
                  const owm_resp_air_pollution_t &owm_air_pollution,
                  const std::optional<Quantity<TemperatureUnit>> &inTemp,
                  const std::optional<float> &inHumidity)
{
  int row = 0;
  int col = 0;

  drawSunrise(x + dx * col, y + dy * row++, current);
  drawWind(x + dx * col, y + dy * row++, current);
  drawUVIndex(x + dx * col, y + dy * row++, current, 8);
  drawAQI(x + dx * col, y + dy * row++, owm_air_pollution, 8);
  drawIndoorTemperature(x + col, y + dy * row++, inTemp);

  col++;
  row = 0;

  drawSunset(x + dx * col, y + dy * row++, current);
  drawHumidity(x + dx * col, y + dy * row++, current);
  drawPressure(x + dx * col, y + dy * row++, current);
  drawVisibility(x + dx * col, y + dy * row++, current);
  drawIndoorHumidity(x + dx * col, y + dy * row++, inHumidity);
}

/* This function is responsible for drawing the current conditions and
 * associated icons.
 */
void drawCurrentConditions(const owm_current_t &current,
                           const owm_daily_t &today,
                           const owm_resp_air_pollution_t &owm_air_pollution,
                           const std::optional<Quantity<TemperatureUnit>> &inTemp,
                           const std::optional<float> &inHumidity)
{
  // current weather icon
  display.drawInvertedBitmap(0,
                             0,
                             getCurrentConditionsBitmap196(current, today),
                             196,
                             196,
                             GxEPD_BLACK);

  // current temp
  auto dataStr = String(static_cast<int>(round(current.temp.in<TemperatureUnit>())));
  auto unitStr = TemperatureUnit::symbol;
  // FONT_**_temperature fonts only have the character set used for displaying
  // temperature (0123456789.-\xB0)
  display.setFont(&FONT_48pt8b_temperature);
  drawString(196 + 164 / 2 - 20, 196 / 2 + 69 / 2, dataStr, CENTER);
  display.setFont(&FONT_14pt8b);
  drawString(display.getCursorX(), 196 / 2 - 69 / 2 + 20, unitStr, LEFT);

  // current feels like
  dataStr = String(TXT_FEELS_LIKE) + ' '
            + String(static_cast<int>(round(current.feels_like.in<TemperatureUnit>())))
            + TemperatureUnit::shortSym;
  display.setFont(&FONT_12pt8b);
  drawString(196 + 164 / 2, 98 + 69 / 2 + 12 + 17, dataStr, CENTER);

  // line dividing top and bottom display areas
  // display.drawLine(0, 196, DISP_WIDTH - 1, 196, GxEPD_BLACK);

  drawDataGrid<170, 48 + 8>(0, 204, current, owm_air_pollution, inTemp, inHumidity);
}

void drawForecastForDay(const owm_daily_t& day, tm timeInfo, int x)
{
  // icons
  display.drawInvertedBitmap(x, 98 + 69 / 2 - 32 - 6,
                             getForecastBitmap64(day),
                             64, 64, GxEPD_BLACK);
  // day of week label
  display.setFont(&FONT_11pt8b);
  char dayBuffer[8] = {};
  _strftime(dayBuffer, sizeof(dayBuffer), "%a", &timeInfo); // abbrv'd day
  drawString(x + 31 - 2, 98 + 69 / 2 - 32 - 26 - 6 + 16, dayBuffer, CENTER);

  // high | low

  auto hiStr = String(static_cast<int>(round(day.temp.max.in<TemperatureUnit>()))) + TemperatureUnit::shortSym;
  auto loStr = String(static_cast<int>(round(day.temp.min.in<TemperatureUnit>()))) + TemperatureUnit::shortSym;
  display.setFont(&FONT_8pt8b);
  drawString(x + 31 - 4, 98 + 69 / 2 + 38 - 6 + 12, hiStr, RIGHT);
  drawString(x + 31 + 8, 98 + 69 / 2 + 38 - 6 + 12, loStr, LEFT);
  drawString(x + 31, 98 + 69 / 2 + 38 - 6 + 12, "|", CENTER);
}

/* This function is responsible for drawing the five day forecast.
 */
void drawForecast(owm_daily_t *const daily, tm timeInfo)
{
  for (int i = 0; i < 5; ++i) {
    int x = 398 + (i * 82);
    drawForecastForDay(daily[i], timeInfo, x);
    timeInfo.tm_wday = (timeInfo.tm_wday + 1) % 7; // increment to next day
  }
}

/* This function is responsible for drawing the current alerts if any.
 * Up to 2 alerts can be drawn.
 */
void drawAlerts(std::vector<owm_alerts_t> &alerts,
                const String &city, const String &date)
{
  if (alerts.size() == 0)
  { // no alerts to draw
    return;
  }

  // Converts all event text and tags to lowercase, removes extra information,
  // and filters out redundant alerts of lesser urgency.
#ifdef ARDUINO
  int ignore_list[alerts.size()] = {};
#else
  std::vector<int> ignore_list_vec(alerts.size());
  auto ignore_list = ignore_list_vec.data();
#endif
  filterAlerts(alerts, ignore_list);

  // limit alert text width so that is does not run into the location or date
  // strings
  display.setFont(&FONT_16pt8b);
  int city_w = getStringWidth(city);
  display.setFont(&FONT_12pt8b);
  int date_w = getStringWidth(date);
  int max_w = DISP_WIDTH - 2 - max(city_w, date_w) - (196 + 4) - 8;

  // find indicies of valid alerts
#ifdef ARDUINO
  int alert_indices[alerts.size()] = {};
#else
  std::vector<int> alert_indices_vec(alerts.size());
  auto alert_indices = alert_indices_vec.data();
#endif
  int num_valid_alerts = 0;
  for (int i = 0; i < alerts.size(); ++i)
  {
    if (!ignore_list[i])
    {
      alert_indices[num_valid_alerts] = i;
      ++num_valid_alerts;
    }
  }

  if (num_valid_alerts == 1)
  { // 1 alert
    // adjust max width to for 48x48 icons
    max_w -= 48;

    owm_alerts_t &cur_alert = alerts[alert_indices[0]];
    display.drawInvertedBitmap(196, 8, getAlertBitmap48(cur_alert), 48, 48,
                               ACCENT_COLOR);
    // must be called after getAlertBitmap
    toTitleCase(cur_alert.event);

    display.setFont(&FONT_14pt8b);
    if (getStringWidth(cur_alert.event) <= max_w)
    { // Fits on a single line, draw along bottom
      drawString(196 + 48 + 4, 24 + 8 - 12 + 20 + 1, cur_alert.event, LEFT);
    }
    else
    { // use smaller font
      display.setFont(&FONT_12pt8b);
      if (getStringWidth(cur_alert.event) <= max_w)
      { // Fits on a single line with smaller font, draw along bottom
        drawString(196 + 48 + 4, 24 + 8 - 12 + 17 + 1, cur_alert.event, LEFT);
      }
      else
      { // Does not fit on a single line, draw higher to allow room for 2nd line
        drawMultiLnString(196 + 48 + 4, 24 + 8 - 12 + 17 - 11,
                          cur_alert.event, LEFT, max_w, 2, 23);
      }
    }
  } // end 1 alert
  else
  { // 2 alerts
    // adjust max width to for 32x32 icons
    max_w -= 32;

    display.setFont(&FONT_12pt8b);
    for (int i = 0; i < 2; ++i)
    {
      owm_alerts_t &cur_alert = alerts[alert_indices[i]];

      display.drawInvertedBitmap(196, (i * 32), getAlertBitmap32(cur_alert),
                                 32, 32, ACCENT_COLOR);
      // must be called after getAlertBitmap
      toTitleCase(cur_alert.event);

      drawMultiLnString(196 + 32 + 3, 5 + 17 + (i * 32),
                        cur_alert.event, LEFT, max_w, 1, 0);
    } // end for-loop
  } // end 2 alerts

  return;
} // end drawAlerts

/* This function is responsible for drawing the city string and date
 * information in the top right corner.
 */
void drawLocationDate(const String &city, const String &date)
{
  // location, date
  display.setFont(&FONT_16pt8b);
  drawString(DISP_WIDTH - 2, 23, city, RIGHT, ACCENT_COLOR);
  display.setFont(&FONT_12pt8b);
  drawString(DISP_WIDTH - 2, 30 + 4 + 17, date, RIGHT);
  return;
} // end drawLocationDate

/* The % operator in C++ is not a true modulo operator but it instead a
 * remainder operator. The remainder operator and modulo operator are equivalent
 * for positive numbers, but not for negatives. The follow implementation of the
 * modulo operator works for +/-a and +b.
 */
inline int modulo(int a, int b)
{
  const int result = a % b;
  return result >= 0 ? result : result + b;
}

/* This function is responsible for drawing the outlook graph for the specified
 * number of hours(up to 47).
 */
void drawOutlookGraph(owm_hourly_t *const hourly, tm timeInfo)
{

  const int xPos0 = 350;
  const int xPos1 = DISP_WIDTH - 46;
  const int yPos0 = 216;
  const int yPos1 = DISP_HEIGHT - 46;

  // x axis
  display.drawLine(xPos0, yPos1    , xPos1, yPos1    , GxEPD_BLACK);
  display.drawLine(xPos0, yPos1 - 1, xPos1, yPos1 - 1, GxEPD_BLACK);

  // calculate y max/min and intervals
  int yMajorTicks = 5;
  float tempMin = hourly[0].temp.in<TemperatureUnit>();
  float tempMax = tempMin;
  int yTempMajorTicks = 5;
  float newTemp = 0;

  for (int i = 1; i < HOURLY_GRAPH_MAX; ++i)
  {
    newTemp = hourly[i].temp.in<TemperatureUnit>();
    tempMin = std::min(tempMin, newTemp);
    tempMax = std::max(tempMax, newTemp);
  }

  int tempBoundMin = static_cast<int>(tempMin - 1) - modulo(static_cast<int>(tempMin - 1), yTempMajorTicks);
  int tempBoundMax = static_cast<int>(tempMax + 1) + (yTempMajorTicks - modulo(static_cast<int>(tempMax + 1), yTempMajorTicks));

  // while we have to many major ticks then increase the step
  while ((tempBoundMax - tempBoundMin) / yTempMajorTicks > yMajorTicks)
  {
    yTempMajorTicks += 5;
    tempBoundMin = static_cast<int>(tempMin - 1)
                      - modulo(static_cast<int>(tempMin - 1), yTempMajorTicks);
    tempBoundMax = static_cast<int>(tempMax + 1) + (yTempMajorTicks
                      - modulo(static_cast<int>(tempMax + 1), yTempMajorTicks));
  }

  // while we have not enough major ticks add to either bound
  while ((tempBoundMax - tempBoundMin) / yTempMajorTicks < yMajorTicks)
  {
    // add to whatever bound is closer to the actual min/max
    if (tempMin - tempBoundMin <= tempBoundMax - tempMax)
    {
      tempBoundMin -= yTempMajorTicks;
    }
    else
    {
      tempBoundMax += yTempMajorTicks;
    }
  }

  // draw y axis
  float yInterval = (yPos1 - yPos0) / static_cast<float>(yMajorTicks);

  for (int i = 0; i <= yMajorTicks; ++i)
  {
    String dataStr;
    int yTick = static_cast<int>(yPos0 + (i * yInterval));
    display.setFont(&FONT_8pt8b);
    // Temperature
    dataStr = String(tempBoundMax - (i * yTempMajorTicks));
    dataStr += TemperatureUnit::shortSym;
    drawString(xPos0 - 8, yTick + 4, dataStr, RIGHT, ACCENT_COLOR);

    // PoP
    dataStr = String(100 - (i * 20));
    drawString(xPos1 + 8, yTick + 4, dataStr, LEFT);
    display.setFont(&FONT_5pt8b);
    drawString(display.getCursorX(), yTick + 4, "%", LEFT);

    // draw dotted line
    if (i < yMajorTicks)
    {
      for (int x = xPos0; x <= xPos1 + 1; x += 3)
      {
        display.drawPixel(x, yTick + (yTick % 2), GxEPD_BLACK);
      }
    }
  }

  int xMaxTicks = 8;
  int hourInterval = static_cast<int>(ceil(HOURLY_GRAPH_MAX
                                           / static_cast<float>(xMaxTicks)));
  float xInterval = (xPos1 - xPos0 - 1) / static_cast<float>(HOURLY_GRAPH_MAX);
  display.setFont(&FONT_8pt8b);

  const float yPxPerUnit_t = (yPos1 - yPos0) / static_cast<float>(tempBoundMax - tempBoundMin);
  const float yPxPerUnit_p = (yPos1 - yPos0) / 100.0;

  for (int i = 0; i < HOURLY_GRAPH_MAX; ++i)
  {
    int xTick = static_cast<int>(xPos0 + (i * xInterval));
    int x0_t, x1_t, y0_t, y1_t;

    if (i > 0)
    {
      // temperature
      x0_t = static_cast<int>(round(xPos0 + ((i - 1) * xInterval) + (0.5 * xInterval)));
      x1_t = static_cast<int>(round(xPos0 + (i * xInterval) + (0.5 * xInterval)));

      auto prevTemp = hourly[i - 1].temp.in<TemperatureUnit>();
      auto currTemp = hourly[i].temp.in<TemperatureUnit>();
      y0_t = static_cast<int>(round(yPos1 - yPxPerUnit_t * (prevTemp - tempBoundMin)));
      y1_t = static_cast<int>(round(yPos1 - yPxPerUnit_t * (currTemp - tempBoundMin)));

      // graph temperature
      display.drawLine(x0_t    , y0_t    , x1_t    , y1_t    , ACCENT_COLOR);
      display.drawLine(x0_t    , y0_t + 1, x1_t    , y1_t + 1, ACCENT_COLOR);
      display.drawLine(x0_t - 1, y0_t    , x1_t - 1, y1_t    , ACCENT_COLOR);
    }

    // PoP
    x0_t = static_cast<int>(round( xPos0 + 1 + (i * xInterval)));
    x1_t = static_cast<int>(round( xPos0 + 1 + ((i + 1) * xInterval) ));
    y0_t = static_cast<int>(round(
                            yPos1 - (yPxPerUnit_p * (hourly[i].pop * 100)) ));
    y1_t = yPos1;

    // graph PoP
    for (int y = y1_t - 1; y > y0_t; y -= 2)
    {
      for (int x = x0_t + (x0_t % 2); x < x1_t; x += 2)
      {
        display.drawPixel(x, y, GxEPD_BLACK);
      }
    }

    if ((i % hourInterval) == 0)
    {
      // draw x tick marks
      display.drawLine(xTick    , yPos1 + 1, xTick    , yPos1 + 4, GxEPD_BLACK);
      display.drawLine(xTick + 1, yPos1 + 1, xTick + 1, yPos1 + 4, GxEPD_BLACK);
      // draw x axis labels
      char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
      time_t ts = hourly[i].dt;
      tm *timeInfo = localtime(&ts);
      _strftime(timeBuffer, sizeof(timeBuffer), HOUR_FORMAT, timeInfo);
      drawString(xTick, yPos1 + 1 + 12 + 4 + 3, timeBuffer, CENTER);
    }

  }

  // draw the last tick mark
  if ((HOURLY_GRAPH_MAX % hourInterval) == 0)
  {
    int xTick = static_cast<int>(round(xPos0 + (HOURLY_GRAPH_MAX * xInterval)));
    // draw x tick marks
    display.drawLine(xTick    , yPos1 + 1, xTick    , yPos1 + 4, GxEPD_BLACK);
    display.drawLine(xTick + 1, yPos1 + 1, xTick + 1, yPos1 + 4, GxEPD_BLACK);
    // draw x axis labels
    char timeBuffer[12] = {}; // big enough to accommodate "hh:mm:ss am"
    time_t ts = hourly[HOURLY_GRAPH_MAX - 1].dt + 3600;
    tm *timeInfo = localtime(&ts);
    _strftime(timeBuffer, sizeof(timeBuffer), HOUR_FORMAT, timeInfo);
    drawString(xTick, yPos1 + 1 + 12 + 4 + 3, timeBuffer, CENTER);
  }

  return;
} // end drawOutlookGraph

/* This function is responsible for drawing the status bar along the bottom of
 * the display.
 */
void drawStatusBar(String statusStr, String refreshTimeStr, int rssi,
                   double batVoltage)
{
  String dataStr;
  uint16_t dataColor = GxEPD_BLACK;
  display.setFont(&FONT_6pt8b);
  int pos = DISP_WIDTH - 2;
  const int sp = 2;

  // battery
  int batPercent = calcBatPercent(batVoltage);
  if (batVoltage < BATTERY_WARN_VOLTAGE) {
    dataColor = ACCENT_COLOR;
  }
  dataStr = String(batPercent) + "% ("
            + String( round(100.0 * batVoltage) / 100.0, 2 ) + "v)";
  drawString(pos, DISP_HEIGHT - 2 - 2, dataStr, RIGHT, dataColor);
  pos -= getStringWidth(dataStr) + 25;
  display.drawInvertedBitmap(pos, DISP_HEIGHT - 2 - 17,
                             getBatBitmap24(batPercent), 24, 24, dataColor);
  pos -= sp + 9;

  // wifi
  dataStr = String(getWiFidesc(rssi));
  dataColor = rssi >= -70 ? GxEPD_BLACK : ACCENT_COLOR;
  if (rssi != 0)
  {
    dataStr += " (" + String(rssi) + "dBm)";
  }
  drawString(pos, DISP_HEIGHT - 2 - 2, dataStr, RIGHT, dataColor);
  pos -= getStringWidth(dataStr) + 19;
  display.drawInvertedBitmap(pos, DISP_HEIGHT - 2 - 13, getWiFiBitmap16(rssi),
                             16, 16, dataColor);
  pos -= sp + 8;

  // last refresh
  dataColor = GxEPD_BLACK;
  drawString(pos, DISP_HEIGHT - 2 - 2, refreshTimeStr, RIGHT, dataColor);
  pos -= getStringWidth(refreshTimeStr) + 25;
  display.drawInvertedBitmap(pos, DISP_HEIGHT - 2 - 21, wi_refresh_32x32,
                             32, 32, dataColor);
  pos -= sp;

  // status
  dataColor = ACCENT_COLOR;
  if (!statusStr.isEmpty())
  {
    drawString(pos, DISP_HEIGHT - 2 - 2, statusStr, RIGHT, dataColor);
    pos -= getStringWidth(statusStr) + 24;
    display.drawInvertedBitmap(pos, DISP_HEIGHT - 2 - 18, error_icon_24x24,
                               24, 24, dataColor);
  }

  return;
} // end drawStatusBar

/* This function is responsible for drawing prominent error messages to the
 * screen.
 */
void drawError(const uint8_t *bitmap_196x196,
               const String &errMsgLn1, const String &errMsgLn2)
{
  display.setFont(&FONT_26pt8b);
  drawString(DISP_WIDTH / 2,
             DISP_HEIGHT / 2 + 196 / 2 + 21,
             errMsgLn1, CENTER);
  drawString(DISP_WIDTH / 2,
             DISP_HEIGHT / 2 + 196 / 2 + 76,
             errMsgLn2, CENTER);
  display.drawInvertedBitmap(DISP_WIDTH / 2 - 196 / 2,
                             DISP_HEIGHT / 2 - 196 / 2 - 21,
                             bitmap_196x196, 196, 196, ACCENT_COLOR);
  return;
} // end drawError

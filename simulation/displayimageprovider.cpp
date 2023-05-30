#include "displayimageprovider.h"

#include "display_utils.h"
#include "renderer.h"
#include "FreeSans.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <widgets.h>

owm_resp_onecall_t parseOneCallResponse(const QJsonDocument &doc)
{
    auto current = doc["current"].toObject();
    auto current_weather = current["weather"].toArray()[0].toObject();

    owm_resp_onecall_t
        r{.lat = static_cast<float>(doc["lat"].toDouble()),
          .lon = static_cast<float>(doc["lon"].toDouble()),
          .timezone = doc["timezone"].toString(),
          .timezone_offset = doc["timezone_offset"].toInt(),
          .current = {.dt = current["dt"].toInteger(),
                      .sunrise = current["sunrise"].toInteger(),
                      .sunset = current["sunset"].toInteger(),
                      .temp = static_cast<float>(current["temp"].toDouble()),
                      .feels_like = static_cast<float>(current["feels_like"].toDouble()),
                      .pressure = current["pressure"].toInt(),
                      .humidity = current["humidity"].toInt(),
                      .dew_point = static_cast<float>(current["dew_point"].toDouble()),
                      .clouds = current["clouds"].toInt(),
                      .uvi = static_cast<float>(current["uvi"].toDouble()),
                      .visibility = static_cast<float>(current["visibility"].toInt()),
                      .wind_speed = static_cast<float>(current["wind_speed"].toDouble()),
                      .wind_gust = static_cast<float>(current["wind_gust"].toDouble()),
                      .wind_deg = current["wind_deg"].toInt(),
                      .rain_1h = static_cast<float>(current["rain"].toObject()["1h"].toDouble()),
                      .snow_1h = static_cast<float>(current["snow"].toObject()["1h"].toDouble()),
                      .weather = {
                          .id = current_weather["id"].toInt(),
                          .main = current_weather["main"].toString(),
                          .description = current_weather["description"].toString(),
                          .icon = current_weather["icon"].toString(),
                      }}};

    int i = 0;
    for (const auto &json : doc["hourly"].toArray()) {
        auto hourly = json.toObject();
        r.hourly[i] = {
            .dt = hourly["dt"].toInteger(),
            .temp = static_cast<float>(hourly["temp"].toDouble()),
            .feels_like = static_cast<float>(hourly["feels_like"].toDouble()),
            .pressure = hourly["pressure"].toInt(),
            .humidity = hourly["humidity"].toInt(),
            .dew_point = static_cast<float>(hourly["dew_point"].toDouble()),
            .clouds = hourly["clouds"].toInt(),
            .uvi = static_cast<float>(hourly["uvi"].toDouble()),
            .visibility = static_cast<float>(hourly["visibility"].toInt()),
            .wind_speed = static_cast<float>(hourly["wind_speed"].toDouble()),
            .wind_gust = static_cast<float>(hourly["wind_gust"].toDouble()),
            .wind_deg = hourly["wind_deg"].toInt(),
            .pop = static_cast<float>(hourly["pop"].toDouble()),
            .rain_1h = static_cast<float>(hourly["rain"].toObject()["1h"].toDouble()),
            .snow_1h = static_cast<float>(hourly["snow"].toObject()["1h"].toDouble()),
        };
        if (i++ == OWM_NUM_HOURLY)
            break;
    }

    i = 0;
    for (const auto &json : doc["daily"].toArray()) {
        auto daily = json.toObject();
        auto daily_temp = daily["temp"].toObject();
        auto daily_feels_like = daily["feels_like"].toObject();
        auto daily_weather = daily["weather"][0].toObject();
        r.daily[i] = {
            .dt = daily["dt"].toInteger(),
            .sunrise = daily["sunrise"].toInteger(),
            .sunset = daily["sunset"].toInteger(),
            .moonrise = daily["moonrise"].toInteger(),
            .moonset = daily["moonset"].toInteger(),
            .moon_phase = static_cast<float>(daily["moon_phase"].toDouble()),
            .temp = {
                .morn = {static_cast<float>(daily_temp["morn"].toDouble())},
                .day = {static_cast<float>(daily_temp["day"].toDouble())},
                .eve = {static_cast<float>(daily_temp["eve"].toDouble())},
                .night = {static_cast<float>(daily_temp["night"].toDouble())},
                .min = {static_cast<float>(daily_temp["min"].toDouble())},
                .max = {static_cast<float>(daily_temp["max"].toDouble())},
            },
            .feels_like = {
                .morn = static_cast<float>(daily_feels_like["morn"].toDouble()),
                .day = static_cast<float>(daily_feels_like["day"].toDouble()),
                .eve = static_cast<float>(daily_feels_like["eve"].toDouble()),
                .night = static_cast<float>(daily_feels_like["night"].toDouble()),
            },
            .pressure = daily["pressure"].toInt(),
            .humidity = daily["humidity"].toInt(),
            .dew_point = static_cast<float>(daily["dew_point"].toDouble()),
            .clouds = daily["clouds"].toInt(),
            .uvi = static_cast<float>(daily["uvi"].toDouble()),
            .visibility = daily["visibility"].toInt(),
            .wind_speed = static_cast<float>(daily["wind_speed"].toDouble()),
            .wind_gust = static_cast<float>(daily["wind_gust"].toDouble()),
            .wind_deg = daily["wind_deg"].toInt(),
            .pop = static_cast<float>(daily["pop"].toDouble()),
            .rain = static_cast<float>(daily["rain"].toDouble()),
            .snow = static_cast<float>(daily["snow"].toDouble()),
            .weather = {
                .id = daily_weather["id"].toInt(),
                .main = daily_weather["main"].toString(),
                .description = daily_weather["description"].toString(),
                .icon = daily_weather["icon"].toString(),
            },
        };

        if (i++ == OWM_NUM_DAILY)
            break;
    }

    return r;
}

DisplayImageProvider::DisplayImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{}

QImage DisplayImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QNetworkAccessManager nam;
    QNetworkRequest req(QUrl("https://api.openweathermap.org/data/3.0/onecall?"
                             "lat=50.81810&"
                             "lon=6.11596&"
                             "units=standard&"
                             "exclude=minutely&"
                             "appid=c6a12eb0a93c51a3c21d1af76043288f&"
                             "lang=DE"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    qDebug() << "Requesting data";
    auto reply = nam.get(req);
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    QTimer timer;
    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, &eventLoop, [&eventLoop] { eventLoop.exit(1); });
    timer.start();
    if (eventLoop.exec() == 1)
        qCritical() << "Request timed out";
    else
        qDebug() << "Request finished" << reply->error();

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError)
        qCritical() << "error parsing JSON response" << error.errorString();

    auto owm_onecall = parseOneCallResponse(doc);
    owm_resp_air_pollution_t owm_air_pollution{};

    time_t rawtime;
    time(&rawtime);
    auto timeInfo = localtime(&rawtime);

    display.clear();

    String refreshTimeStr;
    getRefreshTimeStr(refreshTimeStr, true, timeInfo);

    String statusStr;
    double wifiRSSI = -45;
    double batteryVoltage = 3.9;
    Quantity<Kelvin> inTemp = 293;
    float inHumidity = 56;

    String dateStr;
    getDateStr(dateStr, timeInfo);

#if 1
    W::Font mediumSizedFont{.gfxFont = FONT_16pt8b, .ascent = 15, .descent = 2};
    W::Window w{800, 480};
    W::Text t1;
    w << t1.text("Hagen").font(mediumSizedFont);

    W::DisplayPainter painter(display);
    w.paint(painter);
#else
    drawCurrentConditions(owm_onecall.current,
                          owm_onecall.daily[0],
                          owm_air_pollution,
                          inTemp,
                          inHumidity);
    drawForecast(owm_onecall.daily, *timeInfo);
    drawLocationDate(CITY_STRING, dateStr);
    drawOutlookGraph(owm_onecall.hourly, *timeInfo);
    drawAlerts(owm_onecall.alerts, CITY_STRING, dateStr);
    drawStatusBar(statusStr, refreshTimeStr, wifiRSSI, batteryVoltage);
#endif
    auto image = display.image();
    *size = image.size();
    return image;
}

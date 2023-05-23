#pragma once

#include <QQuickImageProvider>

class DisplayImageProvider : public QQuickImageProvider
{
public:
    DisplayImageProvider();

    // QQuickImageProvider interface
public:
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
};


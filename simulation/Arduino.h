#ifndef ARDUINO_H
#define ARDUINO_H

#include <QString>
#include <cmath>

using std::max, std::round;

class String
{
public:
    String() {}

    String(char c)
        : s(QChar::fromLatin1(c))
    {}

    String(float value)
        : s(QString::number(value))
    {}

    String(float value, int decimals)
        : s(QString::number(value, 'f', decimals))
    {}

    String(double value)
        : s(QString::number(value))
    {}

    String(double value, int decimals)
        : s(QString::number(value, 'f', decimals))
    {}

    String(int value)
        : s(QString::number(value))
    {}

    String(unsigned int value)
        : s(QString::number(value))
    {}

    String(const QString &src)
        : s(src)
    {}

    String(const char *src)
        : s(QString::fromLatin1(src))
    {}

    String &operator+=(const char *a)
    {
        s.append(QString::fromLatin1(a));
        return *this;
    }

    String &operator+=(const String &o)
    {
        s.append(o.s);
        return *this;
    }

    String replace(const char *find, const char *replace)
    {
        s.replace(QString::fromLatin1(find), QString::fromLatin1(replace));
        return *this;
    }

    int length() const { return s.length(); }

    const char *c_str() const
    {
        b = s.toLatin1();
        return b.constData();
    }

    bool endsWith(const char *suffix) { return s.endsWith(QString::fromLatin1(suffix)); }

    bool isEmpty() const { return s.isEmpty(); }

    void remove(qsizetype pos, qsizetype n) { s.remove(pos, n); }

    void remove(qsizetype pos) { s.remove(pos, length()); }

    int lastIndexOf(const char *substr) const { return s.lastIndexOf(QString::fromLatin1(substr)); }

    String substring(qsizetype left, qsizetype right) const
    {
        if (left > right) {
            unsigned int temp = right;
            right = left;
            left = temp;
        }
        if (left >= length())
            return {};
        if (right > length())
            right = length();
        return s.sliced(left, right - left);
    }

    String substring(qsizetype begin) const { return substring(begin, length()); }

    char charAt(qsizetype pos) const { return s[pos].toLatin1(); }

    void setCharAt(unsigned int loc, char c) { s[loc] = QChar::fromLatin1(c); }

    int indexOf(char c) const { return s.indexOf(QChar::fromLatin1(c)); }
    int indexOf(const String &s2) const { return s.indexOf(s2.s); }

    void toLowerCase() { s = s.toLower(); }

    bool operator==(const String &rhs) const { return s == rhs.s; }

private:
    QString s;
    mutable QByteArray b;
};

inline String operator+(String &lhs, const String &rhs)
{
    return lhs += rhs;
}

inline String operator+(String &&lhs, const String &rhs)
{
    return std::move(lhs += rhs);
}

inline String operator+(String &&lhs, const char *rhs)
{
    return std::move(lhs += rhs);
}

inline String operator+(const char *lhs, const String &rhs)
{
    String t(lhs);
    t += rhs;
    return t;
}

inline int toUpperCase(int c)
{
    return toupper(c);
}

inline int toLowerCase(int c)
{
    return tolower(c);
}

#define PROGMEM
#define A0 0

#endif // ARDUINO_H

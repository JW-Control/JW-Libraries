// Mock minimo de Arduino para pruebas N1 de JW_DWIN en host (g++ -std=c++11).
#ifndef JW_DWIN_TEST_MOCK_ARDUINO_H
#define JW_DWIN_TEST_MOCK_ARDUINO_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>

extern uint32_t g_mockMs;
inline uint32_t millis() { return g_mockMs; }
inline uint32_t micros() { return g_mockMs * 1000UL; }
inline void delay(uint32_t ms) { g_mockMs += ms; }

class String
{
public:
    String() {}
    String(const char *s) : _s(s ? s : "") {}
    unsigned length() const { return (unsigned)_s.size(); }
    const char *c_str() const { return _s.c_str(); }
    void remove(unsigned index) { if (index < _s.size()) _s.erase(index); }
    String &operator+=(char c) { _s += c; return *this; }
    bool operator==(const String &o) const { return _s == o._s; }
    bool operator!=(const String &o) const { return _s != o._s; }

private:
    std::string _s;
};

class Stream
{
public:
    std::string out;
    int printf(const char *fmt, ...)
    {
        char b[256];
        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(b, sizeof(b), fmt, ap);
        va_end(ap);
        out += b;
        return n;
    }
    void print(const char *s) { out += s; }
    void print(char c) { out += c; }
    void print(uint32_t v) { printf("%lu", (unsigned long)v); }
    void println() { out += "\n"; }
    void println(const char *s) { out += s; out += "\n"; }
    void println(float v, int d) { printf("%.*f\n", d, v); }
};
extern Stream Serial;

#endif

#pragma once
#if 1
    void Logger(const char *fmt...);
    void Logger(int val);
#else
    #define Logger(...) {}
#endif²

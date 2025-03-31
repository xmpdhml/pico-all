#pragma once

class KeyScan
{
    friend class System;
public:
    void init();
    void scan();
private:
    KeyScan() = default;
    ~KeyScan() = default;
    KeyScan(const KeyScan&) = delete;
    KeyScan& operator=(const KeyScan&) = delete;
    KeyScan(KeyScan&&) = delete;
    KeyScan& operator=(KeyScan&&) = delete;    
};

#ifndef WINDOWS_TIMER_RESOLUTION_H
#define WINDOWS_TIMER_RESOLUTION_H

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>

class WindowsTimerResolutionGuard
{
public:
    explicit WindowsTimerResolutionGuard(UINT periodMs = 1)
        : winmm_(NULL),
          timeBeginPeriod_(NULL),
          timeEndPeriod_(NULL),
          periodMs_(periodMs),
          active_(false)
    {
        winmm_ = LoadLibraryA("winmm.dll");
        if (!winmm_) {
            return;
        }

        timeBeginPeriod_ = reinterpret_cast<TimePeriodFunc>(
            GetProcAddress(winmm_, "timeBeginPeriod"));
        timeEndPeriod_ = reinterpret_cast<TimePeriodFunc>(
            GetProcAddress(winmm_, "timeEndPeriod"));

        if (timeBeginPeriod_ && timeEndPeriod_ && timeBeginPeriod_(periodMs_) == 0) {
            active_ = true;
        }
    }

    ~WindowsTimerResolutionGuard()
    {
        if (active_ && timeEndPeriod_) {
            timeEndPeriod_(periodMs_);
        }
        if (winmm_) {
            FreeLibrary(winmm_);
        }
    }

    bool isActive() const
    {
        return active_;
    }

private:
    typedef UINT (WINAPI *TimePeriodFunc)(UINT);

    HMODULE winmm_;
    TimePeriodFunc timeBeginPeriod_;
    TimePeriodFunc timeEndPeriod_;
    UINT periodMs_;
    bool active_;

    Q_DISABLE_COPY(WindowsTimerResolutionGuard)
};
#else
class WindowsTimerResolutionGuard
{
public:
    explicit WindowsTimerResolutionGuard(unsigned int = 1) {}
    bool isActive() const { return false; }
};
#endif

#endif // WINDOWS_TIMER_RESOLUTION_H

#ifndef HOPPER_TIMER_HPP
#define HOPPER_TIMER_HPP

#include <stdint.h>
#include <Arduino.h>

class HopperTimer
{
    public:
        HopperTimer(uint32_t periodMs)
        {
            periodMs_ = periodMs;
        }
        ~HopperTimer(){}

        void start()
        {
            startTime_ = millis();
            isRunning_ = true;
        }

        void stop()
        {
            isRunning_ = false;
        }

        bool hasPeriodPassed()
        {
            if (!isRunning_) return false;

            if ((millis() - startTime_) >= periodMs_)
            {
                isRunning_ = false;
                return true;
            }

            return false;
        }

        bool isRunning(){ return isRunning_; }

    private:
        uint32_t startTime_ = 0;
        uint32_t periodMs_ = 0;
        bool isRunning_ = false;
};

#endif
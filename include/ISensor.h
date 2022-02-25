#pragma once

namespace interfaces
{
    class ISensor
    {
    public:
        virtual bool init() = 0;
        virtual bool measure() = 0;
        virtual bool newValue() const = 0;
        virtual float getValue() = 0;
        virtual String getName() const = 0;
    };
} // namespace interfaces
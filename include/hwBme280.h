#pragma once

#include <Adafruit_BME280.h>

namespace wrappers
{
    class HwBme280
    {
    private:
        Adafruit_BME280 bme;
        bool is_init = false;

        float temperature = 0.0F;
        float humidity = 0.0F;
        float pressure_raw = 0.0F;
        bool new_temperature = false;
        bool new_humidity = false;
        bool new_pressure_raw = false;

    public:
        bool init(uint8_t address = BME280_ADDRESS_ALTERNATE);
        bool isInit() { return is_init; };
        bool measure();
        float getTemperature();
        bool isNewTemperature() { return new_temperature; };
        float getHumidity();
        bool isNewHumidity() { return humidity; }
        float getPressureRaw();
        bool isNewPressureRaw() { return pressure_raw; }
        HwBme280(){};
    };

    bool HwBme280::init(uint8_t address)
    {
        this->is_init = false;

        if (!this->bme.begin(address))
        {
            is_init = false;
        }
        else
        {
            bme.setSampling(Adafruit_BME280::MODE_FORCED, Adafruit_BME280::SAMPLING_X16,
                            Adafruit_BME280::SAMPLING_X16, Adafruit_BME280::SAMPLING_X16,
                            Adafruit_BME280::FILTER_OFF, Adafruit_BME280::STANDBY_MS_0_5);
            this->is_init = true;
        }

        return this->is_init;
    }

    bool HwBme280::measure()
    {
        bool if_measured = false;
        this->new_temperature = false;
        this->new_humidity = false;
        this->new_pressure_raw = false;

        if (this->is_init && this->bme.takeForcedMeasurement())
        {
            this->temperature = bme.readTemperature();
            this->new_temperature = true;
            this->humidity = bme.readHumidity();
            this->new_humidity = true;
            this->pressure_raw = bme.readPressure() / 100.0F;
            this->new_pressure_raw = true;
            if_measured = true;
        }

        return if_measured;
    }

    float HwBme280::getTemperature()
    {
        this->new_temperature = false;
        return this->is_init ? this->temperature : 0.0F;
    };

    float HwBme280::getHumidity()
    {
        this->new_humidity = false;
        return this->is_init ? this->humidity : 0.0F;
    }

    float HwBme280::getPressureRaw()
    {
        this->new_pressure_raw = false;
        return this->is_init ? this->pressure_raw : 0.0F;
    }
} // namespace wrappers
#ifndef BME280_H
#define BME280_H

#include <QtGlobal>
#include "qi2cbus.h"

#define BME280_ID   0x60
#define BMP280_ID   0x58



//Nom des registres :
#define BME280_REGISTER_DIG_T1  0x88
#define BME280_REGISTER_DIG_T2  0x8A
#define BME280_REGISTER_DIG_T3  0x8C

#define BME280_REGISTER_DIG_P1  0x8E
#define BME280_REGISTER_DIG_P2  0x90
#define BME280_REGISTER_DIG_P3  0x92
#define BME280_REGISTER_DIG_P4  0x94
#define BME280_REGISTER_DIG_P5  0x96
#define BME280_REGISTER_DIG_P6  0x98
#define BME280_REGISTER_DIG_P7  0x9A
#define BME280_REGISTER_DIG_P8  0x9C
#define BME280_REGISTER_DIG_P9  0x9E

#define BME280_REGISTER_DIG_H1  0xA1
#define BME280_REGISTER_DIG_H2  0xE1
#define BME280_REGISTER_DIG_H3  0xE3
#define BME280_REGISTER_DIG_H4  0xE4
#define BME280_REGISTER_DIG_H5  0xE5
#define BME280_REGISTER_DIG_H6  0xE7

#define BME280_CHIP_ID_REG		    0xD0 //Chip ID
#define BME280_RST_REG			    0xE0 //Softreset Reg
#define BME280_CTRL_HUMIDITY_REG    0xF2 //Ctrl Humidity Reg
#define BME280_STAT_REG             0xF3 //Status Reg
#define BME280_CTRL_MEAS_REG        0xF4 //Ctrl Measure Reg
#define BME280_CONFIG_REG           0xF5 //Configuration Reg
#define BME280_PRESSURE_MSB_REG     0xF7 //Pressure MSB
#define BME280_PRESSURE_LSB_REG     0xF8 //Pressure LSB
#define BME280_PRESSURE_XLSB_REG    0xF9 //Pressure XLSB
#define BME280_TEMPERATURE_MSB_REG  0xFA //Temperature MSB
#define BME280_TEMPERATURE_LSB_REG  0xFB //Temperature LSB
#define BME280_TEMPERATURE_XLSB_REG 0xFC //Temperature XLSB
#define BME280_HUMIDITY_MSB_REG     0xFD //Humidity MSB
#define BME280_HUMIDITY_LSB_REG     0xFE //Humidity LSB

class BME280 {
public:
    enum sensor_mode {
                MODE_SLEEP  = 0b00,
                MODE_FORCED = 0b01,
                MODE_NORMAL = 0b11
    };

    enum sensor_sampling {
                SAMPLING_NONE = 0b000,
                SAMPLING_X1   = 0b001,
                SAMPLING_X2   = 0b010,
                SAMPLING_X4   = 0b011,
                SAMPLING_X8   = 0b100,
                SAMPLING_X16  = 0b101
    };

    enum sensor_filter {
                FILTER_OFF = 0b000,
                FILTER_X2  = 0b001,
                FILTER_X4  = 0b010,
                FILTER_X8  = 0b011,
                FILTER_X16 = 0b100
    };

    enum standby_duration {
                STANDBY_MS_0_5  = 0b000,
                STANDBY_MS_10   = 0b110,
                STANDBY_MS_20   = 0b111,
                STANDBY_MS_62_5 = 0b001,
                STANDBY_MS_125  = 0b010,
                STANDBY_MS_250  = 0b011,
                STANDBY_MS_500  = 0b100,
                STANDBY_MS_1000 = 0b101
    };

    BME280(Qi2cBus *busComm, const quint8 _I2CAdress = 0x77);
    virtual ~BME280();

    void Calibrer();
    bool CalibrationEnCourt();
    void Reset();

    void FixerMode(sensor_mode mode = MODE_NORMAL);
    sensor_mode ObtenirMode();

    void Configurer(sensor_sampling temperature = SAMPLING_X1,
                    sensor_sampling humidite = SAMPLING_X1,
                    sensor_sampling pression = SAMPLING_X1,
                    sensor_filter filtre = FILTER_OFF);

    float LireTemperatureC();
    float LireHumiditeRelative();
    float LirePression();

    float CalculerPointDeRosee();
    float CalculerPointDeGivrage();

    void  Version();


private:

    //Main Interface and mode settings
    Qi2cBus *commInterface;
    quint8 I2CAddress;

    // Données de calibration
    quint16 dig_T1;
    qint16  dig_T2;
    qint16  dig_T3;

    quint16 dig_P1;
    qint16  dig_P2;
    qint16  dig_P3;
    qint16  dig_P4;
    qint16  dig_P5;
    qint16  dig_P6;
    qint16  dig_P7;
    qint16  dig_P8;
    qint16  dig_P9;

    quint8  dig_H1;
    qint16  dig_H2;
    quint8  dig_H3;
    qint16  dig_H4;
    qint16  dig_H5;
    qint8   dig_H6;

    // Valeur de la température
    qint32 t_fine;


};


#endif // BME280_H

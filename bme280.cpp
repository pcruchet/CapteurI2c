#include "bme280.h"
#include <QDebug>

BME280::BME280(Qi2cBus *busComm, const quint8 _I2CAdress) {

    quint8 composantID;
    bool composantOk = false;

    commInterface = busComm;
    I2CAddress = _I2CAdress; //Default, jumper open is 0x77

    usleep(2000); // delay de 2ms pour laiser le temps au capteur de démarrer

    commInterface->CommencerTransmission(I2CAddress);
    composantID = commInterface->LireRegistre(BME280_CHIP_ID_REG);

    if (composantID == BMP280_ID || composantID == BME280_ID)
        composantOk = true;
    else
        qDebug() << "Le composant n'est pas présent" ;

    commInterface->TerminerTransmission();



    if (composantOk)
    {
        while(CalibrationEnCourt())
            usleep(1000);

        Calibrer();
        Configurer();
        usleep(3000);
    }
}

BME280::~BME280() {
}

void BME280::Calibrer() {

    commInterface->CommencerTransmission(I2CAddress);
    dig_T1 = commInterface->LireRegistre16(BME280_REGISTER_DIG_T1);
    dig_T2 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_T2);
    dig_T3 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_T3);

    dig_P1 = commInterface->LireRegistre16(BME280_REGISTER_DIG_P1);
    dig_P2 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_P2);
    dig_P3 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_P3);
    dig_P4 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_P4);
    dig_P5 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_P5);
    dig_P6 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_P6);
    dig_P7 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_P7);
    dig_P8 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_P8);
    dig_P9 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_P9);

    dig_H1 = commInterface->LireRegistre(BME280_REGISTER_DIG_H1);
    dig_H2 = (qint16)commInterface->LireRegistre16(BME280_REGISTER_DIG_H2);
    dig_H3 = commInterface->LireRegistre(BME280_REGISTER_DIG_H3);
    dig_H4 = (commInterface->LireRegistre(BME280_REGISTER_DIG_H4) << 4) | (commInterface->LireRegistre(BME280_REGISTER_DIG_H4+1) & 0xF);
    dig_H5 = (commInterface->LireRegistre(BME280_REGISTER_DIG_H5+1) << 4) | (commInterface->LireRegistre(BME280_REGISTER_DIG_H5) >> 4);
    dig_H6 = (qint8)commInterface->LireRegistre(BME280_REGISTER_DIG_H6);
    commInterface->TerminerTransmission();
}

void BME280::FixerMode(BME280::sensor_mode mode) {

    commInterface->CommencerTransmission(I2CAddress);
    quint8 controlData = commInterface->LireRegistre(BME280_CTRL_MEAS_REG);
    controlData &= ~(MODE_NORMAL); // Remise à 0 des 2 premiers bits
    controlData |= mode;
    commInterface->EcrireRegistre(BME280_CTRL_MEAS_REG, controlData);
    commInterface->TerminerTransmission();

}

BME280::sensor_mode BME280::ObtenirMode()
{
    commInterface->CommencerTransmission(I2CAddress);
    quint8 controlData = commInterface->LireRegistre(BME280_CTRL_MEAS_REG) & 0b00000011;
    commInterface->TerminerTransmission();
    return (sensor_mode) controlData ;
}

void BME280::Configurer(BME280::sensor_sampling temperature,
                        BME280::sensor_sampling humidite,
                        BME280::sensor_sampling pression,
                        BME280::sensor_filter filtre)
{
    commInterface->CommencerTransmission(I2CAddress);
    commInterface->EcrireRegistre(BME280_CTRL_HUMIDITY_REG, humidite);

    quint8 configData = commInterface->LireRegistre(BME280_CONFIG_REG);
    configData &= ~( (1<<4) | (1<<3) | (1<<2) ); //remise à 0 des bits 4/3/2
    configData |= (filtre << 2); //Alignement des bits 4/3/2
    commInterface->EcrireRegistre(BME280_CONFIG_REG, configData);
    quint8 controlData = temperature << 5 | pression << 2 | MODE_NORMAL ;
    commInterface->EcrireRegistre(BME280_CTRL_MEAS_REG,controlData);

    commInterface->TerminerTransmission();
}

float BME280::LireTemperatureC() {
    quint8 buffer[3];
    quint32 adc_T;
    float sortie = 0.0;

    commInterface->CommencerTransmission(I2CAddress);
    if (commInterface->LireBlocRegistres(BME280_TEMPERATURE_MSB_REG, buffer, 3) == 3) {
        adc_T = ((quint32) buffer[0] << 12) | ((quint32) buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);

        qint64 var1, var2;
        var1 = ((((adc_T >> 3) - ((quint32) dig_T1 << 1))) * ((quint32) dig_T2)) >> 11;
        var2 = (((((adc_T >> 4) - ((quint32) dig_T1)) * ((adc_T >> 4) - ((quint32) dig_T1))) >> 12) *
                ((quint32) dig_T3)) >> 14;
        t_fine = var1 + var2;

        sortie = (t_fine * 5 + 128) >> 8;
        sortie /= 100.0;
    }
    commInterface->TerminerTransmission();


    return sortie;
}

float BME280::LireHumiditeRelative() {
    quint8 buffer[2];
    quint32 var1 = 0.0;

    commInterface->CommencerTransmission(I2CAddress);
    if (commInterface->LireBlocRegistres(BME280_HUMIDITY_MSB_REG, buffer, 2) == 2) {

        quint32 adc_H = ((quint32) buffer[0] << 8) | ((quint32) buffer[1]);

        var1 = (t_fine - ((quint32) 76800));
        var1 = (((((adc_H << 14) - (((quint32) dig_H4) << 20) - (((quint32) dig_H5) * var1)) +
                  ((quint32) 16384)) >> 15) * (((((((var1 * ((quint32) dig_H6)) >> 10) *
                                                   (((var1 * ((quint32) dig_H3)) >> 11) + ((quint32) 32768))) >> 10) + ((quint32) 2097152)) *
                                                ((quint32) dig_H2) + 8192) >> 14));
        var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((quint32) dig_H1)) >> 4));
        var1 = (var1 < 0.0 ? 0 : var1);
        var1 = (var1 > 419430400 ? 419430400 : var1);
    }
    commInterface->TerminerTransmission();


    return (float) (var1 >> 12) / 1024.0;
}

float BME280::LirePression() {
    quint8 buffer[3];
    float sortie = 0.0;

    commInterface->CommencerTransmission(I2CAddress);
    if (commInterface->LireBlocRegistres(BME280_PRESSURE_MSB_REG, buffer, 3) == 3) {
        quint32 adc_P = ((quint32) buffer[0] << 12) | ((quint32) buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);

        qint64 var1, var2, p_acc;
        var1 = ((qint64) t_fine) - 128000;
        var2 = var1 * var1 * (qint64) dig_P6;
        var2 = var2 + ((var1 * (qint64) dig_P5) << 17);
        var2 = var2 + (((qint64) dig_P4) << 35);
        var1 = ((var1 * var1 * (qint64) dig_P3) >> 8) + ((var1 * (qint64) dig_P2) << 12);
        var1 = (((((qint64) 1) << 47) + var1))*((qint64) dig_P1) >> 33;
        if (var1 == 0)
            sortie = 0.0;
        else {
            p_acc = 1048576 - adc_P;
            p_acc = (((p_acc << 31) - var2)*3125) / var1;
            var1 = (((qint64) dig_P9) * (p_acc >> 13) * (p_acc >> 13)) >> 25;
            var2 = (((qint64) dig_P8) * p_acc) >> 19;
            p_acc = ((p_acc + var1 + var2) >> 8) + (((qint64) dig_P7) << 4);
            sortie = p_acc / 25600.0;
        }
        commInterface->TerminerTransmission();
    }

    return sortie;
}

/**
 * @brief BME280::CalculerPointDeRosee
 * @return  Valeur de la température du point de rosée
 * @details La temperature de rosée est la température la plus basse
 *          à laquelle une masse d'air peut etre soumise,
 *          à pression et humidité données, sans qu'il ne se produise
 *          une formation de condensation.
 *
 *          Formule de Heinrich Gustav Magnus-Tetens
 */
float BME280::CalculerPointDeRosee() {

    float tAir = LireTemperatureC();
    float hRelative = LireHumiditeRelative()/100.0 ;

    float a = 17.27;
    float b = 237.7;
    float z1 = (a*tAir)/(b+tAir) + log (hRelative);
    return b*z1 / (a - z1);
}

/**
 * @brief BME280::CalculerPointDeGivrage
 * @return valeur de la température du point de gelée
 * @details Le point de givrage ou point de gelée représente
 *          tout en gardant inchangées les conditions barométriques courantes,
 *          l'air devient saturé de vapeur d'eau par rapport à la glace.
 *          C'est la température ou se produit le phénomène de déposition
 *          qui créé la gelée blanche.
 *
 *          Applicable uniquement lorsque la température est sous le point
 *          de congélation soit 0 °C.
 */
float BME280::CalculerPointDeGivrage()
{
    float tAir = LireTemperatureC() + 273.15 ; // exprimé en Kelvin
    float z1 = (2954.61 / tAir) + 2.193665 * log (tAir) - 13.3448 ;
    float z2 = (CalculerPointDeRosee() + 273.15) - tAir ;
    return (z2 + 2671.02 / z1 - 273.15); // Converti en °C
}


void BME280::Reset()
{
    commInterface->CommencerTransmission(I2CAddress);
    commInterface->EcrireRegistre(BME280_RST_REG, 0xB6);
    commInterface->TerminerTransmission();

}

bool BME280::CalibrationEnCourt()
{
    commInterface->CommencerTransmission(I2CAddress);
    quint8 status = commInterface->LireRegistre(BME280_STAT_REG);
    commInterface->TerminerTransmission();

    return (status & (1<<0)) != 0 ;
}

void BME280::Version() {

    qDebug() << "\nBME280 PSR - PCT 2018 Version 1.5\n" ;

}

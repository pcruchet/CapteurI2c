#include <QCoreApplication>
#include "qi2cbus.h"
#include "bme280.h"

#include <iostream>
#include <iomanip>
#include <unistd.h>
using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Qi2cBus busI2c("/dev/i2c-1");
    BME280 sensor(&busI2c, 0x77);

    while(1)
    {
        cout <<fixed << setprecision(1);
        cout << "Température : " << sensor.LireTemperatureC() << " °C " << endl;
        cout << "Pression : " << sensor.LirePression() << " hPa " << endl;
        cout << "Humidité relative : " << sensor.LireHumiditeRelative() << " % " << endl;
        cout << "Point de rosée : " << sensor.CalculerPointDeRosee() << " °C " << endl;
        cout << "Point de givrage : " << sensor.CalculerPointDeGivrage() << " °C" << endl;

        sleep(5);
    }

    return a.exec();
}

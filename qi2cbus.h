/**
 * @file    qi2cbus.h
 * @brief   Classe pour gérer le bus i2C avec le framework QT
 * @author  Philippe CRUCHET (LPO Touchard-Washington - Le MANS)
 * @date    15 aout 2018
 * @version 1.0
 */

#ifndef QI2CBUS_H
#define QI2CBUS_H

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>

#include <QObject>
#include <QMutex>

class Qi2cBus : public QObject
{
    Q_OBJECT
public:
    explicit Qi2cBus(QString _i2cDev = "/dev/i2c-1", QObject *_parent = nullptr);
    virtual ~Qi2cBus();

    void CommencerTransmission(uint8_t _adresse);
    void TerminerTransmission();
    quint8 LireRegistre(quint8 _registre);
    int EcrireRegistre(quint8 _registre, quint8 _valeur);
    int LireBlocRegistres(quint8 _registre, quint8 *_valeurs , quint8 _taille);
    quint16 LireRegistre16(quint8 _registre);

private:  
    QString i2cDev;         /// Nom du fichier vers le bus I2c
    int fichierI2c = 0;     /// Descripteur de fichier
    QMutex mutex;           /// Mutex pour bloquer l'accès au bus sur le fichier désigné

    int i2c_smbus_access(char _mode, quint8 _registre, int _taille, union i2c_smbus_data *_data) ;
};

#endif // QI2CBUS_H


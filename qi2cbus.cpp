/**
 * @file    qi2cbus.cpp
 * @brief   Classe pour gérer le bus i2C avec le framework QT
 * @author  Philippe CRUCHET (LPO Touchard-Washington - Le MANS)
 * @date    15 aout 2018
 * @version 1.0
 */

#include "qi2cbus.h"
#include "capteurexception.h"

#include <QDebug>

/**
 * @brief Qi2cBus::Qi2cBus
 * @param _i2cDev   Nom du fichier désignant le bus i2c (/dev/i2c-X avec X = 0,1,2...)
 * @param _parent   Pointeur vers l'objet parent
 *
 * @details Ouvre le fichier en question
 */
Qi2cBus::Qi2cBus(QString _i2cDev, QObject *_parent) :
    QObject(_parent),
    i2cDev(_i2cDev)
{
    try
    {
        if ((fichierI2c = open(i2cDev.toLocal8Bit(), O_RDWR)) < 0)
            throw CapteurException(errno, " Erreur d'ouverture de " + i2cDev);
    }
    catch (CapteurException const& e)
    {
        qDebug() << e.ObtenirErreur();
        exit(errno);
    }
}

/**
 * @brief Qi2cBus::CommencerTransmission
 * @param _adresse  Adresse du composant
 *
 * @details Fonction bloquante, Permet de commencer une transmission sur le bus i2c.
 *
 */
void Qi2cBus::CommencerTransmission(quint8 _adresse)
{
    mutex.lock();
    try
    {
        if (ioctl(fichierI2c, I2C_SLAVE, _adresse) < 0)
            throw CapteurException(errno, " Erreur affectation adresse " + QString::number(_adresse));
    }
    catch (CapteurException const& e)
    {
        qDebug() << e.ObtenirErreur();
        exit(errno);
    }
}

/**
 * @brief Qi2cBus::TerminerTransmission
 *
 * @details Libère le bus I2c pour l'utilisation par un autre capteur.
 */
void Qi2cBus::TerminerTransmission()
{
    mutex.unlock();
}

/**
 * @brief Qi2cBus::~Qi2cBus
 *
 * @details Destructeur de la classe ferme le fichier
 */
Qi2cBus::~Qi2cBus()
{
    close(fichierI2c);
}

/**
 * @brief Qi2cBus::LireRegistre
 * @param  _registre  Adresse du registre à lire dans le composant.
 * @return            Valeur du registre sous la forme d'un octet non signé.
 *
 * @details Lit la valeur du registre passé en paramètre. L'appel de la méthode
 *          CommencerTransmission(quint8 registre) est nécessaire avant pour
 *          désigner l'adresse du composant et prendre le bus pour l'échange.
 *          Plusieurs lecture sont possibles sur le meme composant avant de libérer
 *          le bus I2c avec la méthode TerminerTransmission().
 */
quint8 Qi2cBus::LireRegistre(quint8 _registre)
{
    union i2c_smbus_data data;

    try {
        if (i2c_smbus_access(I2C_SMBUS_READ, _registre, I2C_SMBUS_BYTE_DATA, &data) < 0)
            throw CapteurException(errno, " Erreur Lecture registre 8 bits n° " + QString::number(_registre));
    } catch (CapteurException const& e) {
        qDebug() << e.ObtenirErreur();
        exit(errno);
    }
    return data.byte & 0xFF;
}

/**
 * @brief Qi2cBus::EcrireRegistre
 * @param _registre  Adresse du registre à modifier
 * @param _valeur    octet à déposer dans le registre spécifié
 * @return           0 si l'écriture c'est bien effectuée, sinon l'exception est exécutée
 *
 * @details Ecrit la valeur dans le registre passé en paramètre. L'appel de la méthode
 *          CommencerTransmission(quint8 registre) est nécessaire avant pour
 *          désigner l'adresse du composant et prendre le bus pour l'échange.
 *          Plusieurs lecture sont possibles sur le meme composant avant de libérer
 *          le bus I2c avec la méthode TerminerTransmission().
 */
int Qi2cBus::EcrireRegistre(quint8 _registre, quint8 _valeur)
{
    union i2c_smbus_data data;

    int retour;
    data.byte = _valeur;
    try
    {
        if ((retour = i2c_smbus_access(I2C_SMBUS_WRITE, _registre, I2C_SMBUS_BYTE_DATA, &data)) < 0)
            throw CapteurException(errno, " Erreur Ecriture registre 8 bits n° "+ QString::number(_registre));
    }
    catch (CapteurException const& e)
    {
        qDebug() << e.ObtenirErreur();
        exit(errno);
    }
    return retour;
}

/**
 * @brief Qi2cBus::LireBlocRegistres
 * @param _registre Adresse du bloc de registre à lire
 * @param _valeurs  pointeur sur les valeurs lus
 * @param _taille   nombre d'octets à lire (limité à 32)
 * @return          nombre d'octets lus
 *
 * @details Lit les valeur dans les registres contigus à celui passé enparamètre.
 *          L'appel de la méthode CommencerTransmission(quint8 registre) est nécessaire
 *          avant pour désigner l'adresse du composant et prendre le bus pour l'échange.
 *          Plusieurs lecture sont possibles sur le meme composant avant de libérer
 *          le bus I2c avec la méthode TerminerTransmission().
 */
int Qi2cBus::LireBlocRegistres(quint8 _registre, quint8 *_valeurs, quint8 _taille)
{
    union i2c_smbus_data data;

    if (_taille > 32)
        _taille = 32;

    data.block[0] = _taille;
    try
    {
        if ((i2c_smbus_access(I2C_SMBUS_READ, _registre, _taille == 32 ? I2C_SMBUS_I2C_BLOCK_BROKEN :
                              I2C_SMBUS_I2C_BLOCK_DATA, &data)) < 0)
            throw CapteurException(errno, " Erreur Lecture d'un block n° "+ QString::number(_registre));
        else
            memcpy(_valeurs, &data.block[1], data.block[0] );
    }
    catch (CapteurException const& e)
    {
        qDebug() << e.ObtenirErreur();
        exit(errno);
    }
    return data.block[0];
}

/**
 * @brief Qi2cBus::LireRegistre16
 * @param _registre Adresse du registre à lire
 * @return          Valeur sur 16 bits du registre lu
 *
 * @details Lit la valeur du registre passé en paramètre. L'appel de la méthode
 *          CommencerTransmission(quint8 registre) est nécessaire avant pour
 *          désigner l'adresse du composant et prendre le bus pour l'échange.
 *          Plusieurs lecture sont possibles sur le meme composant avant de libérer
 *          le bus I2c avec la méthode TerminerTransmission().
 */
quint16 Qi2cBus::LireRegistre16(quint8 _registre)
{
    union i2c_smbus_data data;

    try
    {
        if ((i2c_smbus_access (I2C_SMBUS_READ, _registre, I2C_SMBUS_WORD_DATA, &data)) < 0)
            throw CapteurException(errno, " Erreur Lecture registre 16 bits n° "+ QString::number(_registre));
    }
    catch (CapteurException const& e)
    {
        qDebug() << e.ObtenirErreur();
        exit(errno);
    }
    return data.word & 0xFFFF ;
}

/**
 * @brief Qi2cBus::i2c_smbus_access
 * @param _mode     Mode d'accès : I2C_SMBUS_WRITE (ecriture) ou I2C_SMBUS_READ (lecture)
 * @param _registre adresse du registre affecté par l'opération
 * @param _taille   Nombre d'octets à lire ou écrire
 * @param _data     Données à lire ou écrire sur le bus
 * @return          0 si l'opération a été effectué correctement, -1 en cas d'erreur dans ce cas,
 *                  errno contient le code d'erreur.
 *
 * @details Fonction de bas niveau permettant la lecture et lecriture sur le bus I2c
 */
int Qi2cBus::i2c_smbus_access(char _mode, quint8 _registre, int _taille, i2c_smbus_data *_data)
{
    struct i2c_smbus_ioctl_data args;

    args.read_write = _mode;
    args.command = _registre;
    args.size = _taille;
    args.data = _data;
    return ioctl(fichierI2c, I2C_SMBUS, &args);
}

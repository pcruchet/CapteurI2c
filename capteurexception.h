#ifndef CAPTEUREXCEPTION_H
#define CAPTEUREXCEPTION_H

#include <QException>

class CapteurException : public QException
{
private:
    qint32 code;
    QString message;
    CapteurException& operator =(const CapteurException& autre);

public:
    CapteurException(qint32 _codeErreur, QString _message);
    QString ObtenirErreur() const;
};

#endif // CAPTEUREXCEPTION_H

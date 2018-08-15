#include "capteurexception.h"

CapteurException::CapteurException(qint32 _codeErreur, QString _message):
    QException(),
    code(_codeErreur),
    message(_message)
{

}

QString CapteurException::ObtenirErreur() const
{
    QString erreur = "Code Erreur : ";
    erreur += QString::number(code);
    erreur += " " ;
    erreur += message;

    return erreur;
}

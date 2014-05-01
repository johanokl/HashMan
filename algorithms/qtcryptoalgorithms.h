/**
 * This class uses the hashing functions available in QCryptographicHash.
 * Currently the following algorithms are supported:
 *  - SHA-1
 *  - SHA-256
 *  - SHHA-512
 *  - MD4
 *  - MD5
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#ifndef QTCRYPTOALGORITHMS_H
#define QTCRYPTOALGORITHMS_H

#include <QObject>

#include "hashalgorithm.h"

class QtCryptoAlgorithms : public HashAlgorithm
{
public:
   QString hashFile(QString filename, QString algorithm="");
};

#endif // QTCRYPTOALGORITHMS_H

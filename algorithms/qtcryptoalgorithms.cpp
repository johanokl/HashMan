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

#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

#include <QCryptographicHash>

#include "gui/mainwindow.h"
#include "hashproject/sourcedirectory.h"

#include "hashalgorithm.h"
#include "qtcryptoalgorithms.h"

QString QtCryptoAlgorithms::hashFile(QString filename, QString algorithm)
{
   QFile file(filename);
   if (!file.exists()) {
      qDebug() << "ERROR: File not found: " << filename;
      return QString("ERROR: File not found.");
   }
   if (!file.open(QFile::ReadOnly)) {
      qDebug() << "ERROR: " << file.errorString();
      return QString("ERROR: %1").arg(file.errorString());
   }
   QCryptographicHash::Algorithm activeAlgorithm = QCryptographicHash::Md5;
   if (algorithm == "MD4") {
      activeAlgorithm = QCryptographicHash::Md4;
   } else if (algorithm == "SHA-1") {
      activeAlgorithm = QCryptographicHash::Sha1;
   } else if (algorithm == "SHA-256") {
      activeAlgorithm = QCryptographicHash::Sha256;
   } else if (algorithm == "SHA-512") {
      activeAlgorithm = QCryptographicHash::Sha512;
   }
   QCryptographicHash hash(activeAlgorithm);
   hash.addData(&file);

   file.close();

   return QString(hash.result().toHex());
}

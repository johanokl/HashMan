/**
 * This is the CRC32 algorithm used by most other "CRC32 calculators" found on the Internet.
 * Information about the algorithm was taken from http://www.php.net/crc32,
 * http://stackoverflow.com/questions/2587766/how-is-a-crc32-checksum-calculated and
 * http://en.wikipedia.org/wiki/Cyclic_redundancy_check#CRCs_and_data_integrity
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QStack>
#include <QDebug>

#include "crc32algorithm.h"
#include "gui/mainwindow.h"
#include "hashproject/sourcedirectory.h"

#include "hashalgorithm.h"

Crc32algorithm::Crc32algorithm()
{
   crc32table = 0;
}

Crc32algorithm::~Crc32algorithm()
{
   if (crc32table) {
      delete crc32table;
   }
}

/**
 * @brief Crc32algorithm::reflect
 * Reflection is a requirement for the official CRC-32 standard.
 * You can create CRCs without it, but they won't conform to the standard.
 */
quint32 Crc32algorithm::reflect(quint32 ref, char ch)
{
   quint32 value(0);
   // Swap bit 0 for bit 7, bit 1 for bit 6, etc.
   for (int i = 1; i < (ch + 1); i++) {
      if (ref & 1) {
         value |= 1 << (ch - i);
      }
      ref >>= 1;
   }
   return value;
}

/**
 * @brief Crc32algorithm::initCRC32
 * Creates the lookup table used for the calculations.
 */
void Crc32algorithm::initCRC32()
{
   quint32 ulPolynomial = 0x04c11db7;
   crc32table = new quint32[256];
   // 256 values representing ASCII character codes.
   for (int i = 0; i < 256; i++) {
      crc32table[i] = reflect(i, 8) << 24;
      for (int j = 0; j < 8; j++) {
         crc32table[i] = (crc32table[i] << 1) ^ (crc32table[i] & (1 << 31) ? ulPolynomial : 0);
      }
      crc32table[i] = reflect(crc32table[i], 32);
   }
}

/**
 * @brief Crc32algorithm::hashFile
 * @param filename
 * @return CRC32 hash sum in hex format, always a 8 character long string.
 */
QString Crc32algorithm::hashFile(QString filename, QString)
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
   QDataStream datastream(&file);
   if (!crc32table) {
      initCRC32();
   }
   quint32 initmask = 0xffffffff;
   quint32 crc = initmask;
   quint8 currchar;
   while(!datastream.atEnd()) {
      datastream >> currchar;
      crc = (crc >> 8) ^ crc32table[(crc & 0xFF) ^ currchar];
   }
   crc = crc ^ initmask;
   file.close();
   // Return result stringified to 8 characters, prepend zeros if necessary
   return QString("%1").arg(crc, 8, 16, QChar('0')).toUpper();
}

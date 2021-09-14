/**
 * This is the CRC32 algorithm used by most other "CRC32 calculators" found on the Internet.
 * Information about the algorithm was taken from http://www.php.net/crc32,
 * http://stackoverflow.com/questions/2587766/how-is-a-crc32-checksum-calculated and
 * http://en.wikipedia.org/wiki/Cyclic_redundancy_check#CRCs_and_data_integrity
 *
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef CRC32CALCULATOR_H
#define CRC32CALCULATOR_H

#include <QObject>
#include <list>

#include "hashalgorithm.h"

class SourceDirectory;

class Crc32algorithm : public HashAlgorithm
{
public:
   Crc32algorithm();
   ~Crc32algorithm();
   QString hashFile(QString filename, QString algorithm="");

private:
   Crc32algorithm(Crc32algorithm const&);
   void operator=(Crc32algorithm const&);

   quint32 *crc32table;
   void initCRC32();
   quint32 reflect(quint32 ref, char ch);

};

#endif // CRC32CALCULATOR_H

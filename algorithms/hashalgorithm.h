/**
 * Pure abstract class for hashing algorithms.
 * Inherited by for example the class Crc32algorithm.
 *
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef HASHALGORITHM_H
#define HASHALGORITHM_H

#include <QString>

class HashAlgorithm
{
public:
   virtual QString hashFile(QString filename, QString algorithm="") = 0;
   virtual ~HashAlgorithm() {}
};

#endif // HASHALGORITHM_H

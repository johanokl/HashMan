/**
 * Manages the different hash calculation algorithms.
 *
 * While it's not a requirement, this class was designed for and
 * benefits from running in a separate QThread.
 * When in multithreaded mode, other threads can abort the scanning
 * by calling abort().
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#ifndef HASHER_H
#define HASHER_H

#include <QObject>
#include <QThread>

#include "hashproject/hashproject.h"

class QTableWidget;
class HashAlgorithm;

class Hasher : public QObject
{
   Q_OBJECT

public:
   Hasher();
   ~Hasher();
   void abort() { aborted = true; }

public slots:
   void hashProject(HashProject*, bool verify=false, QString basepath="");
   QString hashFile(int i, QString basepath, HashProject::File file, QString algorithm="CRC32", bool verify=false);
   void noMoreFiles();
   void startProcessWork();

signals:
   void progressstatus(int);
   void scanFinished();
   void fileHashCalculated(int id, QString algorithm, QString hash, bool verify);

private:
   bool aborted;
   bool scanFinishedSent;
   HashAlgorithm* crc32algorithm;
   HashAlgorithm* qtcryptoalgorithms;
};

#endif // HASHER_H

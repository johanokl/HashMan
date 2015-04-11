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
#include <QDir>

#include "hashproject/hashproject.h"
#include "hashproject/filelist.h"
#include "algorithms/crc32algorithm.h"
#include "algorithms/qtcryptoalgorithms.h"
#include "hasher.h"

/**
 * @brief Hasher::Hasher
 */
Hasher::Hasher()
{
   crc32algorithm = new Crc32algorithm;
   qtcryptoalgorithms = new QtCryptoAlgorithms;
   aborted = false;
   scanFinishedSent = true;
}

/**
 * @brief Hasher::~Hasher
 */
Hasher::~Hasher()
{
   delete crc32algorithm;
   delete qtcryptoalgorithms;
}

/**
 * @brief Hasher::hashProject
 * @param hashproject The hash project with the hash sums.
 * @param verify Is the calculation done to verify the previous hash sums.
 *
 * Calculates the hash sums for all files without any.
 * If the verify boolean is set to true, the hash sums will be calculated for all
 * files that haven't been verified.
 */
void Hasher::hashProject(HashProject *hashproject, bool verify, QString basepath)
{
   startProcessWork();
   HashProject::Settings settings = hashproject->getSettings();
   QString algorithm = settings.algorithm;
   HashAlgorithm* hashalgorithm = crc32algorithm;
   if (algorithm != "CRC32") {
      hashalgorithm = qtcryptoalgorithms;
   }
   if (!hashproject || !hashalgorithm) {
      scanFinished();
      return;
   }
   if (basepath.right(1) != QDir::separator()) {
      basepath += QDir::separator();
   }
   const QTableWidget* filelist = hashproject->getDataTable();
   for (int i=0; i<filelist->rowCount(); i++) {
      if (aborted) {
         scanFinished();
         return;
      }
      QString filename = filelist->item(i, 0)->text();
      if (filename.left(1) != QDir::separator()) {
         filename.prepend(basepath);
      }
      QString previousHash = filelist->item(i, 2)->text();
      QString previousVerify = filelist->item(i, 3)->text();
      QString previousAlgorithm = filelist->item(i, 5)->text();
      if ((verify && !previousHash.isEmpty() && previousVerify.isEmpty()) ||
          (!verify && previousHash.isEmpty())) {
         if (verify) {
            if (previousAlgorithm == "CRC32") {
               hashalgorithm = crc32algorithm;
            } else {
               hashalgorithm = qtcryptoalgorithms;
            }
            algorithm = previousAlgorithm;
         }
         QString hash = hashalgorithm->hashFile(filename, algorithm);
         emit fileHashCalculated(i, algorithm, hash, verify);
      }
      emit progressstatus(i+1);
   }
   scanFinished();
}

/**
 * @brief Hasher::startProcessWork
 */
void Hasher::startProcessWork()
{
   aborted = false;
}

/**
 * @brief Hasher::noMoreFiles
 * Slot invoked from FileList. Will only emit scanFinished.
 * Used for thread management, to see when the queue of signals from fileList is finished.
 */
void Hasher::noMoreFiles()
{
   emit scanFinished();
}

/**
 * @brief Hasher::hashFile
 * @param id Row id for the file entry. Set to -1 if called directly (won't send signal fileHashCalculated).
 * @param file File object
 * @param algorithm Which algorithm to use.
 * @param verify Pass-trough to signal fileHashCalculated.
 * @return The hash sum in string form.
 */
QString Hasher::hashFile(int id, QString basepath, HashProject::File file, QString algorithm, bool verify)
{
   QString hash;
   if (aborted) {
      return hash;
   }
   HashAlgorithm* hashalgorithm = crc32algorithm;
   if (algorithm != "CRC32") {
      hashalgorithm = qtcryptoalgorithms;
   }
   hash = hashalgorithm->hashFile(basepath + file.filename, algorithm);

   if (id > -1) {
      emit fileHashCalculated(id, algorithm, hash, verify);
   }
   return hash;
}

/**
 * Scans HashProject directories for files.
 *
 * The single public function will scan the directory set for
 * a project and emit a signal for all files found.
 * When all available sub-directories have been traversed and
 * the scan has finished, the signal scanFinished will be emitted.
 *
 * While it's not a requirement, this class was designed for and
 * benefits from running in a separate QThread.
 * When in multithreaded mode, other threads can abort the scanning
 * by calling abort().
 *
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QDirIterator>

#include "hashproject/sourcedirectory.h"
#include "hashproject/hashproject.h"
#include "hashproject/filelist.h"
#include "filefinder.h"

/**
 * @brief FileFinder::abort
 * Abort the scanning run in a separate thread.
 */
void FileFinder::abort()
{
   aborted = true;
}

/**
 * @brief FileFinder::scanProject
 * @param HashProject hashproject
 *
 * Scans the source directory set for the project, and emits the
 * signal fileFound(File) for all found files.
 * No signals are sent for directories.
 *
 * The HashProject::Settings configuration settings that can be defined
 * for this function are:
 * - scanimmediately: Calculate hash sum when a file has been found.
 *                    If set to false only the filename will be added.
 * - blockinghashcalc: If scanimmediately is true, calculate the hash sum
 *                     in this thread. Otherwise it may run in a different thread.
 * - algorithm: Which algorithm should be used for the calculation.
 */
void FileFinder::scanProject(HashProject* hashproject)
{
   aborted = false;

   if (!hashproject) {
      emit scanFinished();
      return;
   }
   if (!hashproject->getSourceDirectory()->isValid()) {
      emit scanFinished();
      return;
   }
   HashProject::Settings settings = hashproject->getSettings();

   QString basepath = hashproject->getSourceDirectory()->getPath();
   if (basepath.right(1) != QDir::separator()) {
      basepath += QDir::separator();
   }
   QDirIterator iterator(basepath, QDirIterator::Subdirectories);

   while (iterator.hasNext()) {
      if (aborted) {
         // Scanning was aborted by a separate thread.
         emit scanFinished();
         return;
      }
      iterator.next();
      if (!iterator.fileInfo().isDir()) {
         // Not a directory. Create a new File object and emit it.
         QString filename = iterator.filePath();
         HashProject::File filenode;
         filenode.filename = filename.remove(0, basepath.length());
         filenode.filesize = iterator.fileInfo().size();
         if (settings.blockinghashcalc && settings.scanimmediately) {
            filenode.hash = hasher.hashFile(-1, basepath, filenode, settings.algorithm);
            filenode.algorithm = settings.algorithm;
         }
         emit fileFound(filenode, false);
      }
   }
   emit scanFinished();
}

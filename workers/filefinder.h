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

#ifndef FILEFINDER_H
#define FILEFINDER_H

#include <QThread>
#include <list>

#include "hashproject/hashproject.h"
#include "hashproject/filelist.h"
#include "workers/hasher.h"

class SourceDirectory;

class FileFinder : public QObject
{
   Q_OBJECT

public:
   void abort();

public slots:
   void scanProject(HashProject* project);

signals:
   void scanFinished();
   void fileFound(HashProject::File, bool forceUpdate);

private:
   bool aborted;
   Hasher hasher;
};

#endif // FILEFINDER_H

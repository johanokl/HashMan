/**
 * Manages the list of files and their hashes.
 * Has to be owned by a HashProject instance.
 *
 * Built around the Qt5 widget QTableWidget, to facilitate asimple Model-View pattern.
 * Because of this, it can easily be displayed on the screen.
 *
 * Before the list is manipulated, the write semaphore has to be
 * locked using writeLock(true), and afterwards unlocked with writeLock(false).
 * While the list widget can still be scrolled when the list is locked, the sorting
 * can't be changed.
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#ifndef FILELIST_H
#define FILELIST_H

#include <QObject>
#include <QTableWidget>
#include <QLinkedList>
#include <QDebug>

#include "hashproject.h"

class FileList : public QTableWidget
{
   Q_OBJECT

public:
   FileList(HashProject* parent=0);
   ~FileList() {}

   bool writeLock(bool enable);
   void clearContents();
   bool isEmpty() { return (rowCount() == 0); }

   bool isHashCompleted() { return (numHashes > 0 && numHashes == rowCount()) ? true : false; }
   bool isHashPartiallyCompleted() { return (numHashes > 0) ? true : false; }

   bool isVerficationCompleted() { return (numHashes == numVerifiedHashes) ? true : false; }
   bool isVerificationPartiallyCompleted() { return (numVerifiedHashes > 0) ? true : false; }

   void removeSelectedRows();
   void copySelectedRowsToClipboard();

signals:
   void fileListSizeChanged(int, int, int, int);
   void displayFile(QString basepath, QString filename, QString hash);
   void hashFile(int id, HashProject::File file, QString algorithm);
   void noMoreFileJobs();
   void processingDone();

public slots:
   void fileAdditionFinished();
   void hashingFinished();
   void addFiles(QLinkedList<HashProject::File> files);
   void addFile(HashProject::File file, bool forceUpdate=false);
   void fileHashCalculated(int id, QString algorithm, QString hash, bool verify);
   void removeHashes();
   void removeVerifiedHashes();
   void setVerificationColumnsVisibility(bool visible);
   void rowSelectionChanged();

protected:
   void keyPressEvent(QKeyEvent *event);

private:
   void processBuffer(bool forcedUpdate=false);

   QList<HashProject::File> filesToAdd;

   int numHashes;
   int numInvalidFiles;
   int numVerifiedHashes;
   bool isWriteLocked;
   bool everythingValid;

   HashProject* parent;
};

#endif // FILELIST_H

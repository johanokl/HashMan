/**
 * This is a Qt widget with drag and drop support, which automatically
 * adds the dropped files to the active project filelist.
 *
 * At the moment only files can be added this way,
 * directories that are dropped are simply ignored.
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#ifndef FILEDROP_H
#define FILEDROP_H

#include <QLabel>
#include <QLinkedList>
#include "hashproject/hashproject.h"

class QMimeData;
class SourceDirectory;
class FileList;

class FileDrop : public QLabel
{
   Q_OBJECT

public:
   FileDrop(FileList* filelist);

public slots:
   void clear();

signals:
   void directoryDropped(QString);
   void startProcessWork();

protected:
   void dragEnterEvent(QDragEnterEvent *event);
   void dragMoveEvent(QDragMoveEvent *event);
   void dragLeaveEvent(QDragLeaveEvent *event);
   void dropEvent(QDropEvent *event);

private:
   QLabel *label;
   FileList* filelist;
   bool hasWriteLock;
};

#endif // FILEDROP_H

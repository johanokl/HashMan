/**
 * This is a Qt widget with drag and drop support, which automatically
 * adds the dropped files to the active project filelist.
 *
 * At the moment only files can be added this way,
 * directories that are dropped are simply ignored.
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDebug>

#include "hashproject/hashproject.h"
#include "hashproject/sourcedirectory.h"
#include "hashproject/filelist.h"
#include "gui/filedrop.h"

/**
 * @brief FileDrop::FileDrop
 * @param filelist
 */
FileDrop::FileDrop(FileList* filelist)
{
   this->filelist = filelist;
   setMinimumSize(200, 100);
   setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
   setAlignment(Qt::AlignCenter);
   setAcceptDrops(true);
   setAutoFillBackground(true);
   clear();
   hasWriteLock = false;
}

/**
 * @brief FileDrop::dragEnterEvent
 *
 * This function is called by the window manager when the mouse curser dragging the files enters the widget rectangle.
 * The widget will try to get a write lock on the filelist, and if it's successful it will change the widget background colour to green.
 * The write lock will be kept until the mouse cursor leaves the widget or when the processing of all dropped files have finished, whichever happens last.
 * If it for any reason is unable to get a write lock, it will change the background colour to red.
 */
void FileDrop::dragEnterEvent(QDragEnterEvent *event)
{
   hasWriteLock = filelist->writeLock(true);
   if (hasWriteLock) {
      setText(tr("Drop file(s)."));
      //setBackgroundRole(QPalette::Highlight);
      setStyleSheet("background-color: green;");
   } else {
      setText(tr("Not possible to add files."));
      setStyleSheet("background-color: red;");
   }
   event->acceptProposedAction();
}

/**
 * @brief FileDrop::dragMoveEvent
 * @param event
 */
void FileDrop::dragMoveEvent(QDragMoveEvent *event)
{
   event->acceptProposedAction();
}

/**
 * @brief FileDrop::dropEvent
 *
 * Called by the window manager when the files have been dropped on the widget.
 * Will create File nodes for all files and add them to the project file list.
 */
void FileDrop::dropEvent(QDropEvent *event)
{
   if (hasWriteLock) {
      QLinkedList<HashProject::File> addFilesList;
      const QMimeData *mimeData = event->mimeData();
      if (mimeData->hasUrls()) {
         QList<QUrl> urlList = mimeData->urls();
         for (int i = 0; i < urlList.size(); i++) {
            QString url = urlList.at(i).path();
            QFileInfo fileinfo(url);
            if (fileinfo.isDir()) {
               qDebug() << "Drag and drop support for directories is currently not available.";
            } else {
               HashProject::File filenode;
               filenode.algorithm ="";
               filenode.basepath = "";
               filenode.filename = url;
               filenode.filesize = fileinfo.size();
               addFilesList.append(filenode);
            }
         }
      }
      if (addFilesList.count() > 0) {
         emit startProcessWork();
         emit filesDropped(addFilesList);
      } else {
         filelist->writeLock(false);
      }
   }
   clear();
   event->acceptProposedAction();
}

/**
 * @brief FileDrop::dragLeaveEvent
 *
 *  The mouse cursor has left the widget space. Release write locks and clear graphics.
 */
void FileDrop::dragLeaveEvent(QDragLeaveEvent *event)
{
   if (hasWriteLock) {
      filelist->writeLock(false);
   }
   clear();
   event->accept();
}

/**
 * @brief FileDrop::clear
 *
 * Clears the graphics.
 */
void FileDrop::clear()
{
   setText(tr("Drag and drop files here"));
   hasWriteLock = false;
   setStyleSheet("");
   setBackgroundRole(QPalette::Dark);
}

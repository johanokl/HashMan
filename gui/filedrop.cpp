/**
 * This is a Qt widget with drag and drop support, which automatically
 * adds the dropped files to the active project filelist.
 *
 * At the moment only files can be added this way,
 * directories that are dropped are simply ignored.
 *
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>

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
   QList<QUrl> urlList;
   if (event->mimeData()->hasUrls()) {
      urlList = event->mimeData()->urls();
   }
   if (urlList.size() != 1) {
      setText(tr("Only one directory can be dropped per time."));
      setStyleSheet("background-color: red;");
   } else if (!filelist->isEmpty()) {
      setText(tr("File list is not empty."));
      setStyleSheet("background-color: red;");
   } else if (!QFileInfo(urlList.first().path()).isDir()) {
      setText(tr("Not possible to add individual files."));
      setStyleSheet("background-color: red;");
   } else  if (!(hasWriteLock = filelist->writeLock(true))) {
      setText(tr("Not possible to drop at the moment."));
      setStyleSheet("background-color: red;");
   } else {
      setText(tr("Drop directory."));
      setStyleSheet("background-color: green;");
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
      filelist->writeLock(false);
      QList<QUrl> urlList;
      if (event->mimeData()->hasUrls()) {
         urlList = event->mimeData()->urls();
      }
      if ((urlList.size() == 1) && (QFileInfo(urlList.first().path()).isDir())) {
         emit directoryDropped(urlList.first().path());
         emit startProcessWork();
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
   setText(tr("Drag and drop a directory here"));
   hasWriteLock = false;
   setStyleSheet("");
   setBackgroundRole(QPalette::Dark);
}

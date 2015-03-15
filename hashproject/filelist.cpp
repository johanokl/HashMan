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

#include <QHeaderView>
#include <QDebug>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QDir>

#include "filelist.h"

/**
 * @brief FileList::FileList
 * @param parent
 */
FileList::FileList(HashProject* parent)
{
   this->parent = parent;
   numHashes = 0;
   numVerifiedHashes = 0;
   isWriteLocked = false;
   numInvalidFiles = 0;

   qRegisterMetaType<QLinkedList<HashProject::File> >("QLinkedList<HashProject::File>");
   qRegisterMetaType<HashProject::File>("HashProject::File");

   QStringList labels;
   labels.append(tr("Basepath"));
   labels.append(tr("Name"));
   labels.append(tr("Filesize"));
   labels.append(tr("Hash"));
   labels.append(tr("Verification"));
   labels.append(tr("Match"));
   labels.append(tr("Alg."));

   setColumnCount(7);
   setShowGrid(true);
   setWordWrap(false);
   setEditTriggers(QAbstractItemView::NoEditTriggers);
   setHorizontalHeaderLabels(labels);
   setColumnHidden(0, true);
   removeHashes();
   horizontalHeader()->setStretchLastSection(false);
   horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
   setSelectionBehavior(QAbstractItemView::SelectRows);
   setSelectionMode(QAbstractItemView::ExtendedSelection);

   connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(rowSelectionChanged()));
}

/**
 * @brief FileList::removeSelectedRows
 * Removes the selected entries from the list.
 */
void FileList::removeSelectedRows()
{
   QList<QTableWidgetItem *> selectionList = selectedItems();
   if (!selectionList.isEmpty()) {
      QSet<int> selectedRows;
      foreach (QTableWidgetItem *item, selectionList) {
         selectedRows.insert(item->row());
      }
      //get a list, and sort it big to small
      QList<int> rows = selectedRows.toList();
      std::sort(rows.begin(), rows.end(), std::greater<int>());
      QList<int>::const_iterator it = rows.begin();
      int deleteLastRows = 0;
      while (it != rows.end()) {
         if ((*it) != rowCount()-1-deleteLastRows) {
            break;
         }
         it++;
         deleteLastRows++;
         rows.removeFirst();
      }
      setRowCount(rowCount()-deleteLastRows);
      foreach(int row, rows) {
         removeRow(row);
      }
      numHashes = 0;
      numVerifiedHashes = 0;
      for (int i=0; i<rowCount(); i++) {
         if (!item(i, 3)->text().isEmpty()) {
            numHashes++;
         }
         if (!item(i, 4)->text().isEmpty()) {
            numVerifiedHashes++;
         }
      }
      emit fileListSizeChanged(rowCount(), numHashes, numVerifiedHashes, numInvalidFiles);
      emit processingDone();
      // Reset the info bar widget.
      emit displayFile("", "", "");
   }
}

/**
 * @brief FileList::copySelectedRowsToClipboard
 * Copies the selected file entries as text to the operating system clipboard.
 * Currently only the files' full path, name and hash sum are copied.
 */
void FileList::copySelectedRowsToClipboard()
{
   QList<QTableWidgetItem *> selectionList = selectedItems();
   if (!selectionList.isEmpty()) {
      QSet<int> selectedRows;
      foreach (QTableWidgetItem *item, selectionList) {
         selectedRows.insert(item->row());
      }
      //get a list, and sort it big to small
      QList<int> rows = selectedRows.toList();
      std::sort(rows.begin(), rows.end());
      QString clipboardText;
      for (QList<int>::const_iterator it = rows.begin(); it != rows.end(); it++) {
         clipboardText += item(*it, 0)->text() + item(*it, 1)->text() + " " + item(*it, 3)->text() + "\n";
      }
      QApplication::clipboard()->setText(clipboardText);
   }
}

/**
 * @brief FileList::keyPressEvent
 * @param event
 * Event handler for key handling. Used for the following key shortcuts:
 *  - Delete/Backspace: removeSelectedRows()
 *  - Cmd-C/Control-C: copySelectedRowsToClipboard()
 */
void FileList::keyPressEvent(QKeyEvent *event)
{
   QString keyseq = QKeySequence(event->modifiers()|event->key()).toString().toLower();
   if (keyseq == "ctrl+c") {
      this->copySelectedRowsToClipboard();
      return;
   }
   switch (event->key()) {
   case Qt::Key_Delete:
   case Qt::Key_Backspace:
      if (writeLock(true)) {
         QMessageBox::StandardButton confirmreply;
         confirmreply = QMessageBox::question(this, "Confirm", "Remove selected rows?", QMessageBox::Yes|QMessageBox::No);
         if (confirmreply == QMessageBox::Yes) {
            this->removeSelectedRows();
         }
         writeLock(false);
      }
      break;
   default:
      QTableWidget::keyPressEvent(event);
   }
}

/**
 * @brief FileList::writeLock
 * @param enable If true, try to aquire the write lock. If false, release it.
 * @return True if successful, false if some other process already holds it.
 */
bool FileList::writeLock(bool enable)
{
   if (enable && isWriteLocked) {
      return false;
   }
   if (enable) {
      // Clear the buffer before we start with something new.
      filesToAdd.clear();
   }
   isWriteLocked = enable;
   setSortingEnabled(!isWriteLocked);
   return true;
}

/**
 * @brief FileList::clearContents
 * Remove all entries from the list.
 */
void FileList::clearContents()
{
   setRowCount(0);
   numHashes = 0;
   numVerifiedHashes = 0;
   numInvalidFiles = 0;
   emit fileListSizeChanged(rowCount(), numHashes, numVerifiedHashes, numInvalidFiles);
   setVerificationColumnsVisibility(false);
   // Reset the info bar widget.
   emit displayFile("", "", "");
}

/**
 * @brief FileList::rowSelectionChanged
 * Invoked by the window manager when the user has changed the entry selection in the list widget.
 */
void FileList::rowSelectionChanged()
{
   QList<QTableWidgetItem *> selectionList = selectedItems();
   if (!selectionList.isEmpty()) {
      int rowNum = selectionList.first()->row();
      emit displayFile(item(rowNum, 0)->text(), item(rowNum, 1)->text(), item(rowNum, 3)->text());
   }
}

/**
 * @brief FileList::setVerificationColumnsVisibility
 * @param visible
 * Whether or not the column with the verification values should be visible.
 * No point in having it visible if there's no verification data.
 */
void FileList::setVerificationColumnsVisibility(bool visible)
{
   setColumnHidden(4, !visible);
   setColumnHidden(5, !visible);
}

void FileList::setFileSizeVisibility(bool visible)
{
   setColumnHidden(2, !visible);
}

/**
 * @brief FileList::removeHashes
 * Remove all hashes from the list, keep the file entries.
 */
void FileList::removeHashes()
{
   numHashes = 0;
   for (int i=0; i<rowCount(); i++) {
      item(i, 3)->setText("");
   }
   removeVerifiedHashes();
}

/**
 * @brief FileList::removeVerifiedHashes
 * Remove all data about verified hashes from the list, keep the files and hashes.
 */
void FileList::removeVerifiedHashes()
{
   numVerifiedHashes = 0;
   numInvalidFiles = 0;
   for (int i=0; i<rowCount(); i++) {
      item(i, 4)->setText("");
      item(i, 5)->setText("");
      item(i, 5)->setBackground(QBrush());
   }
   emit fileListSizeChanged(rowCount(), numHashes, numVerifiedHashes, numInvalidFiles);
   setVerificationColumnsVisibility(false);
}

/**
 * @brief FileList::addFiles
 * @param files
 * Add a list of files. Force an update of the list after they've been added.
 */
void FileList::addFiles(QLinkedList<HashProject::File> files)
{
   for (QLinkedList<HashProject::File>::const_iterator file = files.constBegin(); file != files.constEnd(); ++file) {
      filesToAdd.append(*file);
   }
   processBuffer(true);
   emit noMoreFileJobs();
}

/**
 * @brief FileList::addFile
 * @param file
 * @param forceUpdate If the buffer list should be added even if the buffer threshold hasn't been reached.
 */
void FileList::addFile(HashProject::File file, bool forceUpdate)
{
   filesToAdd.append(file);
   processBuffer(forceUpdate);
}

/**
 * @brief FileList::fileAdditionFinished
 * Invoked by the file adder when there's no more files to be added.
 * Will send noMoreFileJobs() to the hasher threads.
 */
void FileList::fileAdditionFinished()
{
   processBuffer(true);
   emit noMoreFileJobs();
}

/**
 * @brief FileList::hashingFinished
 * The hasher threads got the FileList::noMoreFileJobs() and responded with a signal to this.
 * Emit processingDone() to the manager, signalling that all work is done.
 */
void FileList::hashingFinished()
{
   emit processingDone();
}

/**
 * @brief FileList::processBuffer
 * @param forcedUpdate
 *
 * If the buffer size has reached the threshold, or the forcedUpdate argument is true,
 * all the entries in the buffer will be added to the list.
 */
void FileList::processBuffer(bool forcedUpdate)
{
   if (!isWriteLocked) {
      return;
   }
   if (filesToAdd.isEmpty()) {
      emit noMoreFileJobs();
      return;
   }
   if (!forcedUpdate && filesToAdd.size() < 100) {
      return;
   }
   int numFiles = rowCount();
   setRowCount(numFiles + filesToAdd.size());
   for (QList<HashProject::File>::const_iterator file = filesToAdd.constBegin(); file != filesToAdd.constEnd(); ++file) {
      QTableWidgetItem* basepathcell = new QTableWidgetItem(QDir::toNativeSeparators((*file).basepath));
      QTableWidgetItem* filenamecell = new QTableWidgetItem(QDir::toNativeSeparators((*file).filename));
      QTableWidgetItem* filesizecell = new QTableWidgetItem;
      QTableWidgetItem* hashcell = new QTableWidgetItem;
      QTableWidgetItem* verifyhashcell = new QTableWidgetItem;
      QTableWidgetItem* ismatchcell = new QTableWidgetItem;
      QTableWidgetItem* algorithmcell = new QTableWidgetItem;
      if ((*file).filesize >= 0) {
         filesizecell->setData(Qt::DisplayRole, (*file).filesize);
      }
      if (!(*file).hash.isEmpty()) {
         numHashes++;
         hashcell->setText((*file).hash.toUpper());
      }
      if (!(*file).algorithm.isEmpty()) {
         algorithmcell->setText((*file).algorithm.toUpper());
      }
      basepathcell->setFont(QFont("Helvetica", 13));
      filenamecell->setFont(QFont("Helvetica", 13));
      filesizecell->setFont(QFont("Helvetica", 13));
      hashcell->setFont(QFont("Helvetica", 13));
      verifyhashcell->setFont(QFont("Helvetica", 13));
      ismatchcell->setFont(QFont("Helvetica", 13));
      algorithmcell->setFont(QFont("Helvetica", 13));
      setItem(numFiles, 0, basepathcell);
      setItem(numFiles, 1, filenamecell);
      setItem(numFiles, 2, filesizecell);
      setItem(numFiles, 3, hashcell);
      setItem(numFiles, 4, verifyhashcell);
      setItem(numFiles, 5, ismatchcell);
      setItem(numFiles, 6, algorithmcell);
      if (!isWriteLocked) {
         setRowCount(numFiles);
         return;
      }
      if ((*file).hash.isEmpty() && parent->getSettings().scanimmediately) {
         emit hashFile(numFiles, (*file), parent->getSettings().algorithm);
      }
      numFiles++;
   }
   filesToAdd.clear();
   emit fileListSizeChanged(rowCount(), numHashes, numVerifiedHashes, numInvalidFiles);
}

/**
 * @brief FileList::fileHashCalculated
 * @param id Row number.
 * @param algorithm Which algorithm was used.
 * @param hash Hash sum
 * @param verify Was this for verification?
 *
 * Updates the file list with the new hash sum.
 */
void FileList::fileHashCalculated(int id, QString algorithm, QString hash, bool verify)
{
   if (id < rowCount() && id > -1) {
      hash = hash.toUpper();
      if (!verify && item(id, 3)->text().isEmpty()) {
         numHashes++;
         item(id, 3)->setText(hash);
         item(id, 6)->setText(algorithm);
      } else if (verify) {
         numVerifiedHashes++;
         item(id, 4)->setText(hash);
         // Make the status row green or red depending on if the verification matched.
         if (item(id, 3)->text() == hash) {
            item(id, 5)->setText("MATCH");
            item(id, 5)->setBackground(QBrush(QColor(0,255,0)));
         } else {
            item(id, 5)->setText("INVALID");
            item(id, 5)->setBackground(QBrush(QColor(255,0,0)));
            numInvalidFiles++;
         }
      }
      emit fileListSizeChanged(rowCount(), numHashes, numVerifiedHashes, numInvalidFiles);
      viewport()->update();
   }
}


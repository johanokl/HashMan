#include <QtWidgets>
#include <QMessageBox>
#include <QTableView>
#include <QObject>
#include <QProgressBar>
#include <QSpacerItem>
#include <QCheckBox>
#include <QSettings>
#include <QSplitter>
#include <QFormLayout>

#include "hashcalcapplication.h"
#include "hashproject/sourcedirectory.h"
#include "gui/sourcedirectorywidget.h"
#include "workers/hasher.h"
#include "workers/filefinder.h"
#include "gui/menuactions.h"
#include "hashproject/filelist.h"
#include "hashproject/hashproject.h"
#include "gui/filedrop.h"
#include "gui/statusboxwidget.h"
#include "mainwindow.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(HashCalcApplication* parent) : QMainWindow()
{
   setWindowTitle(tr("HashMan"));
   setAttribute(Qt::WA_DeleteOnClose, true);
   setWindowIcon(QIcon(":/mainicon.icns"));

   isOpeningNewProject = false;
   parentapp = parent;

   mainproject = new HashProject;
   filelist = mainproject->getDataTable();

   createActionButtonBox();
   createDirectoryBoxes();
   createOptionsBox();
   createWorkerThreads();
   createFileDisplayBox();

   statusBox = new StatusBoxWidget;

   filedrop = new FileDrop(filelist);
   connect(filedrop, SIGNAL(startProcessWork()), this, SLOT(startFileFinder()));
   connect(filedrop, SIGNAL(directoryDropped(QString)), mainproject->getSourceDirectory(), SLOT(setPath(QString)));

   QVBoxLayout* controlLayout = new QVBoxLayout;
   controlLayout->addWidget(sourceDirectoryBox);
   controlLayout->addWidget(verifyDirectoryBox);
   controlLayout->addWidget(optionsBox);
   controlLayout->addWidget(statusBox);
   controlLayout->addWidget(actionButtonBox);
   controlLayout->addWidget(filedrop);
   controlLayout->setAlignment(displayFileBox, Qt::AlignTop);
   controlLayout->setContentsMargins(0, 0, 0, 0);

   optionsBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
   actionButtonBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
   statusBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
   filedrop->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

   controlWidgets = new QWidget;
   controlWidgets->setLayout(controlLayout);

   QVBoxLayout* rightLayout = new QVBoxLayout;
   rightLayout->addWidget(displayFileBox);
   rightLayout->addWidget(filelist);

   QWidget *rightWidget = new QWidget;
   rightLayout->setContentsMargins(0, 0, 0, 0);
   rightWidget->setLayout(rightLayout);

   mainWidget = new QSplitter(this);
   mainWidget->addWidget(controlWidgets);
   mainWidget->addWidget(rightWidget);
   mainWidget->setMinimumSize(1000, 750);
   mainWidget->setChildrenCollapsible(false);
   mainWidget->setCollapsible(0, false);
   mainWidget->setCollapsible(1, false);

   mainWidget->setStretchFactor(0, 0);
   mainWidget->setStretchFactor(1, 100);

   setCentralWidget(mainWidget);

   actions = new MenuActions(this);
   actions->createMenus();

   QSettings settings;
   algorithmComboBox->setCurrentText(settings.value("selectedalgorithm", "CRC32").toString());
   calcHashSumWhenFoundCheckbox->setChecked(settings.value("calchashsumwhenfound", false).toBool());
   if (!calcHashSumWhenFoundCheckbox->isChecked()) {
      hashCalculationOwnThreadCheckbox->setEnabled(false);
   }
   hashCalculationOwnThreadCheckbox->setChecked(settings.value("hashcalculationownthread", true).toBool());
   mainWidget->restoreState(settings.value("splittersizes").toByteArray());

   connect(filelist, SIGNAL(displayFile(QString,QString)), this, SLOT(updateFileDisplay(QString,QString)));
   connect(filelist, SIGNAL(fileListSizeChanged(int, int, int, int)), statusBox, SLOT(updateStatusBox(int, int, int, int)));
   connect(filelist, SIGNAL(fileListSizeChanged(int, int, int, int)), actions, SLOT(filelistChanged(int, int, int, int)));
   updateProjectSettings();
}

/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
   QSettings settings;
   settings.setValue("selectedalgorithm", algorithmComboBox->currentText());
   settings.setValue("calchashsumwhenfound", calcHashSumWhenFoundCheckbox->isChecked());
   settings.setValue("hashcalculationownthread", hashCalculationOwnThreadCheckbox->isChecked());
   settings.setValue("splittersizes", mainWidget->saveState());

   hasher->abort();
   filefinder->abort();
   hasherthread->deleteLater();
   filefinderthread->deleteLater();
   hasherthread->quit();
   filefinderthread->quit();

   delete hasher;
   delete filefinder;

   delete actions;
   delete mainWidget;
}

/**
 * @brief MainWindow::closeEvent
 *
 * Will get called by the window manager when it tries to close this window.
 * If data is being processed a warning popup will be displayed and the close
 * request will be ignored.
 * If the program is idle the window will be closed and no popups will be displayed.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
   if (!filelist->writeLock(true)) {
      QMessageBox::warning(this, "Can't close", "Can't close window while processing data.");
      event->ignore();
   } else {
      event->accept();
      parentapp->removeWindow(this);
   }
}


/**
 * @brief MainWindow::isListEmpty
 * True if the list is empty and not write locked.
 */
bool MainWindow::isListEmpty()
{
   if (!filelist->writeLock(true)) {
      return false;
   }
   filelist->writeLock(false);
   return filelist->isEmpty();
}

/**
 * @brief MainWindow::saveFile
 * @param filename The location the file will be written to.
 *
 * Write the project data to an SFV file.
 * See also MainWindow::openFile.
 */
void MainWindow::saveFile(QString filename)
{
   mainproject->saveFile(filename);
   QFileInfo fileInfo(filename);
   setWindowTitle(fileInfo.fileName() + tr(" - HashMan"));
   parent()->windowUpdated(this);
}

/**
 * @brief MainWindow::openFile
 * @param filename The location of the file that will be read.
 *
 * Reads a SFV file and loads the list of files along with their values into a new project.
 * If the file list contains calculated hash sums they will be verified after they've been imported.
 * See also MainWindow::saveFile.
 */
void MainWindow::openFile(QString filename)
{
   // We don't want to automatically scan all files that don't already have a hash sum.
   // Disable the checkbox value.
   bool tmpCheckboxValue = calcHashSumWhenFoundCheckbox->isChecked();
   calcHashSumWhenFoundCheckbox->setChecked(false);
   // When this is set to true, actionStopped() that will be called will verify all read hash values.
   isOpeningNewProject = true;
   bool openFileSuccess = mainproject->openFile(filename);
   calcHashSumWhenFoundCheckbox->setChecked(tmpCheckboxValue);
   if (openFileSuccess) {
      QFileInfo fileInfo(filename);
      setWindowTitle(fileInfo.fileName() + tr(" - HashMan"));
      parent()->windowUpdated(this);
      sourceDirectoryWidget->setReadOnly(true);
   }
}

/**
 * @brief MainWindow::createFileDisplayBox
 *
 * Creates a widget box with two text lines, one for the selected file's name and one for its hash sum.
 * The values are set by updateFileDisplay().
 */
void MainWindow::createFileDisplayBox()
{
   displayFilenameLine = new QLineEdit;
   displayHash = new QLineEdit;
   displayFilenameLine->setReadOnly(true);
   displayHash->setReadOnly(true);

   QGridLayout* layout = new QGridLayout;
   QLabel* displayFilenameLabel = new QLabel("Selected file:");
   QLabel* displayHashLabel = new QLabel("Hash sum:");
   layout->addWidget(displayFilenameLabel, 0, 0);
   layout->addWidget(displayFilenameLine, 0, 1);
   layout->addWidget(displayHashLabel, 1, 0);
   layout->addWidget(displayHash, 1, 1);
   displayFileBox = new QGroupBox();
   displayFileBox->setLayout(layout);
   displayFileBox->setVisible(false);
}

/**
 * @brief MainWindow::updateFileDisplay
 * @param filename Selected file's relative file name.
 * @param hash The hash sum.
 *
 * Update the text fiends in the file display box created by createFileDisplayBox().
 */
void MainWindow::updateFileDisplay(QString filename, QString hash)
{
   displayFileBox->setVisible(true);
   if (!filename.isEmpty()) {
      if (QFileInfo(filename).isRelative()) {
         QString basepath = mainproject->getSourceDirectory()->getPath();
         if (basepath.right(1) != QDir::separator()) {
            basepath += QDir::separator();
         }
         filename.prepend(basepath);
      }
   }
   displayFilenameLine->setText(filename);
   displayHash->setText(hash);
}

/**
 * @brief MainWindow::setSidebarVisible
 * @param display Displays if true, hides if false.
 *
 * Sets whether or not the left sidebar should be visible.
 */
void MainWindow::setSidebarVisible(bool display)
{
   controlWidgets->setVisible(display);
}

/**
 * @brief MainWindow::createWorkerThreads
 *
 * Creates the objects with the processing algorithms that are to be run i separate threads.
 * As of now they are one instance of Hasher and one instad of FileFinder.
 * Each instance is put in a QThread.
 *
 * To prevent race conditions, before doing any processing (calling a slot in the threads)
 * a call to processWorkStarted() must be made. When the threads are finished with the actions,
 * they will announce this with a signal to MainWindow::actionStopped.
 */
void MainWindow::createWorkerThreads()
{
   hasher = new Hasher;
   filefinder = new FileFinder;
   hasherthread = new QThread;
   filefinderthread = new QThread;
   filefinder->moveToThread(filefinderthread);
   hasher->moveToThread(hasherthread);
   filefinderthread->start();
   hasherthread->start();

   connect(this, SIGNAL(findFiles(HashProject*)), filefinder, SLOT(scanProject(HashProject*)));
   connect(this, SIGNAL(hashFiles(HashProject*, bool, QString)), hasher, SLOT(hashProject(HashProject*, bool, QString)));

   connect(filefinder, SIGNAL(fileFound(HashProject::File, bool)), filelist, SLOT(addFile(HashProject::File, bool)));

   connect(filelist, SIGNAL(hashFile(int, QString, HashProject::File, QString)), hasher, SLOT(hashFile(int, QString, HashProject::File, QString)));
   connect(hasher, SIGNAL(fileHashCalculated(int, QString, QString, bool)), filelist, SLOT(fileHashCalculated(int, QString, QString, bool)));

   /**
    * Signal path between the three threads when announcing that they are finished:
    *  Example, scanning for files using filefinder:
    *   filefinder.scanFinished -> filelist.fileAdditionFinished -> hasher.scanFinished -> filelist.processingDone -> mainwindow.actionStopped
    * Example, hashing files after dropping them:
    *   filelist.fileAdditionFinished -> hasher.scanFinished -> filelist.processingDone -> mainwindow.actionStopped
    * Example, hashing a project:
    *   hasher.hashproject -> hasher.scanFinished -> filelist.processingDone -> mainwindow.actionStopped
    */
   connect(this, SIGNAL(processWorkStarted()), hasher, SLOT(startProcessWork()));
   connect(filefinder, SIGNAL(scanFinished()), filelist, SLOT(fileAdditionFinished()));
   connect(filelist, SIGNAL(noMoreFileJobs()), hasher, SLOT(noMoreFiles()));
   connect(hasher, SIGNAL(scanFinished()), filelist, SLOT(hashingFinished()));
   connect(filelist, SIGNAL(processingDone()), this, SLOT(actionStopped()));

   connect(hasher, SIGNAL(progressstatus(int)), progressbar, SLOT(setValue(int)));
}

/**
 * @brief MainWindow::createOptionsBox
 *
 * Creates a group box with widgets allowing the user to define project settings.
 * When the settings have been updated a signal will invoke updateProjectSettings().
 * These available settings are as of now:
 *  - Which algorithm to use.
 *  - Scan the new files immidietly
 *  - If the above, should FileList or FileFinder calculate the hash in
 *    their own threads instead of issuing a signal to the HasherThread.
 */
void MainWindow::createOptionsBox()
{
   algorithmComboBoxLabel = new QLabel(tr("Hashing algorithm:"));
   algorithmComboBox = new QComboBox();
   algorithmComboBox->addItem(tr("CRC32"), "CRC32");
   algorithmComboBox->addItem(tr("MD4"), "MD4");
   algorithmComboBox->addItem(tr("MD5"), "MD5");
   algorithmComboBox->addItem(tr("SHA-1"), "SHA-1");
   algorithmComboBox->addItem(tr("SHA-256"), "SHA-256");
   algorithmComboBox->addItem(tr("SHA-512"), "SHA-512");

   QLabel* scanAfterFileFoundLabel = new QLabel(tr("Hash files when found:"));
   calcHashSumWhenFoundCheckbox = new QCheckBox;
   calcHashSumWhenFoundCheckbox->setChecked(false);
   scanAfterFileFoundLabel->setBuddy(calcHashSumWhenFoundCheckbox);

   QLabel* hashCalculationOwnThreadLabel = new QLabel(tr("Calculate concurrently:"));
   hashCalculationOwnThreadLabel->setWordWrap(false);
   hashCalculationOwnThreadCheckbox = new QCheckBox;
   hashCalculationOwnThreadCheckbox->setChecked(true);
   hashCalculationOwnThreadLabel->setBuddy(hashCalculationOwnThreadCheckbox);

   QGridLayout* layout = new QGridLayout;
   layout->addWidget(algorithmComboBoxLabel, 0, 1);
   layout->addWidget(algorithmComboBox, 0, 2);
   layout->addWidget(scanAfterFileFoundLabel, 2, 1);
   layout->addWidget(calcHashSumWhenFoundCheckbox, 2, 2);
   layout->addWidget(hashCalculationOwnThreadLabel, 3, 1);
   layout->addWidget(hashCalculationOwnThreadCheckbox, 3, 2);
   layout->setColumnStretch(0, 1);
   layout->setColumnStretch(4, 1);

   connect(calcHashSumWhenFoundCheckbox, SIGNAL(toggled(bool)), hashCalculationOwnThreadCheckbox, SLOT(setEnabled(bool)));
   connect(calcHashSumWhenFoundCheckbox, SIGNAL(toggled(bool)), hashCalculationOwnThreadLabel, SLOT(setEnabled(bool)));

   connect(algorithmComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateProjectSettings()));
   connect(calcHashSumWhenFoundCheckbox, SIGNAL(toggled(bool)), this, SLOT(updateProjectSettings()));
   connect(hashCalculationOwnThreadCheckbox, SIGNAL(toggled(bool)), this, SLOT(updateProjectSettings()));

   optionsBox = new QGroupBox(tr("Options"));
   optionsBox->setLayout(layout);
}

/**
 * @brief MainWindow::getSettings
 * @return HashProject::Settings
 *
 * Reads the values set in the options box and creates a HashProject::Settings object with them set.
 */
HashProject::Settings MainWindow::getSettings()
{
   HashProject::Settings settings;
   settings.algorithm = algorithmComboBox->currentText();
   settings.scanimmediately = calcHashSumWhenFoundCheckbox->isChecked();
   settings.blockinghashcalc = !hashCalculationOwnThreadCheckbox->isChecked();
   return settings;
}

/**
 * @brief MainWindow::updateProjectSettings
 * Slot called whenever something has changed in the options box. Will update the project with the new settings.
 */
void MainWindow::updateProjectSettings()
{
   mainproject->setSettings(this->getSettings());
}

/**
 * @brief MainWindow::removeSelectedRows
 * Removes the list of files selected in the file list.
 */
void MainWindow::removeSelectedRows()
{
   filelist->removeSelectedRows();
}

void MainWindow::setFileSizeVisible(bool visible)
{
   filelist->setFileSizeVisibility(visible);
}

/**
 * @brief MainWindow::copySelectedRows
 * Copies the list of files selected in the file list to the system's clip board in text form.
 */
void MainWindow::copySelectedRows()
{
   filelist->copySelectedRowsToClipboard();
}

/**
 * @brief MainWindow::moveToFront
 * Moves the window to the front, so that it lies on top of all other windows.
 */
void MainWindow::moveToFront()
{
   this->raise();
   this->activateWindow();
   this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
}

/**
 * @brief MainWindow::startProcessWork
 * Called when processing is about to start. Displays the progress bar and the thread abort button.
 */
void MainWindow::startProcessWork()
{
   progresswidget->show();
   progressbar->setValue(0);
   progressbar->setMaximum(100);
   actionButtons->setEnabled(false);
   optionsBox->setEnabled(false);
   sourceDirectoryWidget->setReadOnly(true);
}

/**
 * @brief MainWindow::startFileFinder
 * Removes all list entries and starts scanning with the file finder thread.
 * If the list isn't empty, a confirmation dialog will be displayed.
 */
void MainWindow::startFileFinder()
{
   if (!filelist->writeLock(true)) {
      return;
   }
   if (!filelist->isEmpty()) {
      QMessageBox msgBox(QMessageBox::Warning, "File Finder", "The list of files will be cleared and the scan will start from the beginning. Proceed?");
      msgBox.addButton(QMessageBox::Ok);
      QPushButton *abortButton = msgBox.addButton(QMessageBox::Cancel);
      msgBox.exec();
      if (msgBox.clickedButton() == abortButton) {
         filelist->writeLock(false);
         return;
      }
   }
   filelist->clearContents();
   startProcessWork();
   emit processWorkStarted();
   emit findFiles(mainproject);
}

/**
 * @brief MainWindow::startFileHasher
 */
void MainWindow::startFileHasher()
{
   if (!filelist->writeLock(true)) {
      return;
   }
   if (filelist->isHashCompleted()) {
      QMessageBox msgBox(QMessageBox::Question, "Hasher", "All listed files have already been scanned. Erase all old hashed values?");
      msgBox.addButton(tr("Erase and rescan"), QMessageBox::ActionRole);
      QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
      msgBox.exec();
      if (msgBox.clickedButton() == abortButton) {
         filelist->writeLock(false);
         return;
      }
      filelist->removeHashes();
      verifyFilesButton->setVisible(false);
   }
   else if (filelist->isHashPartiallyCompleted()) {
      QMessageBox msgBox(QMessageBox::Question, "Hasher", "Some of the listed files have already been scanned. How to proceed?");
      msgBox.addButton(tr("Scan only unscanned files."), QMessageBox::ActionRole);
      QPushButton *clearButton = msgBox.addButton(tr("Erase all old values and rescan"), QMessageBox::ActionRole);
      QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
      msgBox.exec();
      if (msgBox.clickedButton() == clearButton) {
         filelist->removeHashes();
      } else if (msgBox.clickedButton() == abortButton) {
         filelist->writeLock(false);
         return;
      }
   }
   startProcessWork();
   progressbar->setMaximum(filelist->rowCount());
   emit processWorkStarted();
   emit hashFiles(mainproject, false, mainproject->getSourceDirectory()->getPath());
}

/**
 * @brief MainWindow::startVerifyFiles
 */
void MainWindow::startVerifyFiles()
{
   if (!filelist->writeLock(true)) {
      return;
   }
   if (!filelist->isHashPartiallyCompleted()) {
      filelist->writeLock(false);
      return;
   }
   if (filelist->isVerficationCompleted()) {
      QMessageBox msgBox(QMessageBox::Question, "Verifier", "All listed files have already been verified. Redo everyting?");
      msgBox.addButton(tr("Verify everything again."), QMessageBox::ActionRole);
      QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
      msgBox.exec();
      if (msgBox.clickedButton() == abortButton) {
         filelist->writeLock(false);
         return;
      }
      filelist->removeVerifications();
   }
   else if (filelist->isVerificationPartiallyCompleted()) {
      QMessageBox msgBox(QMessageBox::Question, "Verifier", "Some of the listed files have already been verified. Redo everyting?");
      msgBox.addButton(tr("Scan only the unverified files."), QMessageBox::ActionRole);
      QPushButton *clearButton = msgBox.addButton(tr("Erase all old values and verify again."), QMessageBox::ActionRole);
      QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
      msgBox.exec();
      if (msgBox.clickedButton() == clearButton) {
         filelist->removeVerifications();
      } else if (msgBox.clickedButton() == abortButton) {
         filelist->writeLock(false);
         return;
      }
   }
   startProcessWork();
   progressbar->setMaximum(filelist->rowCount());
   emit processWorkStarted();
   emit hashFiles(mainproject, true, mainproject->getVerifyDirectory()->getPath());
}

/**
 * @brief MainWindow::clearResults

 * Remove all content from the file list.
 * Will display a message box asking the user to confirm the action.
 */
void MainWindow::clearResults()
{
   QMessageBox msgBox(QMessageBox::NoIcon, "Please confirm", "Clear the file list?");
   msgBox.addButton(QMessageBox::Yes);
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
   if (msgBox.exec() != QMessageBox::Yes) {
      return;
   }
   // Remove any appended file name from the title bar.
   setWindowTitle(tr("HashMan"));
   filelist->clearContents();
   actionStopped();
}

/**
 * @brief MainWindow::clearResults
 */
void MainWindow::clearHashes()
{
   QMessageBox msgBox(QMessageBox::NoIcon, "Please confirm", "Remove all calculated hashes?");
   msgBox.addButton(QMessageBox::Yes);
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
   if (msgBox.exec() != QMessageBox::Yes) {
      return;
   }
   filelist->removeHashes();
   actionStopped();
}

/**
 * @brief MainWindow::clearVerifications
 */
void MainWindow::clearVerifications()
{
   QMessageBox msgBox(QMessageBox::NoIcon, "Please confirm", "Remove all verification data?");
   msgBox.addButton(QMessageBox::Yes);
   msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
   if (msgBox.exec() != QMessageBox::Yes) {
      return;
   }
   filelist->removeVerifications();
   actionStopped();
}

/**
 * @brief MainWindow::stopScan
 * Aborts the work in both worker threads.
 */
void MainWindow::stopScan()
{
   hasher->abort();
   filefinder->abort();
}

/**
 * @brief MainWindow::actionStopped
 *
 * This is invoked after all threads have said that they are finished with the processing
 * started with processWorkStarted() has finished.
 * This function will remove the write lock from the file list.
 */
void MainWindow::actionStopped()
{
   filelist->resizeColumnToContents(6);
   actionButtons->setEnabled(true);
   optionsBox->setEnabled(true);
   if (filelist->isEmpty()) {
      displayFileBox->setVisible(false);
      hashFilesButton->setVisible(false);
      clearResultsButton->setVisible(false);
      sourceDirectoryWidget->setReadOnly(false);
   } else {
      displayFileBox->setVisible(true);
      hashFilesButton->setVisible(true);
      clearResultsButton->setVisible(true);
      sourceDirectoryWidget->setReadOnly(true);
   }
   if (filelist->isHashPartiallyCompleted()) {
      clearHashesButton->setVisible(true);
   } else {
      clearHashesButton->setVisible(false);
   }
   if (filelist->isVerificationPartiallyCompleted()) {
      clearVerificationsButton->setVisible(true);
   } else {
      clearVerificationsButton->setVisible(false);
   }
   if (filelist->isHashPartiallyCompleted()) {
      verifyFilesButton->setVisible(true);
   } else {
      verifyFilesButton->setVisible(false);
   }
   progresswidget->hide();
   filelist->writeLock(false);
   //
   if (isOpeningNewProject) {
      isOpeningNewProject = false;
      // Start verifying the files.
      if (filelist->isHashCompleted()) {
         startVerifyFiles();
      }
   }
}

/**
 * @brief MainWindow::createActionButtonBox
 *
 * Creates a simple group box with a couple of functions.
 * Te different functions invoke the actions:
 *  - MainWindow::startFileFinder
 *  - MainWindow::startFileHasher
 *  - MainWindow::startVerifyFiles
 *  - MainWindow::stopScan
 *  - MainWindow::clearResults
 */
void MainWindow::createActionButtonBox()
{
   findFilesButton = new QPushButton(tr("Find files"));
   hashFilesButton = new QPushButton(tr("Calculate"));
   verifyFilesButton = new QPushButton(tr("Verify"));
   cancelButton = new QPushButton(tr("Stop"));
   clearResultsButton = new QPushButton(tr("Clear list"));
   clearHashesButton = new QPushButton(tr("Clear hashes"));
   clearVerificationsButton = new QPushButton(tr("Clear verifications"));

   connect(findFilesButton, SIGNAL(clicked()), this, SLOT(startFileFinder()));
   connect(clearResultsButton, SIGNAL(clicked()), this, SLOT(clearResults()));
   connect(hashFilesButton, SIGNAL(clicked()), this, SLOT(startFileHasher()));
   connect(clearHashesButton, SIGNAL(clicked()), this, SLOT(clearHashes()));
   connect(clearVerificationsButton, SIGNAL(clicked()), this, SLOT(clearVerifications()));
   connect(verifyFilesButton, SIGNAL(clicked()), this, SLOT(startVerifyFiles()));
   connect(cancelButton, SIGNAL(clicked()), this, SLOT(stopScan()));

   progressbar = new QProgressBar;
   QHBoxLayout* progresslayout = new QHBoxLayout;
   progresslayout->addWidget(progressbar);
   progresslayout->addWidget(cancelButton);

   progresswidget = new QWidget;
   progresswidget->setLayout(progresslayout);
   progresswidget->hide();

   QGridLayout* actionButtonsLayout = new QGridLayout;
   actionButtonsLayout->addWidget(findFilesButton, 0, 0);
   actionButtonsLayout->addWidget(clearResultsButton, 0, 1);
   actionButtonsLayout->addWidget(hashFilesButton, 1, 0);
   actionButtonsLayout->addWidget(clearHashesButton, 1, 1);
   actionButtonsLayout->addWidget(verifyFilesButton, 2, 0);
   actionButtonsLayout->addWidget(clearVerificationsButton, 2, 1);
   hashFilesButton->setVisible(false);
   verifyFilesButton->setVisible(false);
   clearResultsButton->setVisible(false);
   clearHashesButton->setVisible(false);
   clearVerificationsButton->setVisible(false);
   actionButtons = new QWidget;
   actionButtons->setLayout(actionButtonsLayout);

   QVBoxLayout* layout = new QVBoxLayout;
   layout->addWidget(actionButtons);
   layout->addWidget(progresswidget);
   layout->setAlignment(actionButtons, Qt::AlignTop);
   layout->setAlignment(progresswidget, Qt::AlignTop);

   actionButtonBox = new QGroupBox("Scanning");
   actionButtonBox->setLayout(layout);
}

/**
 * @brief MainWindow::createDirectoryBoxes
 */
void MainWindow::pathStatusChanged()
{
   SourceDirectory* sourceDir = mainproject->getSourceDirectory();
   if (sourceDir && sourceDir->isValid()) {
      findFilesButton->setEnabled(true);
   } else {
      findFilesButton->setEnabled(false);
   }
   SourceDirectory* verifyDir = mainproject->getVerifyDirectory();
   if (verifyDir && verifyDir->isValid()) {
      verifyFilesButton->setEnabled(true);
   } else {
      verifyFilesButton->setEnabled(false);
   }
}

/**
 * @brief MainWindow::createDirectoryBoxes
 * Creates a group box widget containing a source directy widget.
 */
void MainWindow::createDirectoryBoxes()
{
   sourceDirectoryWidget = new SourceDirectoryWidget();
   QVBoxLayout* sourceDirectoryBoxLayout = new QVBoxLayout;
   sourceDirectoryBoxLayout->setContentsMargins(0, 0, 0, 0);
   sourceDirectoryBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);
   sourceDirectoryBoxLayout->addWidget(sourceDirectoryWidget);
   sourceDirectoryBoxLayout->setAlignment(sourceDirectoryWidget, Qt::AlignTop);
   sourceDirectoryBox = new QGroupBox("Source directory");
   sourceDirectoryBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   sourceDirectoryBox->setLayout(sourceDirectoryBoxLayout);

   verifyDirectoryWidget = new SourceDirectoryWidget();
   QVBoxLayout* verifyDirectoryBoxLayout = new QVBoxLayout;
   verifyDirectoryBoxLayout->setContentsMargins(0, 0, 0, 0);
   verifyDirectoryBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);
   verifyDirectoryBoxLayout->addWidget(verifyDirectoryWidget);
   verifyDirectoryBoxLayout->setAlignment(verifyDirectoryWidget, Qt::AlignTop);
   verifyDirectoryBox = new QGroupBox("Verify directory");
   verifyDirectoryBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   verifyDirectoryBox->setLayout(verifyDirectoryBoxLayout);

   sourceDirectoryWidget->setDirNode(mainproject->getSourceDirectory());
   verifyDirectoryWidget->setDirNode(mainproject->getVerifyDirectory());
   connect(sourceDirectoryWidget, SIGNAL(pathStatusSignal()), this, SLOT(pathStatusChanged()));
   connect(verifyDirectoryWidget, SIGNAL(pathStatusSignal()), this, SLOT(pathStatusChanged()));
}

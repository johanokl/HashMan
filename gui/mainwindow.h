#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QThread>
#include <QApplication>

#include "hashproject/hashproject.h"

class QGroupBox;
class StatusBoxWidget;
class QLabel;
class QPushButton;
class QTextEdit;
class QComboBox;
class QTableWidget;
class SourceDirectory;
class SourceDirectoryWidget;
class QGridLayout;
class QVBoxLayout;
class QProgressBar;
class Hasher;
class FileFinder;
class MenuActions;
class FileListView;
class FileList;
class QBoxLayout;
class QCheckBox;
class FileDrop;
class QLineEdit;
class QSplitter;
class HashCalcApplication;

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   MainWindow(HashCalcApplication *parent=0);
   ~MainWindow();
   HashCalcApplication* parent() const { return parentapp; }
   bool isListEmpty();

signals:
   void findFiles(HashProject*);
   void hashFiles(HashProject*, bool, QString basepath="");
   void processWorkStarted();

public slots:
   void moveToFront();
   //
   void startProcessWork();
   //
   void startFileFinder();
   void startFileHasher();
   void startVerifyFiles();
   //
   void stopScan();
   void actionStopped();
   //
   void pathStatusChanged();
   //
   void clearResults();
   void clearVerifications();
   void clearHashes();
   void saveFile(QString);
   void openFile(QString);
   void setSidebarVisible(bool);
   void setFileSizeVisible(bool);
   void updateFileDisplay(QString filename, QString hash);
   void updateProjectSettings();
   //
   void removeSelectedRows();
   void copySelectedRows();

protected:
   void closeEvent(QCloseEvent *event);

private:
   // Init functions
   void createWorkerThreads();
   void createActionButtonBox();
   void createOptionsBox();
   void createDirectoryBoxes();
   void createFileDisplayBox();

   HashProject::Settings getSettings();

   HashCalcApplication* parentapp;

   // When a new project is opened this will be set to true, and the calculator will start verifying all hash sums.
   bool isOpeningNewProject;

   // Project objects
   HashProject* mainproject;
   FileList* filelist;

   // Worker threads
   Hasher* hasher;
   FileFinder* filefinder;
   QThread* hasherthread;
   QThread* filefinderthread;

   // Window related
   QSplitter* mainWidget;
   QWidget* controlWidgets;
   MenuActions* actions;
   FileDrop* filedrop;

   // Status
   StatusBoxWidget* statusBox;

   // File display
   QGroupBox* displayFileBox;
   QLineEdit* displayFilenameLine;
   QLineEdit* displayHash;

   // Directories
   QGroupBox* sourceDirectoryBox;
   QGroupBox* verifyDirectoryBox;
   SourceDirectoryWidget* sourceDirectoryWidget;
   SourceDirectoryWidget* verifyDirectoryWidget;

   // Options
   QGroupBox* optionsBox;
   QLabel* algorithmComboBoxLabel;
   QComboBox* algorithmComboBox;
   QCheckBox* calcHashSumWhenFoundCheckbox;
   QCheckBox* hashCalculationOwnThreadCheckbox;

   // Actions
   QWidget* actionButtons;
   QGroupBox* actionButtonBox;
   QPushButton* cancelButton;
   QPushButton* findFilesButton;
   QPushButton* hashFilesButton;
   QPushButton* verifyFilesButton;
   QPushButton* clearResultsButton;
   QPushButton* clearHashesButton;
   QPushButton* clearVerificationsButton;
   QProgressBar* progressbar;
   QWidget* progresswidget;

};


#endif // MAINWINDOW_H

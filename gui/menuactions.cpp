/**
 * The menu bars and actions for MainWindow.
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#include <QtCore>
#include <QIcon>
#include <QAction>
#include <QToolBar>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QMainWindow>

#include "hashcalcapplication.h"
#include "mainwindow.h"
#include "menuactions.h"


/**
 * @brief MenuActions::MenuActions
 * @param parent
 */
MenuActions::MenuActions(MainWindow *parent) :
   QObject(parent), parentwindow(parent)
{
   QSettings settings;
   lastOpenedDirectory = settings.value("lastopeneddirectory", QDir::homePath()).toString();
}

/**
 * @brief MenuActions::~MenuActions
 */
MenuActions::~MenuActions()
{
   parent()->parent()->disconnect(this);

   /**
    * Save the state.
    * - sidebarvisible: setSidebarVisible(bool)
    * - filetoolbarvisible: setMenubarVisible
    * - lastopeneddirectory: File dialog starts in this directory.
    */
   QSettings settings;
   settings.setValue("sidebarvisible", displaySidebarAct->isChecked());
   settings.setValue("filetoolbarvisible", displayFileToolbarAct->isChecked());
   settings.setValue("lastopeneddirectory", lastOpenedDirectory);

   delete fileToolBar;
   delete fileMenu;
   delete editMenu;
   delete windowMenu;
   delete helpMenu;
}

/**
 * @brief MenuActions::parent
 * @return MainWindow* owner window.
 */
MainWindow* MenuActions::parent() const
{
   return parentwindow;
}

/**
 * @brief MenuActions::createActions
 * Creates all the action objects.
 */
void MenuActions::createActions()
{
   newAct = new QAction(QIcon(":/newicon.png"), tr("&New..."), parent());
   newAct->setShortcuts(QKeySequence::New);
   connect(newAct, SIGNAL(triggered()), parent()->parent(), SLOT(addWindow()));

   openAct = new QAction(QIcon(":/openicon.png"), tr("&Open..."), parent());
   openAct->setShortcuts(QKeySequence::Open);
   openAct->setStatusTip(tr("Open an existing file"));
   connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

   saveAct = new QAction(QIcon(":/saveicon.png"), tr("&Save"), parent());
   saveAct->setShortcuts(QKeySequence::Save);
   saveAct->setStatusTip(tr("Save the document to disk"));
   connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));

   saveAsAct = new QAction(tr("Save &As..."), parent());
   saveAsAct->setShortcuts(QKeySequence::SaveAs);
   saveAsAct->setStatusTip(tr("Save the document under a new name"));

   copyRowsAct = new QAction(tr("Copy selected rows"), parent());
   copyRowsAct->setShortcuts(QKeySequence::Copy);
   connect(copyRowsAct, SIGNAL(triggered()), parent(), SLOT(copySelectedRows()));
   copyRowsAct->setEnabled(false);

   removeRowsAct = new QAction(tr("Remove the selected rows"), parent());
   removeRowsAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
   connect(removeRowsAct, SIGNAL(triggered()), parent(), SLOT(removeSelectedRows()));
   removeRowsAct->setEnabled(false);

   closeWindowAct = new QAction(tr("Close &window"), parent());
   closeWindowAct->setShortcuts(QKeySequence::Close);
   connect(closeWindowAct, SIGNAL(triggered()), parent(), SLOT(close()));

   exitAct = new QAction(tr("&Quit program"), parent());
   exitAct->setShortcuts(QKeySequence::Quit);
   exitAct->setStatusTip(tr("Exit the application"));
   connect(exitAct, SIGNAL(triggered()), parent()->parent(), SLOT(quit()));

   displaySidebarAct = new QAction(tr("Display sidebar"), parent());
   displaySidebarAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
   displaySidebarAct->setCheckable(true);
   displaySidebarAct->setChecked(true);
   connect(displaySidebarAct, SIGNAL(triggered(bool)), this, SLOT(setSidebarVisible(bool)));

   displayFileToolbarAct = new QAction(tr("Display shortcut menu"), parent());
   displayFileToolbarAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
   displayFileToolbarAct->setCheckable(true);
   displayFileToolbarAct->setChecked(true);
   connect(displayFileToolbarAct, SIGNAL(triggered(bool)), this, SLOT(setMenubarVisible(bool)));

   aboutAct = new QAction(tr("&About"), parent());
   aboutAct->setStatusTip(tr("Show the application's About box"));
   connect(aboutAct, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
}

/**
 * @brief MenuActions::filelistChanged
 * @param numFiles
 *
 * Slot listening to the file list size change events.
 * Displays and hides the actions that can't be used unless there is at least one file list entry.
 */
void MenuActions::filelistChanged(int numFiles, int, int, int)
{
   if (numFiles > 0) {
      removeRowsAct->setEnabled(true);
      copyRowsAct->setEnabled(true);
   } else {
      removeRowsAct->setEnabled(false);
      copyRowsAct->setEnabled(false);
   }
}

/**
 * @brief MenuActions::windowsChanged
 *
 * Slot invoked when the HashCalcApplication window manager has closed a window.
 * Updates the list of other windows in the window menu.
 */
void MenuActions::windowsChanged()
{
   while(!windowlistActions.isEmpty()) {
      QAction* currAction = windowlistActions.last();
      windowlistActions.remove(windowlistActions.size()-1);
      windowMenu->removeAction(currAction);
      delete currAction;
   }
   for (int i=0; i < parent()->parent()->mainwindows.count(); i++) {
      MainWindow* currWindow = parent()->parent()->mainwindows[i];
      QAction* newAction = new QAction(currWindow->windowTitle(), parent());
      if (i < 10) {
         newAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1 + i));
      }
      connect(newAction, SIGNAL(triggered()), currWindow, SLOT(moveToFront()));
      windowMenu->addAction(newAction);
      windowlistActions.push_back(newAction);
   }
}

/**
 * @brief MenuActions::createMenus
 * Puts the actions in menus and toolbars.
 */
void MenuActions::createMenus()
{
   createActions();
   connect(parent()->parent(), SIGNAL(windowsChanged()), this, SLOT(windowsChanged()));

   fileMenu = parent()->menuBar()->addMenu(tr("&File"));
   fileMenu->addAction(newAct);
   fileMenu->addAction(openAct);
   fileMenu->addAction(saveAct);
   fileMenu->addAction(saveAsAct);
   fileMenu->addSeparator();
   fileMenu->addAction(closeWindowAct);
   fileMenu->addAction(exitAct);

   editMenu = parent()->menuBar()->addMenu(tr("&Edit"));
   editMenu->addAction(copyRowsAct);
   editMenu->addAction(removeRowsAct);

   windowMenu = parent()->menuBar()->addMenu(tr("&Window"));
   windowMenu->addAction(displaySidebarAct);
   windowMenu->addAction(displayFileToolbarAct);
   windowMenu->addSeparator();

   helpMenu = parent()->menuBar()->addMenu(tr("&Help"));
   helpMenu->addAction(aboutAct);

   fileToolBar = parent()->addToolBar(tr("File"));
   fileToolBar->setFloatable(false);
   fileToolBar->setMovable(false);
   fileToolBar->setIconSize(QSize(22, 22));
   fileToolBar->addAction(newAct);
   fileToolBar->addAction(openAct);
   fileToolBar->addAction(saveAct);

   QSettings settings;
   this->setMenubarVisible(settings.value("filetoolbarvisible", true).toBool());
   this->setSidebarVisible(settings.value("sidebarvisible", true).toBool());
}

/**
 * @brief MenuActions::saveFile
 * Displays a save file dialog and writes the project file to the chosen location.
 */
void MenuActions::saveFile()
{
   QString fileName = QFileDialog::getSaveFileName(parent(), tr("Save File"), QDir::homePath(), tr("Hash sets (*.sfv)"));
   if (fileName.isNull()) {
      return;
   }
   parent()->saveFile(fileName);
}

/**
 * @brief MenuActions::openFile
 * Displays an open file dialog and opens the chosen project file.
 */
void MenuActions::openFile()
{
   QString fileName = QFileDialog::getOpenFileName(parent(), tr("Open File"), lastOpenedDirectory, tr("Hash sets (*.sfv)"));
   if (fileName.isNull()) {
      return;
   }
   lastOpenedDirectory = QFileInfo(fileName).path();
   parent()->openFile(fileName);
}

/**
 * @brief MenuActions::setMenubarVisible
 * @param display True if it should be visible.
 * Controls whether the tool bar to the top in the main window should be visible or not.
 */
void MenuActions::setMenubarVisible(bool display)
{
   fileToolBar->setVisible(display);
   displayFileToolbarAct->setChecked(display);
}

/**
 * @brief MenuActions::setSidebarVisible
 * @param display True if it should be visible.
 * Controls whether the tool bar to the left in the main window should be visible or not.
 */
void MenuActions::setSidebarVisible(bool display)
{
   parent()->setSidebarVisible(display);
   displaySidebarAct->setChecked(display);
}

/**
 * @brief MenuActions::showAboutDialog
 * Very simple info text box.
 */
void MenuActions::showAboutDialog()
{
   QMessageBox::about(parent(), tr("About File Hash Calculator"),
                      QString("<p align='center'>")
                      .append("<h2>File Hash Calculator</h2>")
                      .append("<hr> <br>")
                      .append("(c) 2014 <br>")
                      .append("Johan Lindqvist <br>")
                      .append("<a href='mailto:johan.lindqvist@gmail.com'>johan.lindqvist@gmail.com</a>")
                      .append("<p>")
                      .append("<p align='center'>")
                      .append("More information at<br>")
                      .append("<a href='http://github.com/johanokl/FileHashCalculator'>http://github.com/johanokl/FileHashCalculator</a><br>")
                      .append("</p>"));
}

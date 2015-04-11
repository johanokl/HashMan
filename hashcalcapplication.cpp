/**
 * Manages the instances of class MainWindow.
 *
 * All instances of MainWindow has this as their parent object, and all windows' menu
 * bars contain links to all the other instances.
 *
 * Inherits QApplication and adds some convenient application specifc functions.
 * Will be destroyed (and the application process exited) when the
 * last visible MainWindow is closed.
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#include <QMessageBox>
#include <QFileOpenEvent>
#include <QWindow>
#include <QList>
#include <QWindowList>
#include <QMenu>

#include "gui/mainwindow.h"
#include "hashcalcapplication.h"

/**
 * @brief HashCalcApplication::HashCalcApplication
 * @param argc Number of arguments passed (including the application name).
 * @param argv Array of C char strings.
 */
HashCalcApplication::HashCalcApplication(int argc, char * argv[]) : QApplication(argc,argv)
{
   addWindow();
   if (argc > 1) {
      mainwindows.first()->openFile(QString(argv[1]));
   }
}

/**
 * @brief HashCalcApplication::quit
 * Tries to close all windows. Stops if one can't be closed.
 */
void HashCalcApplication::quit()
{
   while (!mainwindows.isEmpty()) {
      if (mainwindows.last()->close() == false) {
         return;
      }
   }
}

/**
 * @brief HashCalcApplication::addWindow
 * Adds a new MainWindow.
 */
MainWindow* HashCalcApplication::addWindow()
{
   MainWindow* newWindow = new MainWindow(this);
   mainwindows.push_back(newWindow);
   newWindow->show();
   emit windowsChanged();
   return newWindow;
}

/**
 * @brief HashCalcApplication::removeWindow
 * @param windowObj The window to be removed.
 * Removes a MainWindows from the window list.
 */
void HashCalcApplication::removeWindow(MainWindow* windowObj)
{
   int index = mainwindows.indexOf(windowObj);
   if (index == -1) {
      return;
   }
   mainwindows.remove(index);
   emit windowsChanged();
}

/**
 * @brief HashCalcApplication::windowUpdated
 *
 * Slot invoked when a program has updated its title.
 * Will emit windowsChanged to all the windows, telling them to update their window lists.
 */
void HashCalcApplication::windowUpdated(MainWindow*)
{
   emit windowsChanged();
}

/**
 * @brief HashCalcApplication::event
 * Used to handle files being dropped on the application icon in OS X.
 */
bool HashCalcApplication::event(QEvent * event) {
   if (event->type() == QEvent::FileOpen) {
      MainWindow* projectWindow = mainwindows.last();
      if (mainwindows.length() != 1 || !projectWindow->isListEmpty()) {
         // There are either multiple windows opened or the only one open
         // already contains data. Create a new fresh window for this project file.
         projectWindow = this->addWindow();
      }
      projectWindow->openFile(static_cast<QFileOpenEvent*>(event)->file());
      return true;
   }
   return QApplication::event(event);
}

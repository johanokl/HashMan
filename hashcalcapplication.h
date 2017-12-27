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
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef HASHCALCALCAPPLICATION_H
#define HASHCALCALCAPPLICATION_H

#include <QApplication>
#include <QVector>

class QMenu;

class MainWindow;

class HashCalcApplication : public QApplication
{
   Q_OBJECT

public:
   HashCalcApplication(int argc, char * argv[]);
   ~HashCalcApplication() {}
   void windowUpdated(MainWindow*);

signals:
   void windowsChanged();

public slots:
   MainWindow* addWindow();
   void removeWindow(MainWindow*);
   void quit();

protected:
   bool event(QEvent * event);

public:
   QVector<MainWindow*> mainwindows;

};


#endif // HASHCALCALCAPPLICATION_H

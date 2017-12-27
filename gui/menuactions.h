/**
 * The menu bars and actions for MainWindow.
 *
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef MENUACTIONS_H
#define MENUACTIONS_H

#include <QObject>
#include <QVector>

class QAction;
class QMenu;
class QToolBar;
class QMainWindow;
class MainWindow;

class MenuActions : public QObject
{
   Q_OBJECT

public:
   explicit MenuActions(MainWindow* parent = 0);
   ~MenuActions();
   void createMenus();
   MainWindow* parent() const;

signals:

public slots:
   void showAboutDialog();
   void setMenubarVisible(bool);
   void setSidebarVisible(bool);
   void setFileSizeVisible(bool);
   void openFile();
   void saveFile();
   void filelistChanged(int, int, int, int);
   void windowsChanged();

private:
   void createActions();
   QVector<QAction*> windowlistActions;

   QToolBar* fileToolBar;
   QMenu* fileMenu;
   QMenu* helpMenu;
   QMenu* editMenu;
   QMenu* windowMenu;

   QAction* newAct;
   QAction* openAct;
   QAction* saveAct;
   QAction* saveAsAct;
   QAction* closeWindowAct;
   QAction* exitAct;
   QAction* copyRowsAct;
   QAction* removeRowsAct;
   QAction* aboutAct;
   QAction* displaySidebarAct;
   QAction* displayFileToolbarAct;
   QAction* displayFileSizeAct;

   MainWindow* parentwindow;

   QString lastOpenedDirectory;
   QString lastSavedDirectory;
};

#endif // MENUACTIONS_H

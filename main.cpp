#include <QCoreApplication>
#include <QApplication>
#include "hashcalcapplication.h"

int main(int argc, char *argv[])
{
   QCoreApplication::setOrganizationName("Johan Lindqvist");
   QCoreApplication::setOrganizationDomain("github.com/johanokl");
   QCoreApplication::setApplicationName("File Hash Calculator");

   HashCalcApplication app(argc, argv);
   return app.exec();
}

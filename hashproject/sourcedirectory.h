/**
 * Holds a text path to a directory to be scanned for hashable files.
 *
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#ifndef SOURCEDIRECTORY_H
#define SOURCEDIRECTORY_H

#include <QObject>

class SourceDirectory : public QObject
{
   Q_OBJECT

public:
   SourceDirectory(QString dir);
   bool isValid();
   QString getPath();

signals:
   void pathStatusChanged(bool status);
   void pathChanged(QString);

public slots:
   void setPath(QString newpath);

private:
   QString path;
   bool validpath;
};




#endif // SOURCEDIRECTORY_H

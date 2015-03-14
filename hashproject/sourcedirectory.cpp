/**
 * Holds a text path to a directory to be scanned for hashable files.
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#include <QDir>
#include "sourcedirectory.h"

/**
 * @brief SourceDirectory::SourceDirectory
 * @param dir
 */
SourceDirectory::SourceDirectory(QString dir)
{
   validpath = false;
   if (dir.isEmpty()) {
      dir = QDir::homePath();
   }
   setPath(dir);
}

/**
 * @brief SourceDirectory::setPath
 * @param newpath The new path to be used.
 */
void SourceDirectory::setPath(QString newpath)
{
   this->path = newpath;
   QDir dir(newpath);
   if (dir.exists() != this->validpath) {
      this->validpath = !this->validpath;
      emit pathStatusChanged(this->validpath);
   }
}

/**
 * @brief SourceDirectory::getPath
 * @return The new path string.
 */
QString SourceDirectory::getPath()
{
   return path;
}

/**
 * @brief SourceDirectory::isValid
 * @return True if the path string points to a scannable directory.
 */
bool SourceDirectory::isValid()
{
   return validpath;
}


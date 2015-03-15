#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QHash>
#include <QLinkedList>

#include "filelist.h"
#include "hashproject.h"
#include "sourcedirectory.h"

/**
 * @brief HashProject::HashProject
 * @param parent
 */
HashProject::HashProject(QObject *parent) :
   QObject(parent)
{
   basepathSettingName = "FileHasherSetting:Directory=";
   algorithmSettingName = "FileHasherSetting:Algorithm=";
   sourceDirectory = 0;
   filelist = new FileList(this);
}

/**
 * @brief HashProject::~HashProject
 */
HashProject::~HashProject()
{
   if (sourceDirectory) {
     delete sourceDirectory;
   }
   delete filelist;
}

/**
 * @brief HashProject::setSourceDirectory
 * @param newDir The new directory.
 *
 * Changes which source directory object is tied to this project.
 * Note that the source directory's path can be changed independently of this function.
 */
void HashProject::setSourceDirectory(SourceDirectory* newDir)
{
   SourceDirectory* oldDir = sourceDirectory;
   sourceDirectory = newDir;
   emit sourceDirectoryChanged(newDir);
   if (oldDir) {
      delete oldDir;
   }
}

/**
 * @brief HashProject::setSettings
 * @param newSettings
 *
 * Updates the settings for the project.
 */
void HashProject::setSettings(Settings newSettings)
{
   activeSettings = newSettings;
}

/**
 * @brief HashProject::getSettings
 * @return The settings for the project.
 */
HashProject::Settings HashProject::getSettings()
{
   return activeSettings;
}

/**
 * @brief HashProject::openFile
 * @param filename
 * @return True if successful.
 *
 * Reads a SFV file and loads the list of files along with their values into a new file list.
 * See also HashProject::saveFile.
 */
bool HashProject::openFile(QString filename)
{
   if (!filelist->writeLock(true)) {
      QMessageBox::critical(0, "Error", "Can't load a new SFV file while data processing is in progress.");
      return false;
   }
   QFile file(filename);
   if (!file.exists()) {
      return false;
   }
   if (!file.open(QFile::ReadOnly)) {
      filelist->writeLock(false);
      return false;
   }

   filelist->clearContents();

   QTextStream textstream(&file);
   QString inpath = QFileInfo(filename).path();
   QString basepath, fullbasepath;
   QString algorithm = "CRC32";

   QHash<QString, HashProject::File> newFiles;
   QLinkedList<QString> filesOrder;
   while(!textstream.atEnd()) {
      QString textline = textstream.readLine();
      bool isComment = false;
      if ((textline.indexOf(";") != -1) &&
          (textline.mid(0, textline.indexOf(";")).simplified().length() == 0)) {
         // Line starts with ;
         // Either comment or metadata
         isComment = true;
      }
      int algorithmSettingPos = textline.indexOf(algorithmSettingName);
      int basepathSettingPos = textline.indexOf(basepathSettingName);
      if (isComment && algorithmSettingPos != -1) {
         // It's algorithm metadata, use the new value for the following list entries.
         algorithm = textline.right(textline.length() - algorithmSettingPos - algorithmSettingName.length());
      } else if (isComment && basepathSettingPos != -1) {
         // It's basepath metadata, use the new value for the following list entries.
         basepath = textline.right(textline.length() - basepathSettingPos - basepathSettingName.length());
         SourceDirectory* newDir;
         if (QFileInfo(basepath).isRelative()) {
            // The basepath is relative the stored location.
            fullbasepath = inpath + QDir::separator() + basepath;
            newDir = new SourceDirectory(fullbasepath);
         } else {
            // It's an absolute address.
            newDir = new SourceDirectory(basepath);
         }
         setSourceDirectory(newDir);
      } else {
         QStringList elements(textline);
         QString outfilename;
         QString filesize;
         if (isComment && textline.length() > 36) {
            filesize = textline.mid(1, 13).trimmed();
            textline = textline.right(textline.length() - 36);
         }
         if (textline.count("\"") > 1) {
            int firstPos = textline.indexOf("\"");
            int lastPos = textline.lastIndexOf("\"");
            outfilename = textline.mid(firstPos+1, lastPos-firstPos-1);
            textline.remove(firstPos+1, lastPos-firstPos-1);
         }
         elements = textline.split(" ", QString::SkipEmptyParts);
         if (outfilename.isNull()) {
            outfilename = elements.first();
         }
         outfilename = outfilename.remove(0, basepath.length());
         elements.removeFirst();
         QString hash;
         if (elements.length() > 0) {
            hash = elements.takeLast();
         }
         HashProject::File newfile;
         if (newFiles.contains(outfilename)) {
            newfile = newFiles[outfilename];
         } else {
            filesOrder.push_back(outfilename);
         }
         if (!filesize.isNull()) {
            newfile.filesize = filesize.toInt();
         }
         if (!isComment) {
            newfile.filename = outfilename;
            newfile.hash = hash;
            newfile.algorithm = algorithm;
            if (basepath.isEmpty() && fullbasepath.isEmpty()) {
               newfile.basepath = inpath;
            } else if (fullbasepath.isEmpty()) {
               newfile.basepath = basepath;
            } else {
               newfile.basepath = fullbasepath;
            }
         }
         newFiles[outfilename] = newfile;
      }
   }

   QLinkedListIterator<QString> i(filesOrder);
   while (i.hasNext()) {
      HashProject::File newfile = newFiles[i.next()];
      if (!newfile.filename.isNull()) {
         filelist->addFile(newfile);
      }
   }

   if (!sourceDirectory) {
      SourceDirectory* newDir = new SourceDirectory(inpath);
      setSourceDirectory(newDir);
   }
   filelist->fileAdditionFinished();
   file.close();
   return true;
}

/**
 * @brief HashProject::saveFile
 * @param filename
 * @return True if successful.
 *
 * Writes the project data to an SFV file.
 * Extends the standard CRC32 SFV file format with extra metadata added as comments.
 * Thus it's possible to open the saved files in other programs as long as they only
 * contain CRC32 hash sums, hile at the same time it's possible to save complex projects.
 * Information about the SFV file format is mainly taken from http://rescene.wikidot.com/pdsfv#format
 * See also HashProject::openFile.
 */
bool HashProject::saveFile(QString filename)
{
   if (filelist->isEmpty()) {
      return false;
   }
   QFile file(filename);
   if (!file.open(QFile::WriteOnly | QFile::Text)) {
      return false;
   }
   QString outpath = QFileInfo(filename).path();

   QString currAlgorithm = this->getSettings().algorithm;

   QTextStream out(&file);
   out << "; Generated by File Hash Calculator ver. " << QString(__DATE__).replace(" ", "-") << "-" << __TIME__ << " on ";

   QDateTime datetime = QDateTime::currentDateTime();
   out << datetime.date().year() << "-" << datetime.date().month() << "-" << datetime.date().day() << " ";
   out << datetime.time().hour() << ":" << datetime.time().minute() << "." << datetime.time().second() << endl;
   out << "; ---------------" << endl;

   out << "; " << algorithmSettingName << currAlgorithm << endl;
   QString basepath = filelist->item(0, 0)->text();
   if (basepath.indexOf(outpath) == 0) {
      basepath.remove(0, outpath.length() + 1);
   }
   out << "; " << basepathSettingName << basepath << endl;
   out << "; ---------------" << endl;

   for (int i=0; i < filelist->rowCount(); i++) {
      if (filelist->item(i, 6) && currAlgorithm != filelist->item(i, 6)->text()) {
         currAlgorithm = filelist->item(i, 6)->text();
         out << "; " << algorithmSettingName << currAlgorithm << endl;
      }
      QString fullpath = filelist->item(i, 0)->text() + filelist->item(i, 1)->text();
      if (fullpath.indexOf(outpath) == 0) {
         fullpath.remove(0, outpath.length() + 1);
      }
      if (fullpath.indexOf(" ") != -1) {
         fullpath.prepend("\"").append("\"");
      }
      if (filelist->item(i, 2) && !filelist->item(i, 2)->text().isEmpty()) {
         out << "; " << filelist->item(i, 2)->text().rightJustified(12) << QString().leftJustified(22) << fullpath << endl;
      }
      out << fullpath;
      if (filelist->item(i, 3)) {
         // Hash
         out << " " << filelist->item(i, 3)->text();
      }
      out << endl;
   }
   file.close();
   return true;
}

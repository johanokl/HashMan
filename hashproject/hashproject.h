#ifndef HASHPROJECT_H
#define HASHPROJECT_H

#include <QObject>

class FileList;

class SourceDirectory;

class HashProject : public QObject
{
   Q_OBJECT
public:

   struct File {
      QString filename;
      qint64 filesize;
      QString hash;
      QString algorithm;
   };

   struct Settings {
      QString algorithm;
      bool scanimmediately;
      bool blockinghashcalc;
   };

   explicit HashProject(QObject *parent = 0);
   ~HashProject();

   bool openFile(QString filename);
   bool saveFile(QString filename);

   SourceDirectory* getSourceDirectory() const {
      return sourceDirectory;
   }

   SourceDirectory* getVerifyDirectory() const {
      return verifyDirectory;
   }

   FileList* getDataTable() const { return filelist; }
   void setDataTable(FileList* filelist) { this->filelist = filelist; }

public slots:
   void setSettings(Settings newSettings);
   Settings getSettings();

private:
   SourceDirectory* sourceDirectory;
   SourceDirectory* verifyDirectory;
   FileList* filelist;
   Settings activeSettings;

   QString basepathSettingName;
   QString algorithmSettingName;
};

#endif // HASHPROJECT_H

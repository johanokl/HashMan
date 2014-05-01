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
      QString basepath;
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

   FileList* getDataTable() const { return filelist; }
   void setDataTable(FileList* filelist) { this->filelist = filelist; }

signals:
   void sourceDirectoryChanged(SourceDirectory*);

public slots:
   void setSourceDirectory(SourceDirectory* newDir);
   //
   void setSettings(Settings newSettings);
   Settings getSettings();

private:
   SourceDirectory* sourceDirectory;
   FileList* filelist;
   Settings activeSettings;

   QString basepathSettingName;
   QString algorithmSettingName;
};

#endif // HASHPROJECT_H

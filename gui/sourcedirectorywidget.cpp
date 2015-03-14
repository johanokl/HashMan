/**
 * Widget to display and modify an instance of the class SourceDirectory.
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#include <QFileDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QProgressBar>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>

#include "gui/sourcedirectorywidget.h"
#include "hashproject/sourcedirectory.h"

/**
 * @brief SourceDirectoryWidget::SourceDirectoryWidget
 * @param dirNode
 */
SourceDirectoryWidget::SourceDirectoryWidget(SourceDirectory* dirNode)
{
   this->dirNode = 0;
   lineedit = new QLineEdit;
   browsebutton = new QPushButton(tr("Browse for folder"));
   openbutton = new QPushButton(tr("Open externally"));
   validpathlabel = new QLabel("");
   validpathlabel->setStyleSheet("border: 2px solid black");
   validpathlabel->setMaximumSize(20,20);
   validpathlabel->setMinimumSize(20,20);

   connect(openbutton, SIGNAL(clicked()), this, SLOT(openInExplorer()));
   connect(browsebutton, SIGNAL(clicked()), this, SLOT(browseButtonAction()));

   QVBoxLayout *mainLayout = new QVBoxLayout;
   QHBoxLayout *firstRow = new QHBoxLayout;
   QHBoxLayout *secondRow = new QHBoxLayout;

   firstRow->addWidget(lineedit);
   firstRow->addWidget(validpathlabel);
   secondRow->addWidget(browsebutton);
   secondRow->addWidget(openbutton);

   mainLayout->addLayout(firstRow);
   mainLayout->addLayout(secondRow);
   setLayout(mainLayout);
   setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

   if (dirNode) {
      setDirNode(dirNode);
   }
}

/**
 * @brief SourceDirectoryWidget::setDirNode
 * @param newNode The new SourceDirectory instance to be managed by this instance.
 */
void SourceDirectoryWidget::setDirNode(SourceDirectory* newNode)
{
   if (newNode && newNode != dirNode) {
      if (dirNode) {
         lineedit->disconnect(dirNode);
         dirNode->disconnect(this);
      }
      connect(lineedit, SIGNAL(textChanged(QString)), newNode, SLOT(setPath(QString)));
      connect(newNode, SIGNAL(pathStatusChanged(bool)), this, SLOT(pathStatusChanged(bool)));
      pathStatusChanged(newNode->isValid());
      this->setDir(newNode->getPath());
      dirNode = newNode;
   }
}

/**
 * @brief SourceDirectoryWidget::openInExplorer
 * Opens the source location using the operating system file manager (OSX Finder etc).
 */
void SourceDirectoryWidget::openInExplorer()
{
   QString path = QDir::toNativeSeparators(lineedit->text());
   QDesktopServices::openUrl(QUrl("file:///" + path));
}

/**
 * @brief SourceDirectoryWidget::setDir
 * @param dirpath Sets the text in the line edit widget to this path.
 */
void SourceDirectoryWidget::setDir(QString dirpath)
{
   lineedit->setText(QDir::toNativeSeparators(dirpath));
}

/**
 * @brief SourceDirectoryWidget::browseButtonAction
 * Opens a directory selection dialog, the chosen location will be the new source directory.
 */
void SourceDirectoryWidget::browseButtonAction()
{
   QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), lineedit->text(),
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
   if (!dir.isNull()) {
      setDir(dir);
   }
}

/**
 * @brief SourceDirectoryWidget::setReadOnly
 * @param enable Set whether it shouldn't be possible to change which directory this points to.
 * In this mode it's still possible to select and copy the entered address.
 * Setting it to false reverts back to the normal behaviour.
 */
void SourceDirectoryWidget::setReadOnly(bool enable)
{
   if (enable) {
     lineedit->setReadOnly(true);
     browsebutton->setEnabled(false);
   } else {
     lineedit->setReadOnly(false);
     browsebutton->setEnabled(true);
   }
}

/**
 * @brief SourceDirectoryWidget::pathStatusChanged
 * @param isValid
 * The path verifier has changed the path status.
 * Change the status indicator to either green, if valid, or red, if invalid path.
 */
void SourceDirectoryWidget::pathStatusChanged(bool isValid)
{
   if (isValid) {
      validpathlabel->setStyleSheet("border: 2px solid black; background-color: green;");
      openbutton->setEnabled(true);
   } else {
      validpathlabel->setStyleSheet("border: 2px solid black; background-color: red;");
      openbutton->setEnabled(false);
   }
   validpathlabel->repaint();
}

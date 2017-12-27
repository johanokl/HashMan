/**
 * A simple widget to display information about the current file list.
 *
 *  - Total number of files in the file list.
 *  - Number of files that have been hashed.
 *  - Number of files that have been verified.
 *  - Number of verified files for which the two hash sums mismatched.
 *
 * Johan Lindqvist (johan.lindqvist@gmail.com)
 */

#include <QLabel>
#include <QGridLayout>

#include "statusboxwidget.h"

/**
 * @brief StatusBoxWidget::StatusBoxWidget
 */
StatusBoxWidget::StatusBoxWidget()
{
   projectvalidstatus = new QLabel("");
   projectvalidstatus->setStyleSheet("background-color: grey;");

   QLabel* fileslabel = new QLabel("Total Files:");
   QLabel* hashedlabel = new QLabel("Hashed:");
   QLabel* verifiedlabel = new QLabel("Verified:");
   QLabel* invalidlabel = new QLabel("Mismatches:");
   numfileslabel = new QLabel("0");
   numhashedlabel = new QLabel("0");
   numverifiedlabel = new QLabel("0");
   numinvalidlabel = new QLabel("0");

   QGridLayout* layout = new QGridLayout;
   layout->addWidget(fileslabel, 0, 1);
   layout->addWidget(numfileslabel, 0, 2);
   layout->addWidget(hashedlabel, 1, 1);
   layout->addWidget(numhashedlabel, 1, 2);
   layout->addWidget(verifiedlabel, 2, 1);
   layout->addWidget(numverifiedlabel, 2, 2);
   layout->addWidget(invalidlabel, 3, 1);
   layout->addWidget(numinvalidlabel, 3, 2);
   layout->addWidget(projectvalidstatus, 0, 4, 4, 1);

   layout->setColumnStretch(0, 10);
   layout->setColumnStretch(3, 10);
   layout->setColumnStretch(4, 100);

   this->setTitle(tr("Status"));
   this->setLayout(layout);
}

/**
 * @brief StatusBoxWidget::updateStatusBox
 * @param numfiles Total number of files in the file list.
 * @param numhashed Number of files that are hashed.
 * @param numverified Number of files that were verified.
 * @param numinvalid Number of files for which the two hash sums mismatched.
 *
 * Update the status box. Set the status label to either grey, if no files have been
 * verified, green, if all verified files were correct or red, if at least one verified files
 * had the wrong hash sum.
 */
void StatusBoxWidget::updateStatusBox(int numfiles, int numhashed, int numverified, int numinvalid)
{
   numfileslabel->setNum(numfiles);
   numhashedlabel->setNum(numhashed);
   numverifiedlabel->setNum(numverified);
   numinvalidlabel->setNum(numinvalid);
   if (numverified == 0) {
      projectvalidstatus->setStyleSheet("background-color: grey;");
   } else if (numinvalid > 0) {
      projectvalidstatus->setStyleSheet("background-color: red;");
   } else {
      projectvalidstatus->setStyleSheet("background-color: green;");
   }
}

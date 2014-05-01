/**
 * A simple widget to display information about the current file list.
 *
 *  - Total number of files in the file list.
 *  - Number of files that have been hashed.
 *  - Number of files that have been verified.
 *  - Number of verified files for which the two hash sums mismatched.
 *
 * Johan Lindqvist 2014, johan.lindqvist@gmail.com
 */

#ifndef STATUSBOXWIDGET_H
#define STATUSBOXWIDGET_H

#include <QGroupBox>

class QLabel;

class StatusBoxWidget : public QGroupBox
{
   Q_OBJECT
public:
   explicit StatusBoxWidget();

public slots:
   void updateStatusBox(int numfiles, int numhashed, int numverified, int numinvalid);

private:
   QLabel* projectvalidstatus;
   QLabel* numfileslabel;
   QLabel* numhashedlabel;
   QLabel* numverifiedlabel;
   QLabel* numinvalidlabel;
};

#endif // STATUSBOXWIDGET_H

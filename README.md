HashMan
==================

### Introduction
Calculates hash sums for files.  
Supports multiple hashing algorithm, including CRC32, MD5 and SHA-1.  
Can read, write and verify SFV files as well as scan directories for hashable files.

### Screenshot
![Screenshot 1](/screenshots/screenshot1.png?raw=true)

### Technical Details
Written in C++ and uses Qt 5.2 or later.  
Uses multiple threads on multiple CPU cores where supported.  
Tested on Mac OS X (10.10 and 10.13), Ubuntu 13.10 and Windows (7 and 10) with Qt 5.2.1, 5.3.0, 5.4.1 and 5.9.1.

### How to build
Install and configure Qt 5.9, available at http://www.qt.io/qt5-9.  
If Qt Creator was installed, use it to open and build the project file `HashMan.pro`.  
If Qt Creator isn't available, use a terminal to browse to the project root directory and run `qmake && make && make install`.

### Author
Johan Lindqvist  
johan.lindqvist@gmail.com  
https://github.com/johanokl

Icon by Alexandre Moore, http://sa-ki.deviantart.com.

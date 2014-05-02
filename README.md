File Hash Calculator
==================

### Introduction
Calculates hash sums for files.  
Supports multiple hashing algorithm, including CRC32, MD5 and SHA-1.  
Can read, write and verify SFV files as well as scan directories for hashable files.

### Technical Details
Written in C++ and uses Qt 5.2.  
Uses multiple threads on multiple CPU cores where supported.  
Tested on OSX 10.9, Ubuntu 13.10 and Windows 7 with Qt 5.2.1.

### How to build
Install and configure Qt 5.2, available at https://qt-project.org/qt5/qt52.  
If Qt Creator was installed, use it to open and build the project file `FileHashCalculator.pro`.  
If Qt Creator isn't available, use a terminal to browse to the project root directory and run `qmake && make && make install`.

### Author
Johan Lindqvist  
johan.lindqvist@gmail.com  
https://github.com/johanokl

Icon by Alexandre Moore, http://sa-ki.deviantart.com.
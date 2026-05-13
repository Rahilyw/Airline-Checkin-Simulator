====================================================================
CSC 360 - Project 2: Airline Check-in System (ACS)
Student: Rahil Wijeyesekera
====================================================================

Files Included:
- acs.c          (Main multi-threaded simulation logic)
- queue.c        (Queue operations and logic)
- queue.h        (Queue definitions and external variables)
- Makefile       (Compilation script)
- customers.txt  (Sample input file)
- design_report.pdf (Design document answering the 8 prompt questions)

How to Compile:
-------------------------------------------------------------------
Simply run the 'make' command in the terminal. The included Makefile 
will compile the source files using gcc with the -Wall and -pthread flags.

    $ make

This will generate an executable named 'ACS'.

How to Run:
------------------------------------------------------------------
Run the executable and provide the input text file as an argument:

    $ ./ACS customers.txt

To clean up the compiled executable and object files, run:

    $ make clean
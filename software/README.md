# ECE486-586-PDP11-Simulation

Software

To test your simulator you’ll write program fragments in PDP-11 assembly language and use the Macro-11 assembler to assemble them. Documentation for the Macro-11 assembler can be found at:
http://bitsavers.trailing-edge.com/pdf/dec/pdp11/rsx11/RSX11M_V2/DEC-11-OIMRA-A-D_MACRO_75.pdf
Professor Faust has written a translator to convert these object files to an easily parsed ASCII representation your simulator can read.

Executable versions of the Macro-11 assembler and the translator can be found in the following bin directory:
(~faustm/bin/macro11 and ~faustm/bin/obj2ascii) on the MCECS Linux systems (linux.cecs.pdx.edu).

Invoking macro11 without arguments will give you help. You’ll most likely want to use a command line like:
% macro11 source.mac –o source.obj –l source.lst

where source.mac is your assembly language input file, source.obj is the object file created and source.lst is the list file created.

To convert the code from the object file to ASCII use:
% obj2ascii source.obj source.ascii

Your simulator will then read the source.ascii memory image file.

Note that because your code will not be simulated along with an operating system, you cannot make system calls (e.g. PRINT).

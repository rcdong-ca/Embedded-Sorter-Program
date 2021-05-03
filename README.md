The solution for assignment for our Embedded Linux Programming Class.

This program was ran on the Beagle Bone Green.
The solution is comprised of 4 parts:
1. The Sorter Program
2. 14 Segment Display via i2c
3. Potentiometer via Analog to Digital Reading
4. User interface

The sorter program is constantly sorting an array of intergers of size N, while accepting commands from the user. 
The user can communicate with the program, that is running on the target, via UDP to get information of the current state of the sorting array.
The potentiometer sets the size of the array, while the 14 segment display shows the rate of the arrays being sorted
per second.

The accepted commands are :
1. help       -- shows the available commands
2. count      -- display number of arrays sorted
3. get length -- display length of array currently being sorted
4. get array  -- display the full array being sorted
5. get #      -- # = number, display the # ekenebt if array currently being sorted
6. stop       -- cause the server program to end

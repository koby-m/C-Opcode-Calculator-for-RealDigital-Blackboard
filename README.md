**Program Name:**           
Keypad Op-Code Calculator

**Contributor(s):**         
Koby Miller

**Date last modified:**     
February 13th, 2024

**Description:**            
An extremely basic calculator. On-board switches are used to determine operation to perform.
The four right-most switches are used to input 4-bit ARM Assembly op-codes. 
Only 4-digit numbers may be entered, though it operates in two modes, hexadecimal and decimal. The mode is
determined by the position of the 12th (left-most) switch. Digits are appended to the right-most digit of 
the number, shifting the number over. Operand is confirmed when any pushbutton is pressed, moving to
the next operand, then the solution based on the operation.
Should the solution of an operation exceed what is able to be displayed on the seven-segment display, 
four LEDs will come on to alert that the number is not displayed correctly on the seven-segment due to the
four digit limitation. This warning triggers when a negative number is produced also.

**Dependancies[2]:**           
klib_io (written in C)<br>
https://github.com/koby-m/klib_io---C-for-RealDigital-Blackboard<br>
klib_pfrl (written in C)<br>
https://github.com/koby-m/klib_prfl---C-for-RealDigital-Blackboard<br>

# Concurrent Character Sorting

## Description
- This program is used to order an unordered list of 7 characters (letters and numbers) entered by the user
- It uses Linux semaphores and shared memory to achieve concurrency (multiple tasks at one time) 
- The characters are ordered based on their decimal value they represent in the [Ascii Table](https://github.com/Anthony-Massaad/Concurrent-Character-Sorting/blob/main/ASCII_Table.png)
- The program uses 3 processes to handle concurrency and shared memory
  - Process 1 accesses and handles characters 1, 2 and 3 which the 3rd character is shared with process 2
  - Process 2 accesses and handles characters 3, 4 and 5 which the 5th character is shared with process 3
  - Process 3 accesses and handles characters 5, 6 and 7 

## Running the Program
- To run the program, it must be set in a Linux environment
- requires gcc version of 9.3.1 or higher 

1. To compile the program, simply use the makefile by typing:
```
$ make
```
2. After, an executable named CSORT will be created and can be run using: 
``` 
$ ./CSORT
```

## Instructions
** The program is NOT case-sensitive. Any valid input can be entered using uppercase or lowercase characters ** 
|----------------------------------------------------------------------------------------------------|

When first running the program, you'd be asked whether you'd wish to run the program in debug mode or not. Only valid responses are "y" or "n". Anything else would repeat the question until a valid response is provided. By enabling debug mode, the program will run with the additional feature of describing the sorting process in the terminal.

After, Enter exactle 7 characters with no spaces. Ideally, numbers or letters only. Any more than 7 would remove the excess and take only the first 7 characters entered. 

### Test 1:
```
Would you like debug mode? (y/n): n
Enter 7 chars (no spaces): XAzMWyD
Final results for process 3 [a d m w x y z]
Final results for process 2 [a d m w x y z]
Final results for process 1 [a d m w x y z]
```
### Test 2:
```
Would you like debug mode? (y/n): n
Enter 7 chars (no spaces): QHtbUSa
Final results for process 3 [a b h q s t u]
Final results for process 2 [a b h q s t u]
Final results for process 1 [a b h q s t u]
```

### Program can be unpredictable if:
- Entering less than 7 characters
- Entering special characters (i.e., !@#$..etc). Despite it might working with special characters, it might not produce the correct results
- Adding spaces between the entered characters

## Credits
Author: Anthony Massaad

Copyright © 2021 Anthony Massaad. All rights reserved

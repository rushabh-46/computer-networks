First run Makefile as 
$ make 

Both the executables will be generated emailserver and emailclient
Run emailserver in the current terminal as
$ ./emailserver 9999

Open another terminal and go into the same directory as above and run emailclient as
$ ./emailclient localhost 9999


I have used MAX as a macro and setup its value as 256
So, maximum limit for all things is MAX - 
    number of users, 
    number of characters that can be scanned or printed using character arrays
    number of characters in a user's name

Use make clean for deleting executables
$ make clean 

Commands for execution : (Type the Commands as follows Capital letters as Capital and small letters as small)
Listusers
Adduser username            - username can be replaced with any name and can have Capital letters at arbitary positions
SetUser username
    Read
    Delete
    Send username
    Done
Quit

Note : All spool files created will be Deleted(removed) when the command "Quit" is given in the Main-Prompt,
        so any checking of spool files can be done before giving this "Quit" command in the client terminal.
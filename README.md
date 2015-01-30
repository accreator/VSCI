# VSCI
Very Simple C Interpreter (VSCI) is an interpreter for simplified C. VSCI is designed to achieve easier and more flexible software configuration. 

Currently, VSCI supports the following features:

* if else while break continue return
* float int void
* / + - % & ^ ~ | << >> ,
* > < >= <= == != && || !
* = ( ) , ; { } [ ] 
* *(only 1-dim array is supported; VLA is supported)*

Try the following:

    #For Windows
    gcc vsci.c -o vsci.exe
    vsci test/test8.c

    #For Linux/Mac/BSD
    gcc vsci.c -o vsci
    ./vsci test/test8.c

It will output the square root of 2

Kai Sun
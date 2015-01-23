# VSCI
Very Simple C Interpreter (VSCI) is an interpreter for simplified C. VSCI is designed to achieve easier and more flexible software configuration. 

Currently, VSCI supports the following features:

* if else while break continue return
* float int void
* / + - % & ^ ~ | ,
* > < >= <= == != && || !
* = ( ) , ; { } [ ] 
* *(only 1-dim array)*

Try the following:

    #For Windows
    gcc vsci.c -o vsci.exe
    vsci test/test3.c

    #For Linux/Mac/BSD
    gcc vsci.c -o vsci
    ./vsci test/test3.c

It will output `ln(1.5)`

Kai Sun
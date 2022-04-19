# CMake Project Template on Windows

![avatar](https://badgen.net/badge/Language/C11/orange)
![stars](https://badgen.net/badge/Dev%20Env./Windows/green)
![license](https://badgen.net/badge/License/Apache-2.0/blue)

## ONE - Directory ##
```
.
├── bin                 //Execurable Files
├── build               //Temporary Files
├── auto.bat             //Auto-Compile Script
├── include             //Header Files
│   ├── main.h
│   └── module1
│       └── head.h
├── CMakeLists.txt      //CMake Configuration File
├── README.md
└── src                 //Source Files
    ├── main.c        
    └── module1
        └── test1.c
```
## * NOTICE 
- Personal CMake Configuration Required
- VSCode With CMake-Tools Recommended

## TWO - Command ##
* i . Compile Project .
    
    ```
        $ ./auto.bat 
    ```
    Revise **`auto.sh`** for personal change.


* ii . Run Project .
    ```
        $ cd bin
        $ ${PROJECT_NAME}
    ```

## THREE - Module ##
* i . Add source files at **`/src`** .
* ii . Add included header files at **`/include`** .
* iii . Cite header file **`main.h`** , add this on your files
    ```c
        #include <$(MODULE_NAME)/main.h>
    ```
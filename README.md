# Pongo
A webserver for my website. The webserver and website, except media files are part of one binary file. This repository contains only an example website. 

## build under linux
open a terminal in the project folder and enter:

    cmake CMakeLists.txt
    make
    ./Pongo

The sever should then response to http://127.0.0.1:8080/ respectively  https://127.0.0.1:8081/ 
### dependecies
cmake and a compiler (build-essential package) are needed. The LibreSSL library and a valid ssl certificate are also needed if encryption is desired.

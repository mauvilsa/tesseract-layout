# NAME

tesseract-layout - A tool for obtaining tesseract's layout analysis in Page XML format.

# INSTALLATION AND USAGE

    git clone https://github.com/mauvilsa/tesseract-layout
    mkdir tesseract-layout/build
    cd tesseract-layout/build
    cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME ..
    make install
    
    tesseract-layout --help
    tesseract-layout IMAGE > IMAGE.xml

# COPYRIGHT

The MIT License (MIT)

Copyright (c) 2015-present, Mauricio Villegas <mauvilsa@upv.es>

# NAME

tesseract-layout - A tool for obtaining tesseract's layout analysis in Page XML format.

# DEPRECATION

Exactly the same functionality that this tool provides is available in the much more versatile [tesseract-recognize](https://github.com/mauvilsa/tesseract-recognize) tool. This tesseract-layout is considered deprecated and will no longer be supported.

# INSTALLATION AND USAGE

    git clone https://github.com/mauvilsa/tesseract-layout
    mkdir tesseract-layout/build
    cd tesseract-layout/build
    cmake -DCMAKE_INSTALL_PREFIX:PATH=$HOME ..
    make install
    
    tesseract-layout --help
    tesseract-layout IMAGE > IMAGE.xml

# VIEWING RESULTS

The results can be viewed/edited using the Page XML editor available at https://github.com/mauvilsa/nw-page-editor or using other tools that support this format such as http://www.primaresearch.org/tools and https://transkribus.eu/Transkribus/ .

# COPYRIGHT

The MIT License (MIT)

Copyright (c) 2015-present, Mauricio Villegas <mauricio_ville@yahoo.com>

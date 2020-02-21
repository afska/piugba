#!/bin/bash

DIR_PROJECT=$(cmd //c cd)
DIR_DEVKITPRO="$DIR_PROJECT\..\..\tools\devkitPro"

DIR_PROJECT_ESCAPED="${DIR_PROJECT//\\/\\\\}"
DIR_DEVKITPRO_ESCAPED1="${DIR_DEVKITPRO//\\/\\\\}"
DIR_DEVKITPRO_ESCAPED2="${DIR_DEVKITPRO//\\/\\\\\\\\}"

sed -i -e "s/DIR_PROJECT/$DIR_PROJECT_ESCAPED/g" Makefile
sed -i -e "s/DIR_DEVKITPRO/$DIR_DEVKITPRO_ESCAPED1/g" Makefile
sed -i -e "s/DIR_DEVKITPRO/$DIR_DEVKITPRO_ESCAPED2/g" .vscode/c_cpp_properties.json
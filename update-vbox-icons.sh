#!/bin/bash

set -e

cd vbox-icons
for img in *.png; do
    # Use --fail so that the command fails when the icon is missing
    echo Updating $img ...
    curl --fail https://www.virtualbox.org/svn/vbox/trunk/src/VBox/Frontends/VirtualBox/images/$img > $img
done

#!/bin/bash

if [ "$0" != "./del.sh" ]
then
  echo "wrong working dir"; exit
fi

echo "good working dir: $0"; read
P=$(dirname $(realpath "$0"))
echo "delete '$P'?"; read
cd ../
yes | rm -r "$P"
echo "done."



# ~~~~~~~~~~~~~~
# ~~~~~~~~~~~~~~

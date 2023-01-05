#!/usr/bin/env bash
SHAMONDIR=$(readlink -f $(dirname $0))
echo $SHAMONDIR

for FILE in bin/*; do
  echo $SHAMONDIR/$FILE;
  sed "s|\$BASEPATH/shamon|$SHAMONDIR|" $FILE > tmpfile
  sudo chmod +xxx tmpfile
  sudo mv tmpfile /usr/$FILE
done

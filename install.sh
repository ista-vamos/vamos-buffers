#!/usr/bin/env bash
VAMOSDIR=$(readlink -f $(dirname $0))
echo $VAMOSDIR

for FILE in bin/*; do
  echo $VAMOSDIR/$FILE;
  sed "s|\$BASEPATH/shamon|$VAMOSDIR|" $FILE > tmpfile
  sudo chmod +xxx tmpfile
  sudo mv tmpfile /usr/$FILE
done

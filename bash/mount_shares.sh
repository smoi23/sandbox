#!/bin/bash

# available shares
shares="thb idm abc"

# helper function
function ItemInList
{
  result=1
  for item in $1; do
    if [[ $item == "$2" ]]
    then
      result=0
    fi
  done
  return $result  
}

function ItemInListReg
{
  result=1
  if [[ $1 =~ (^|[[:space:]])"$2"($|[[:space:]]) ]]
  then
      result=0
  fi
  return $result  
}

echo "Available shares: $shares"
 
if [ -z $1 ]
then
  echo "Specify the share name"
else
  shr=$1
  if `ItemInListReg "$shares" $shr`
  then
    echo "Mount: "$shr
    sudo mkdir -p /media/bigfoot/$shr
    sudo mount -t cifs -o username=aschuste,uid=aschuste,domain=MEDIANET,vers=2\.0 //bigfoot/$shr /media/bigfoot/$shr 
  else
    echo "Share not found: "$shr
 fi
fi


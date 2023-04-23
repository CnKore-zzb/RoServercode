
if [ $# != 2 ]; then
  echo "./select.sh 5 23"
  exit 1;
fi


reg1=$(grep -w $1 Proto/xCmd.proto)
key=$(echo $reg1|awk '{print $1}')
file=$(grep -w $key */*.proto|awk -F':' '{print $1}'|grep -v "Proto/xCmd.proto"|awk '!a[$0]++')
if [ -n "$file" ];then
    reg2=$(grep -w $2 $file)
fi

if [  -n "$reg2" ];then
  echo $reg1
  grep -w $2 $file|grep -Ee "[A-Z]+(_[A-Z]+)+ = \w+;"|sed 's/^[ \t]*//g'
  echo $file
else
  echo $reg1
fi


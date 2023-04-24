#!/bin/sh

#编译指定分支代码,参数为master studio tf release,
#参数为空则编译上述4个分支,并将编译后的可执行文件
#上传到svn目录

#rootdir=/home/lyq
exedir=bin/Debug

branch=$1

if [ ! -d "/autoWork" ]
then
  mkdir autoWork
  echo "autoWork 文件夹创建"
else
  echo "autoWork 文件夹已经存在"
fi

if [ -f "test_autoMake.txt" ]
then
  echo "程序正在运行,无法重复启动"
  logtime=$(date "+%Y-%m-%d %H:%M:%S")
  echo "更新时间 $logtime"
  exit
fi

cd autoWork
echo $PWD

workdir="master studio tf release"
for optdir in $workdir
do
  if [ -n "$branch" ] && [ "$branch" != "$optdir" ]
  then
    echo "当前分支$optdir 不是目标分支$branch"
    continue
  fi

  if [ ! -d "$optdir" ]
  then
    git clone git@hub000.xindong.com:ro/server.git $optdir
    echo "拉取分支$optdir"
  fi

  if [ -d "$optdir" ]
  then
    cd $optdir
    echo $PWD

    clientexedir=Executable
    git checkout $optdir
    echo "分支切换到$optdir"

    touch ../../test_autoMake.txt
    echo "代码更新: "
    echo `git pull`
    beginruntime=$(date "+%Y-%m-%d %H:%M:%S")
    echo "开始编译时间 $beginruntime"
    if [ "$optdir" = "master" ]; then
      make -j8 && echo "$optdir $optdir  $optdir  $optdir $optdir  $optdir"
      echo "trunk 分支编译"
    elif [ "$optdir" = "release" ]; then
      make publish -j8 &&  echo "$optdir $optdir  $optdir  $optdir $optdir  $optdir"
      echo "release 分支编译"
    else
      make $optdir -j8 && echo "$optdir $optdir $optdir $optdir $optdir"
      echo "$optdir 分支编译"
    fi
    endruntime=$(date "+%Y-%m-%d %H:%M:%S")
    echo "结束编译时间 $endruntime"

    if [ ! -d "$clientexedir" ]
    then
      mkdir $clientexedir
      echo "$clientexedir 文件夹创建"
    fi

    cd $clientexedir
    curhour=$(date "+ %H")
    if [ $curhour -eq 4 ]
    then
      rm -rf *
    fi

    if [ "$optdir" = "master" ]; then
      svn co svn://svn.sg.xindong.com/RO/server/bin/trunk
      clientexedir=Executable/trunk
      echo "$clientexedir svn创建"
    elif [ "$optdir" = "studio" ]; then
      svn co svn://svn.sg.xindong.com/RO/server/bin/studio
      clientexedir=Executable/studio
      echo "$clientexedir svn创建"
    elif [ "$optdir" = "tf" ]; then
      svn co svn://svn.sg.xindong.com/RO/server/bin/tf
      clientexedir=Executable/tf
      echo "$clientexedir svn创建"
    elif [ "$optdir" = "release" ]; then
      svn co svn://svn.sg.xindong.com/RO/server/bin/release
      clientexedir=Executable/release
      echo "$clientexedir svn创建"
    fi

    cd ..

    exefile=`find $exedir | grep Server$ | awk -F'/' '{print $3}'`
    echo $exefile
    for clientfile in $exefile
    do
      if [ -f "$exedir/$clientfile" ]
      then
        cp $exedir/$clientfile $clientexedir
        echo "copy $exedir/$clientfile $clientexedir"
      fi
    done

    svn add $clientexedir/*
    svn commit -m "[opt] : 服务器运行文件提交" $clientexedir/*

    cd ..
    echo $PWD
    time=$(date "+%Y-%m-%d %H:%M:%S")
    echo "$optdir 更新时间 $time"
  else
    echo "创建$optdir目录失败"
    rm ../test_autoMake.txt
    exit
  fi
done

cd .. 
rm test_autoMake.txt

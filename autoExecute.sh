#!/bin/sh

#将172.26.24.115上面4个分支里面的可执行文件
#覆盖当前分支的可执行文件

exedir=bin/Debug
outdir=ExeObj

branch=$(git branch | grep "^*" | sed "s/* //g")

if [ $branch != "master" -a $branch = "studio" -a $branch = "tf" -a $branch = "release" ]; then
  echo "$branch 分支错误"
  exit
fi

git pull

#更新配置
mkdir -p bin/Debug
if [ $branch = "master" ]; then
  ./update_resource.sh Debug client-trunk
elif [ $branch = "studio" ]; then
  ./update_resource.sh Debug Studio
elif [ $branch = "tf" ]; then
  ./update_resource.sh Debug TF_New
elif [ $branch = "release" ]; then
  ./update_resource.sh Debug Release_New
else
  echo "$branch 分支错误,无法更新配置"
  exit
fi

cp -au ./Config/* ./bin/Debug
cp -au ./trade ./bin/Debug/
rm -fr bin/Debug/map/.svn
rm -fr bin/Debug/sql_log/.svn
rm -fr bin/Debug/sql_main/.svn
rm -fr bin/Debug/sql_main/platform/.svn

cd $exedir

if [ ! -d "/$outdir" ]
then
  mkdir $outdir
  echo "$outdir 文件夹创建"
else
  echo "$outdir 文件夹已经存在"
fi

cd $outdir
clientexedir=trunk
if [ $branch = "master" ]; then
  #svn co svn://svn.sg.xindong.com/RO/server/bin/trunk
  clientexedir=trunk
  echo "$clientexedir svn创建"
elif [ $branch = "studio" ]; then
  #svn co svn://svn.sg.xindong.com/RO/server/bin/studio
  clientexedir=studio
  echo "$clientexedir svn创建"
elif [ $branch = "tf" ]; then
  #svn co svn://svn.sg.xindong.com/RO/server/bin/tf
  clientexedir=tf
  echo "$clientexedir svn创建"
elif [ $branch = "release" ]; then
  #svn co svn://svn.sg.xindong.com/RO/server/bin/release
  clientexedir=release
  echo "$clientexedir svn创建"
else
  echo "分支错误,无法拉取对应可执行文件"
fi

cd $clientexedir
svn up

cp -au ./* ../../
echo "********执行成功**********"

cd ..
cd ..


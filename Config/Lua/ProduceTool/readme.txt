###使用说明###

一 生成功能

1)Compare File
当点击Select File时, 可以选择Compare File, 选择的文件必须是之前生成的产出文件, 格式为csv. 如果成功选择了Compare File, 则在后续点击Produce生成时,会将Compare File作为对比文件, 在新生成的产出文件上包含对比信息.

2) Start Time To End Time
选择计算产出的时间.
当后面的use svn 勾选后, 在生成产出文件时, 会先利用svn 将本地配置更新到 StartTime, 然后使用StartTime的配置来计算产出.
如果没有勾选use svn, 则根据本地配置, 计算此时间段内的产出.

3) Set FileName
设置生成的产出文件名, 不可为空.

4) use svn
勾选成功后, 每次生成时, 会先将本地配置更新到 StartTime
未勾选时, 生成时使用本地配置


二 对比功能

1) select file1 & select file2
选择两个对比文件, 必须是产出文件, csv格式

2) compare
点击后, 对比选择的两个文件 file1.csv 与file2.csv, 并生成对比文件file1-file2.csv 在当前目录中


三 其他

1) window电脑, 若要使用 use svn 功能, 需检查计算机svn版本, 在当前目录svnExe中包含三个版本svn exe文件, 当前目录中的svn.exe为1.8.6版本
在勾选use svn 后, 如果提示版本不一致, 请根据安装版本, 使用对应的svn exe文件替换当前目录下svn.exe文件

2) 解压后, ProduceTool文件夹需放置在Cehua/Lua目录下,不能有二级目录

3）mac电脑直接点击Produce文件运行

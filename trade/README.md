# RO 交易所服务器

## 生产环境部署

CentOS 7/RHEL/Scientific Linux 7 x86_64 安装：

```
yum install -y https://mirrors4.tuna.tsinghua.edu.cn/remi/enterprise/remi-release-7.rpm
rpm --import https://rpms.remirepo.net/RPM-GPG-KEY-remi
yum --enablerepo remi,remi-php70 install -y php php-swoole php-msgpack php-redis php-mysqli php-ds php-yaml php-lua
```

服务器环境依赖

* php (>=7.0)
* swoole (>=1.9.6)
* php-yaml
* php-lua
* php-mysqli
* php-msgpack
* php-redis


## 服务器启动参数

`bin/trade-server` 服务器参数说明：

```
-p platName     平台名
-r regionName   区名
-n name         服务名，一般都是 TradeServer
-t              是否测试服
-v version      版本
-w workerNum    工作进程数，不设置则为当前cpu数（测试环境为2）
-d              守护进程模式
-c PATH         本地配置文件，开发用
--debug         打开debug模式
--timezone      时区，默认 PRC
--local         本地开发模式，使用dev-config目录配置，不重新编译策划表Lua文件到JSON文件
--ip 0.0.0.0    强制指定监听IP, 不设置则 0.0.0.0
--port 6800     强制指定监听端口，不设置则读 ro_global 数据库中配置端口
--log PATH      指定log路径
-h, --help      帮助
```

## 安全的停止服务器

`bin/trade-server stop -p xd -r r9`

`-p`, `-r` 参数同启动。

## 第一次使用流程

* 导入根目录 `trade.sql` 数据库文件；
* 执行迁移脚本；
* 启动服务器（参数见服务器启动参数）；

## 数据迁移

运行 `php bin/migrate-data.php -pxd -rr9 -v1.0.15` 其中参数 -p, -r -v 和启动交易所相同。

## 本地开发
先安装好 `php`，再安装 `composer` (see [https://getcomposer.org/download/](https://getcomposer.org/download/)) 快速安装 `composer`：

```bash
php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');"
php composer-setup.php
php -r "unlink('composer-setup.php');"
mv composer.phar /usr/local/bin/composer
```

如果proto文件发生修改，则需要 **编译 protobuf 文件**。

执行 `sh bin/builder.sh` 即可全部编译完成，如果要添加或删除文件，修改这个文件即可。

目前用到的协议文件包括：

* `SceneTrade.proto`
* `SceneItem.proto`
* `SocialCmd.proto`
* `SceneUser2.proto`
* `SessionCmd.proto`
* `LogCmd.proto`
* `SystemCmd.proto`

优化 composer 生产服务器运行效率（`sh bin/builder.sh` 在执行时会自动运行过，不要额外执行）

`composer dump-autoload --optimize`

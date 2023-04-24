<?php
namespace RO\Trade;

use Swoole\Lock;

class Server extends \MyQEE\Server\Server
{
    /**
     * 平台ID
     *
     * @var string
     */
    public static $platId;

    /**
     * 平台名
     *
     * 例如: xd
     *
     * @var string
     */
    public static $platName;

    /**
     * 区Id
     *
     * @var string
     */
    public static $regionId;

    /**
     * 区名
     *
     * 例如: r9
     *
     * @var string
     */
    public static $regionName;

    /**
     * 交易所服务器是否开通的标记，进程启动时会更新
     *
     * @var bool
     */
    public static $isOpen = true;

    /**
     * 正在购买的排队数
     *
     * @var \Swoole\Atomic
     */
    public static $counterBuying;

    /**
     * 启动后购买成功次数
     *
     * 真正的请求数为 `$this->counterSuccessCarry->get() * 200000000 + $this->counterSuccess->get()`
     * 请使用 `$this->counterSuccess()` 直接获取;
     *
     * @var \Swoole\Atomic
     */
    public static $counterSuccess;

    /**
     * @var \Swoole\Atomic
     */
    public static $counterSuccessX;

    /**
     * 购买失败数
     *
     * @var \Swoole\Atomic
     */
    public static $counterFail;

    /**
     * @var \Swoole\Atomic
     */
    public static $stockUpPos;

    /**
     * 物品信息多进程共享表，key 为 itemid
     *
     * @var \Swoole\Table
     */
    public static $item;

    /**
     * 缓存在线玩家数据
     *
     * charid, zoneid, name, level
     *
     * @var \Swoole\Table
     */
    public static $players;

    /**
     * 场景服务器列表
     *
     * @var \Swoole\Table
     */
    public static $zone;

    /**
     * 任务进程状态表
     *
     * @var \Swoole\Table
     */
    public static $taskStatus;

    /**
     * ItemList 表
     *
     *
     * @var \Swoole\Table
     */
    public static $itemList;

    /**
     * ItemList Id对应的共享表
     *
     * @var \Swoole\Table
     */
    public static $itemListId2KeyMap;

    /**
     * ItemList Id对应表的进程当前自己的数据
     *
     * @var \Ds\Map
     */
    public static $itemListId2KeyMapCached;

    /**
     * 快速兑换记录表
     *
     * @var \Swoole\Table
     */
    public static $userLastConvertId;

    /**
     * 购买中计数, key 是 item_list 表对应的 Id
     *
     * @var \Swoole\Table
     */
    public static $itemBuying;

    /**
     * 待更新库存的挂单
     *
     * @var \Swoole\Table
     */
    public static $updatingGoodsStock;

    /**
     * 已加载的策划表hash值
     *
     * @var array
     */
    public static $configTableHash = [];

    /**
     * 每个Task分配的一个高性能内存队列
     *
     * @var \Ds\Vector
     */
    public static $taskChannel;

    /**
     * 消息通知队列
     *
     * @var \Swoole\Channel
     */
    //public static $noticeChannel;

    /**
     * 获取列表列队
     *
     * @var \Swoole\Channel
     */
    public static $listChannel;

    /**
     * Fluent 推送通道
     *
     * @var \Swoole\Channel
     */
    public static $fluentChannel;

    /**
     * Fluent 调价推送通道
     *
     * @var \Swoole\Channel
     */
    public static $adjustFluentChannel;

    /**
     * 更新库存到数据库通道
     *
     * @var \Swoole\Channel
     */
    public static $stockUpdateChannel;

    /**
     * @var \RO\Redis|\Redis
     */
    public static $redis;

    /**
     * 服务器配置
     *
     * （从游戏服务器的 GameConfig 文件中读取）
     *
     * @var array
     */
    public static $configExchange;

    /**
     * 摆摊服务器配置
     *
     * （从游戏服务器的 GameConfig 文件中读取）
     *
     * @var array
     */
    public static $configBooth;

    /**
     * @var \RO\MySQLi
     */
    public static $mysqlMaster;

    /**
     * 程序根目录
     *
     * @var string
     */
    public static $baseDir;

    /**
     * 当前数据目录
     *
     * @var string
     */
    public static $dataDir;

    /**
     * 日志目录
     *
     * @var string
     */
    public static $logDir;

    /**
     * JSON配置目录
     *
     * @var string
     */
    public static $configPath;

    /**
     * 统计服务器
     *
     * 例如: [127.0.0.1, 7131]
     *
     * @var string|null
     */
    public static $statServer = null;

    /**
     * 分支ID
     *
     * 0 - 内网开发
     *
     * @var int
     */
    public static $branchID = 0;

    /**
     * 分支ID所对应的Key
     *
     * @var array
     */
    public static $branchKeys = [
            0 => 'Debug',
            // 内网
            1 => 'TF',
            // 预言之地
            2 => 'Publish',
            // 正式服
        ];

    /**
     * 公示锁
     *
     * @var \Swoole\Lock
     */
    public static $publicityLock = null;

    /** @var string 公示锁文件 */
    public static $publicityLockFile = 'publicity.lock';

    /**
     * 处理ItemList的进程数
     *
     * 启动时会自动根据进程数创建
     *
     * @var int
     */
    protected static $itemListTaskCount = 0;

    /**
     * taskId = 0 的进程处理列表输出，不处理 ItemList
     * taskId = 2 的进程处理延迟更新sql，不处理 ItemList
     *
     * @var int
     */
    protected static $itemListTaskBegin = 2;

    /**
     * 原来可以交易的物品被设为不能交易的集合
     *
     * @var array
     */
    public static $cannotTradeItems = [];

    public static $delayBuyFileName = 'delay-buy.job';

    /**
     * 最后的错误信息
     *
     * @var
     */
    public static $lastError = [];

    const DEFAULT_MONEY_TYPE = 131;

    const EREDISKEYTYPE_GLOBAL_CHAR_DATA = 'gchardata';

    const SENTRY_ITEM_LIST_ID = 1000000000;

    protected function checkConfig()
    {
        if (version_compare(SWOOLE_VERSION, '1.9.11', '<'))
        {
            echo '必须 1.9.11 以上版本 Swoole';
            exit;
        }

        if (!extension_loaded('ds'))
        {
            echo '必须安装ds扩展';
            exit;
        }
        if (!extension_loaded('lua'))
        {
            echo '必须安装lua扩展';
            exit;
        }

        global $options;

        if (!isset($options['p']))
        {
            $options['p'] = 'xd';
        }

        if (!isset($options['r']))
        {
            $options['r'] = 'r1';
        }

        self::$platName   = $options['p'];
        self::$regionName = $options['r'];
        self::$baseDir    = realpath(__DIR__ . '/../../') . '/';

        # 读取 branch 版本
        if (isset($options['local']))
        {
            self::$branchID = 0;

            # 读取配置文件
            $sockPath         = '/tmp/';
            $logDir           = self::$baseDir . 'log/';
            self::$logDir     = $logDir;
            self::$dataDir    = self::$baseDir . 'data/';
            self::$configPath = self::$baseDir . 'config-dev/';

            # 使用当前目录
            if (!is_dir(self::$dataDir))
            {
                mkdir(self::$dataDir);
            }

            if (!is_dir($logDir))
            {
                mkdir($logDir);
            }

            try
            {
                # 尝试生成配置, 如果../Lua目录存在
                self::formatLua2json(true);
            }
            catch (\Exception $e)
            {
                echo '[生成配置错误] ' . $e->getMessage();
                exit;
            }
        }
        else
        {
            # 使用服务器上目录
            $logDir           = "/data/rogame/log/" . self::$platName . '/' . self::$regionName . '/';
            $dataDir          = "/data/rogame/data/" . self::$platName . '/' . self::$regionName . '/';
            $configDir        = $dataDir . 'config-' . (isset($options['v']) ? $options['v'] : 'unknown') . '/';
            $sockPath         = $dataDir;
            self::$dataDir    = $dataDir;
            self::$configPath = $configDir;
            self::$logDir     = $logDir;

            $parentDir = realpath(self::$baseDir . '../') . '/';
            if (!is_file("{$parentDir}branch.lua"))
            {
                echo "配置 {$parentDir}branch.lua 文件不存在，无法启动";
                exit;
            }

            try
            {
                $lua            = new \Lua();
                self::$branchID = intval($lua->eval(file_get_contents("{$parentDir}branch.lua") . "\nreturn BranchID.id"));
                unset($lua);
            }
            catch (\Exception $e)
            {
                echo '解析 branch.lua 配置失败, ' . $e->getMessage();
                exit;
            }

            if (!is_dir($logDir))
            {
                if (false === mkdir($logDir, 0755, true))
                {
                    echo "日志目录 {$logDir} 无法创建，不能启动\n";
                    exit;
                }
            }

            if (!is_dir($dataDir))
            {
                if (false === mkdir($dataDir, 0755, true))
                {
                    echo "数据目录 {$dataDir} 无法创建，不能启动\n";
                    exit;
                }
            }

            if (!is_dir($configDir))
            {
                if (false === mkdir($configDir, 0755, true))
                {
                    echo "配置目录 {$configDir} 无法创建，不能启动\n";
                    exit;
                }
            }

            try
            {
                # 重新生成配置
                self::formatLua2json();
            }
            catch (\Exception $e)
            {
                echo '[生成配置错误] ' . $e->getMessage();
                exit;
            }
        }

        if (self::$branchID == 0 && !isset($options['w']))
        {
            $this->config['server']['worker_num'] = 2;
            $this->config['task']['number']       = 3;
        }

        if (!isset(self::$branchKeys[self::$branchID]))
        {
            echo "未知 BranchID.id (" . self::$branchID . ")\n";
            exit;
        }

        if (!isset($this->config['log']))
        {
            $this->config['log'] = [
                'level' => [
                    'warn',
                    'info'
                ]
            ];
        }
        elseif (0 === self::$branchID)
        {
            $this->config['log']['level'][] = 'info';
            $this->config['log']['level']   = array_unique($this->config['log']['level']);
        }

        # 读取配置
        $config = self::loadBranchConfig();
        if (!isset($this->config['mysql']))
        {
            $this->config['mysql'] = $config['mysql'];
        }
        if (!isset($this->config['redis']))
        {
            $this->config['redis'] = $config['redis'];
        }
        if (!isset($this->config['globalDB']))
        {
            $this->config['globalDB'] = $config['globalDB'];
        }
        if (!isset($this->config['gameDb']))
        {
            $this->config['gameDb'] = $config['gameDb'];
        }

        # 守护进程模式
        if (1 == $this->config['swoole']['daemonize'] || !isset($options['local']))
        {
            $this->config['swoole']['pid_file'] = self::$dataDir . 'TradePHP.pid';
            $this->config['log']['path']        = $logDir . 'TradePHP.log';
        }

        if (isset($options['log']) && $options['log'])
        {
            $dir = dirname($options['log']);
            if (!is_dir($dir))
            {
                if (false === mkdir($dir, 0755, true))
                {
                    echo("目录 $dir 不可写");
                    exit;
                }
            }
            $this->config['log']['path'] = $options['log'];
        }

        # 配置log相关
        $this->checkConfigForLog();

        $mysql = new \RO\MySQLi($this->config['globalDB']);
        # 读取平台ID
        $sql = "SELECT `platid` FROM `platform` WHERE `platname` = " . $mysql->quote(self::$platName);
        if ($rs = $mysql->query($sql))
        {
            if ($rs->num_rows > 0)
            {
                $row          = $rs->fetch_object();
                self::$platId = (int)$row->platid;
            }
            else
            {
                $this->warn("不存在指定的平台: " . self::$platName);
                exit;
            }
            $rs->free();
        }
        else
        {
            $this->warn("SQL: $sql, Error: " . $mysql->error);
            exit;
        }

        # 读取区
        $sql = "SELECT `regionid`, `platid`, `regionname` FROM `region` WHERE `platid` = " . $mysql->quote(self::$platId) . ' AND `regionname` = ' . $mysql->quote(self::$regionName);
        if ($rs = $mysql->query($sql))
        {
            if ($rs->num_rows > 0)
            {
                $row            = $rs->fetch_object();
                self::$regionId = (int)$row->regionid;
            }
            else
            {
                $this->warn("不存在指定的区: " . self::$regionName);
                exit;
            }
            $rs->free();
        }
        else
        {
            $this->warn("SQL: $sql, Error: " . $mysql->error);
            exit;
        }

        if ($this->config['hosts']['Main']['port'] == -1)
        {
            # 读取要启动的端口设置
            $name = isset($options['n']) && $options['n'] ? $options['n'] : 'trade-server';
            $sql  = "SELECT `port` FROM `region_svrlist` WHERE `regionid` = " . $mysql->quote(self::$regionId) . " AND `servertype` = '{$name}'";
            if ($rs = $mysql->query($sql))
            {
                if ($rs->num_rows > 0)
                {
                    $row                                   = $rs->fetch_object();
                    $this->config['hosts']['Main']['port'] = (int)$row->port;

                    # --local模式下根据交易所配置端口改变http和shell端口
                    if (isset($options['local']))
                    {
                        $this->config['hosts']['Http']['port'] = (int)$row->port + 20000;
                        $this->config['remote_shell']['port']  = (int)$row->port + 20001;
                    }
                }
                else
                {
                    $this->warn("不存在指定的交易所设置, platName: " . self::$platName . ', regionName: ' . self::$regionName . ', serverType: ' . $name);
                    exit;
                }
                $rs->free();
            }
            else
            {
                $this->warn("SQL: $sql, Error: " . $mysql->error);
                exit;
            }
        }

        $sql = "SELECT `ip`, `port` FROM `region_svrlist` WHERE `regionid` = " . $mysql->quote(self::$regionId) . " AND `servertype` = 'StatServer'";
        if ($rs = $mysql->query($sql))
        {
            if ($rs->num_rows > 0)
            {
                $row              = $rs->fetch_object();
                self::$statServer = [
                    $row->ip,
                    (int)$row->port
                ];
            }
            $rs->free();
        }
        else
        {
            $this->warn("SQL: $sql, Error: " . $mysql->error);
            exit;
        }

        $mysql->close();
        unset($mysql);

        if (0 === self::$branchID)
        {
            $this->info("当前启动的模式是开发模式");
        }

        parent::checkConfig();
    }

    public function onBeforeStart()
    {
        if ($this->config['swoole']['task_worker_num'] > 128)
        {
            $this->warn('任务进程数分配的太多了，有：' . $this->config['swoole']['task_worker_num']);
            exit;
        }

        self::$configExchange                    = self::loadConfig('ExchangeConfig');
        self::$configTableHash['ExchangeConfig'] = self::getConfigHash('ExchangeConfig');
        if (!self::$configExchange)
        {
            $this->warn('加载交易所配置 ExchangeConfig.json 失败');
            exit;
        }

        self::$configBooth                    = self::loadConfig('BoothConfig');
        self::$configTableHash['BoothConfig'] = self::getConfigHash('BoothConfig');
        if (!self::$configBooth)
        {
            $this->warn('加载交易所配置 BoothConfig.json 失败');
            exit;
        }

        # listList投递的总Task数
        self::$instance->config['swoole']['task_worker_num'] = max(3, self::$instance->config['swoole']['task_worker_num']);
        self::$itemListTaskCount                             = self::$instance->config['swoole']['task_worker_num'] - self::$itemListTaskBegin;

        self::$counterBuying   = new \Swoole\Atomic();
        self::$counterSuccess  = new \Swoole\Atomic();
        self::$counterSuccessX = new \Swoole\Atomic();
        self::$counterFail     = new \Swoole\Atomic();
        self::$stockUpPos      = new \Swoole\Atomic();

        # 物品表信息
        self::$item = new \Swoole\Table(1024 * 256);
        self::$item->column('isTrade', \Swoole\Table::TYPE_INT, 1);             # 是否售卖
        self::$item->column('isOverlap', \Swoole\Table::TYPE_INT, 1);           # 是否堆叠物品
        self::$item->column('buying', \Swoole\Table::TYPE_INT, 4);              # 有人正在购买
        self::$item->column('sellingNum', \Swoole\Table::TYPE_INT, 2);          # 正在卖的数量
        self::$item->column('soldNum', \Swoole\Table::TYPE_INT, 4);             # 近期成交数,该数值会由一个进程管理
        self::$item->column('price', \Swoole\Table::TYPE_INT, 8);               # 单价
        self::$item->column('maxPrice', \Swoole\Table::TYPE_INT, 8);            # 最高价格, 负数代表不设置
        self::$item->column('upRatio', \Swoole\Table::TYPE_FLOAT, 4);           # 涨幅系数
        self::$item->column('downRatio', \Swoole\Table::TYPE_FLOAT, 4);         # 跌幅系数
        self::$item->column('stock', \Swoole\Table::TYPE_INT, 4);               # 总库存
        self::$item->column('boothStock', \Swoole\Table::TYPE_INT, 4);          # 摆摊总库存
        self::$item->column('priceUpdateTime', \Swoole\Table::TYPE_INT, 4);     # 价格最后更新时间
        self::$item->column('cycle', \Swoole\Table::TYPE_INT, 4);               # 价格调价周期
        self::$item->column('refineCycle', \Swoole\Table::TYPE_INT, 1);         # 精炼调价周期
        self::$item->column('exchangeNum', \Swoole\Table::TYPE_INT, 4);         # 公示期交易量，在售物品少于此量的将为公示物品
        self::$item->column('equipType', \Swoole\Table::TYPE_INT, 4);           # 装备类型, 不是装备为0
        self::$item->column('minPriceType', \Swoole\Table::TYPE_INT, 1);        # 最低价格计算类型
        self::$item->column('equipUpgradeId', \Swoole\Table::TYPE_INT, 4);      # 升级装备ID
        self::$item->column('mainEquipId', \Swoole\Table::TYPE_INT, 4);         # 合成主装备ID
        self::$item->column('itemType', \Swoole\Table::TYPE_INT, 4);            # 物品类型
        self::$item->column('publicityShowTime', \Swoole\Table::TYPE_INT, 4);   # 公示物品公示时间
        self::$item->column('category', \Swoole\Table::TYPE_INT, 4);            # 物品分类ID
        self::$item->column('fashionType', \Swoole\Table::TYPE_INT, 1);         # 时装类型
        self::$item->column('moneyType', \Swoole\Table::TYPE_INT, 2);           # 消费类型, zeny
        self::$item->column('publicityNum', \Swoole\Table::TYPE_INT, 1);        # 公示数
        self::$item->column('lastPubEndTime', \Swoole\Table::TYPE_INT, 4);      # 最新公示结束时间
        self::$item->column('name', \Swoole\Table::TYPE_STRING, 128);           # 名称
        self::$item->column('tradeTime', \Swoole\Table::TYPE_INT, 4);           # 允许交易时间
        self::$item->column('unTradeTime', \Swoole\Table::TYPE_INT, 4);         # 允许交易结束时间
        self::$item->create();

        $itemListSize = IS_DEBUG ? 1024 * 256 : 1024 * 2048;
        # 物品列表信息
        self::$itemList = new \Swoole\Table($itemListSize);
        self::$itemList->column('id', \Swoole\Table::TYPE_INT, 4);              # ID
        self::$itemList->column('item_id', \Swoole\Table::TYPE_INT, 4);         # 物品id
        self::$itemList->column('stock', \Swoole\Table::TYPE_INT, 4);           # 库存
        self::$itemList->column('boothStock', \Swoole\Table::TYPE_INT, 4);      # 摆摊库存
        self::$itemList->column('is_publicity', \Swoole\Table::TYPE_INT, 1);    # 是否公示
        self::$itemList->column('pub_buy_people', \Swoole\Table::TYPE_INT, 4);  # 抢购人数
        self::$itemList->column('pub_price', \Swoole\Table::TYPE_INT, 8);       # 公示价格
        self::$itemList->column('start_time', \Swoole\Table::TYPE_INT, 4);      # 公示期开始时间
        self::$itemList->column('end_time', \Swoole\Table::TYPE_INT, 4);        # 公示期结束时间
        self::$itemList->column('delay_time', \Swoole\Table::TYPE_INT, 4);      # 公示期延长时间
        self::$itemList->column('refine_lv', \Swoole\Table::TYPE_INT, 1);       # 精炼等级
        self::$itemList->column('is_damage', \Swoole\Table::TYPE_INT, 1);       # 是否损坏
        self::$itemList->column('is_good_enchant', \Swoole\Table::TYPE_INT, 1); # 是否好的附魔
        self::$itemList->column('isOverlap', \Swoole\Table::TYPE_INT, 1);       # 是否堆叠物品
        self::$itemList->column('isResetStock', \Swoole\Table::TYPE_INT, 1);    # 服务器启动后是否重置过库存
        self::$itemList->column('item_data', \Swoole\Table::TYPE_STRING, 512);  # itemData
        self::$itemList->create();

        self::$itemListId2KeyMap = new \Swoole\Table($itemListSize);
        self::$itemListId2KeyMap->column('key', \Swoole\Table::TYPE_STRING, 32);
        self::$itemListId2KeyMap->create();
        self::$itemListId2KeyMapCached = new \Ds\Map();
        self::$itemListId2KeyMapCached->allocate(1024 * 16);

        # 购买中计数, key 是 item_list 表对应的 Id
        self::$itemBuying = new \Swoole\Table(self::$branchID == 0 ? 1024 : 1024 * 256);
        self::$itemBuying->column('count', \Swoole\Table::TYPE_INT, 4);
        self::$itemBuying->create();

        # 任务进程状态
        self::$taskStatus = new \Swoole\Table(128);
        self::$taskStatus->column('queue', \Swoole\Table::TYPE_INT, 4);       # 处理购买请求的队列数
        self::$taskStatus->create();

        # 兑换记录临时表
        self::$userLastConvertId = new \Swoole\Table(self::$branchID == 0 ? 64 : 1024 * 1024);
        self::$userLastConvertId->column('id', \Swoole\Table::TYPE_INT, 8);       # 兑换记录ID
        self::$userLastConvertId->column('time', \Swoole\Table::TYPE_INT, 4);       # 时间
        self::$userLastConvertId->create();

        # 用户表缓存
        self::$players = new \Swoole\Table(self::$branchID == 0 ? 128 : 1024 * 1024);
        self::$players->column('zoneId', \Swoole\Table::TYPE_INT, 8);            # 区ID
        self::$players->column('pendingLimit', \Swoole\Table::TYPE_INT, 1);      # 交易所挂单最大上限
        self::$players->column('returnRate', \Swoole\Table::TYPE_INT, 4);        # 下架时返回的费用
        self::$players->column('quota', \Swoole\Table::TYPE_INT, 8);             # 玩家当前额度
        self::$players->column('boothPendingLimit', \Swoole\Table::TYPE_INT, 1); # 摆摊挂单最大上限
        self::$players->column('boothOpenStatus', \Swoole\Table::TYPE_INT, 1);       # 摆摊是否开启
        self::$players->create();

        # 服务器列表
        self::$zone = new \Swoole\Table(self::$branchID == 0 ? 32 : 1024 * 8);
        self::$zone->column('fd', \Swoole\Table::TYPE_INT, 8);
        self::$zone->column('fromId', \Swoole\Table::TYPE_INT, 1);
        self::$zone->create();

        # 待更新挂单列表，当一个购买请求涉及到非常多的挂单时会采用延迟更新的策略，此数据记录了待延迟更新的数量
        self::$updatingGoodsStock = new \Swoole\Table(self::$branchID == 0 ? 128 : 1024 * 1024);
        self::$updatingGoodsStock->column('count', \Swoole\Table::TYPE_INT, 4);
        self::$updatingGoodsStock->create();

        self::$listChannel         = new \Swoole\Channel(self::$branchID == 0 ? 1024 * 1024 : 1024 * 1024 * 30);        # 处理列表的消息队列
        self::$fluentChannel       = new \Swoole\Channel(self::$branchID == 0 ? 1024 * 1024 * 8 : 1024 * 1024 * 100);   # 处理推送Fluent
        self::$adjustFluentChannel = new \Swoole\Channel(self::$branchID == 0 ? 1024 * 1024 : 1024 * 1024 * 8);         # 处理推送调价日志Fluent
        self::$stockUpdateChannel  = new \Swoole\Channel(self::$branchID == 0 ? 1024 * 1024 : 1024 * 1024 * 30);        # 处理更新库存

        # 创建一个公示处理锁
        self::$publicityLock = new Lock(SWOOLE_FILELOCK, self::$dataDir . self::$publicityLockFile);

        # 为每个Task创建一个内存列队
        self::$taskChannel = new \Ds\Vector();
        $size              = self::$branchID == 0 ? 1024 * 1024 * 2 : 1024 * 1024 * 10;       # 每个列队内存占用数（字节）
        for ($i = 0; $i < $this->config['swoole']['task_worker_num']; $i++)
        {
            try
            {
                self::$taskChannel->push(new \Swoole\Channel($size));
            }
            catch (\Exception $e)
            {
                self::$instance->warn('分配列队通道失败，无法启动，可能内存不足');
                exit;
            }
        }

        # 处理异步更新库存任务
        if (glob(self::$dataDir . 'delay-*.job'))
        {
            self::doDelayJobAtStart();
        }

        # 加载模块
        //$load = swoole_load_module(realpath(__DIR__.'/../../cpp/d3dex').'/ro_des_p'. PHP_VERSION .'_s'. SWOOLE_VERSION .'.so');
        //if (false === $load)
        //{
        //    include __DIR__ .'/../../ide_helper/ro_enc.php';
        //}

        # 加载RO配置
        try
        {
            self::initData();
        }
        catch (\Exception $e)
        {
            $this->warn('[服务启动] 加载配置发生错误, 错误信息:' . $e->getMessage() . ' 错误行数:' . $e->getLine() . ' 错误文件:' . $e->getFile());
            if (IS_DEBUG)
            {
                throw $e;
            }

            exit;
        }

        set_error_handler(function($errno, $errstr, $errfile, $errline)
        {
            static $sameErrorNum = 0;
            if (isset(self::$lastError[1]) && self::$lastError[1] === $errstr && self::$lastError[3] === $errline)
            {
                # 相同错误
                $sameErrorNum++;
            }
            elseif ($sameErrorNum > 0)
            {
                $this->warn("#{$this->pid} Error Handler: no: $errno, err: $errstr, file: $errfile, line: $errline");
            }

            self::$lastError = [
                $errno,
                $errstr,
                $errfile,
                $errline
            ];
            $this->warn("#{$this->pid} Error Handler: no: $errno, err: $errstr, file: $errfile, line: $errline");

            return true;
        });
    }

    /**
     * 成功数
     *
     * @return int
     */
    public static function counterSuccess()
    {
        return self::$counterSuccessX->get() * 200000000 + self::$counterSuccess->get();
    }

    /**
     * 加载策划表
     *
     * @return array
     */
    public static function loadTableConfig()
    {
        $changed = [];
        $name    = 'ExchangeConfig';
        $hash    = self::getConfigHash($name);
        if (!isset(self::$configTableHash[$name]) || self::$configTableHash[$name] != $hash)
        {
            self::$configExchange         = self::loadConfig($name);
            self::$configTableHash[$name] = $hash;
            $changed[]                    = $name;
        }

        $list = [
            'RoleData'          => 'RoleDataConfig',
            'EquipRefine'       => 'RefineTradeConfig',
            'Equip'             => 'EquipConfig',
            'EquipEnchant'      => 'EnchantConfig',
            'EquipEnchantPrice' => 'EnchantPriceConfig',
            'EquipUpgrade'      => 'EquipUpgradeConfig',
        ];

        try
        {
            foreach ($list as $name => $fun)
            {
                $hash = self::getConfigHash($name);
                $funF = "\\RO\\Trade\\$fun";
                if (!isset(self::$configTableHash[$name]) || self::$configTableHash[$name] != $hash)
                {
                    /**
                     * @var mixed $funF
                     */
                    $funF::loadConfig(self::loadConfig($name));
                    self::$configTableHash[$name] = $hash;
                    $changed[]                    = $name;
                }
            }
        }
        catch (\Exception $e)
        {
            Server::$instance->warn($e->getMessage());
            if (isset(Server::$instance->server))
            {
                Server::$instance->server->shutdown();
            }
            else
            {
                exit;
            }
        }

        return $changed;
    }

    /**
     * 更新物品内存表信息
     *
     * 在价格管理进程启动后会执行
     */
    protected static function initData()
    {
        $sTime    = $begin = microtime(1);
        $exchange = self::loadConfig('Exchange');
        if (!$exchange)
        {
            self::$instance->warn('读取 Exchange.json 配置失败');
            exit;
        }

        $equip = self::loadConfig('Equip');
        if (!$equip)
        {
            self::$instance->warn('读取 Equip.json 配置失败');
            exit;
        }

        $itemConfig = self::loadConfig('Item');
        if (!$itemConfig)
        {
            self::$instance->warn('读取 Item.json 配置失败');
            exit;
        }

        $equipComposeConfig = Server::loadConfig('EquipCompose');
        if (!$equipComposeConfig)
        {
            self::$instance->warn('读取 EquipCompose.json 配置失败');
            exit;
        }

        $mysql = self::createMySQL(true);
        $redis = self::createRedis();
        if (!$mysql || !$redis)
        {
            exit;
        }

        self::$instance->debug("配置加载耗时: " . (microtime(1) - $begin));

        $time = microtime(true);
        foreach (glob(self::$dataDir . '*.sql') as $file)
        {
            if (!self::sqlFileExecute($file, $mysql))
            {
                self::$instance->warn("执行 $file 失败");
                exit;
            }
        }
        self::$instance->debug("处理遗留数据耗时: " . (microtime(1) - $time));

        self::$instance->info("正在加载最近成交量...");
        # 读取近期成交量
        $begin       = microtime(1);
        $currentTime = time() - self::$configExchange['LogTime'];
        $sql         = "SELECT SUM(`count`) as `soldNum`, `item_id` AS `itemId` FROM `trade_record_sold` WHERE `time` >= {$currentTime} GROUP BY `item_id`";
        $result      = $mysql->query($sql);
        $soldNum     = [];
        if ($result)
        {
            while ($row = $result->fetch_object())
            {
                $soldNum[$row->itemId] = (int)$row->soldNum;
            }

            $result->free();
            unset($result);
        }
        else
        {
            self::$instance->warn("SQL: {$sql}, Error: {$mysql->error}");
            exit;
        }
        self::$instance->info("最近成交量加载耗时: " . (microtime(1) - $begin));

        # 物品是否堆叠信息
        $overlap  = [];
        $begin    = microtime(1);
        $items    = [];
        # 读取物品信息
        $result = $mysql->query('SELECT * FROM `trade_item_info`');
        if ($result)
        {
            while ($row = $result->fetch_assoc())
            {
                $itemId = $row['itemid'];

                if (!isset($exchange[$itemId]))
                {
                    self::$instance->info("[加载配置] Exchange 策划表中不存在ItemId:{$itemId}的物品, 但trade_item_info数据表中存在该记录");
                    if (time() - $row['last_calc_price_time'] > 86400 * 7)
                    {
                        $mysql->query("DELETE FROM `trade_item_info` WHERE `itemid` = '{$itemId}'");
                    }
                    continue;
                }

                if (!isset($itemConfig[$itemId]))
                {
                    self::$instance->warn("[加载配置] Exchange 策划表配置异常，{$itemId} 在 Item 中不存在");
                }

                # 记录原可以交易转为不能交易的物品id
                if ($row['is_trade'] == 1 && $exchange[$itemId]['Trade'] == 0)
                {
                    self::$cannotTradeItems[] = $itemId;
                    if (IS_DEBUG)
                    {
                        self::$instance->debug("[加载配置] 新配置对物品:{$itemId} 修改为不能交易");
                    }
                }
                else if ($row['is_trade'] == 0 && $exchange[$itemId]['Trade'] == 1)
                {
                    $mysql->query("UPDATE `trade_item_info` SET `is_trade` = 1 WHERE `itemid` = '{$itemId}'");
                }

                $items[$itemId] = [
                    'price'           => (int)$row['last_server_price'],
                    'maxPrice'        => (int)$row['max_price'],
                    'upRatio'         => (float)$row['up_ratio'],
                    'downRatio'       => (float)$row['down_ratio'],
                    'priceUpdateTime' => (int)$row['last_calc_price_time'],
                    'cycle'           => (int)$row['t'],
                ];
            }

            # 释放对象
            $result->free();
            unset($result);
        }
        else
        {
            self::$instance->warn("SQL: {$mysql->last_query}, Error: {$mysql->error}");
            exit;
        }

        foreach ($exchange as $itemId => $itemExc)
        {
            $branchKey   = self::$branchKeys[self::$branchID];
            if ($branchKey === 'TF') {
                $tradeTime   = isset($itemExc['TFTradeTime']) ? strtotime($itemExc['TFTradeTime']) ?: 0 : 0;
                $unTradeTime = isset($itemExc['TFUnTradeTime']) ? strtotime($itemExc['TFUnTradeTime']) ?: 0 : 0;
            } else {
                $tradeTime   = isset($itemExc['TradeTime']) ? strtotime($itemExc['TradeTime']) ?: 0 : 0;
                $unTradeTime = isset($itemExc['UnTradeTime']) ? strtotime($itemExc['UnTradeTime']) ?: 0 : 0;
            }

            $minPriceType = $itemExc['MinPrice']['type'] ?? ExchangeItemNode::MIN_PRICE_TYPE_SELF;
            $mainEquipId  = 0;
            if ($minPriceType === ExchangeItemNode::MIN_PRICE_TYPE_EQUIP_NEW_COMPOSE)
            {
                $id = $itemExc['MinPrice']['equipcomposeid'] ?? 0;
                $mainEquipId = $equipComposeConfig[$id]['Material'][1]['id'] ?? 0;
            }

            $data = [
                'isTrade'           => isset($itemExc['Trade']) ? (int)$itemExc['Trade'] : 0,
                'isOverlap'         => isset($itemExc['Overlap']) ? (int)$itemExc['Overlap'] : 0,
                'name'              => $itemExc['NameZh'],
                'buying'            => 0,
                'soldNum'           => isset($soldNum[$itemId]) ? $soldNum[$itemId] : 0,
                'price'             => 0,
                'maxPrice'          => 0,
                'upRatio'           => 1,
                'downRatio'         => 1,
                'priceUpdateTime'   => 0,
                'cycle'             => 0,
                'refineCycle'       => isset($itemExc['RefineCycle']) ? (int)$itemExc['RefineCycle'] : 0,
                'exchangeNum'       => isset($itemExc['ExchangeNum']) ? (int)$itemExc['ExchangeNum'] : 0,
                'equipType'         => isset($equip[$itemId]['EquipType']) ? (int)$equip[$itemId]['EquipType'] : 0,
                'minPriceType'      => $minPriceType,
                'mainEquipId'       => $mainEquipId,
                'equipUpgradeId'    => isset($itemExc['MinPrice']) ? isset($itemExc['MinPrice']['equip_upgrade_id']) ? (int)$itemExc['MinPrice']['equip_upgrade_id'] : 0 : 0,
                'itemType'          => isset($itemConfig[$itemId]['Type']) ? (int)$itemConfig[$itemId]['Type'] : 0,
                'publicityShowTime' => isset($itemExc['ShowTime']) ? (int)$itemExc['ShowTime'] : 0,
                'category'          => isset($itemExc['Category']) ? (int)$itemExc['Category'] : 0,
                'fashionType'       => isset($itemExc['FashionType']) ? (int)$itemExc['FashionType'] : 0,
                'moneyType'         => isset($itemExc['MoneyType']) ? (int)$itemExc['MoneyType'] : 131,
                'tradeTime'         => $tradeTime,
                'unTradeTime'       => $unTradeTime,
            ];
            $overlap[$itemId]  = $data['isOverlap'];

            if (isset($items[$itemId]))
            {
                $data = $items[$itemId] + $data;
            }
            else
            {
                if (1 == $data['isTrade'])
                {
                    # 更新到数据库
                    if (!$mysql->query("INSERT INTO `trade_item_info` (`itemid`) VALUES ('" . $itemId . "')"))
                    {
                        self::$instance->warn("SQL: {$mysql->last_query}, Error: {$mysql->error}");
                        exit;
                    }
                }
            }

            self::$item->set($itemId, $data);
        }
        self::$instance->info("初始化ItemTable，耗时: " . (microtime(1) - $begin));

        self::$instance->info("正在初始化ItemList, 请稍候...");
        $begin               = microtime(1);
        $exitItemListItemKey = [];
        $exitItemListRs      = $mysql->query("SELECT `item_key` from `trade_goods` group by `item_key`");
        if ($exitItemListRs)
        {
            while ($row = $exitItemListRs->fetch_assoc())
            {
                $exitItemListItemKey[$row['item_key']] = 1;
            }
            $exitItemListRs->free();
        }

        $exitItemListRs = $mysql->query("SELECT `item_key` from `booth_order` group by `item_key`");
        if ($exitItemListRs)
        {
            while ($row = $exitItemListRs->fetch_assoc())
            {
                $exitItemListItemKey[$row['item_key']] = 1;
            }
            $exitItemListRs->free();
        }

        $result          = $mysql->query('SELECT * FROM `trade_item_list` WHERE `is_hot` = 1');
        $itemStocks      = [];
        $itemBoothStocks = [];
        $pubNum          = [];
        $pubLastEndTime  = [];
        if ($result)
        {
            while ($row = $result->fetch_assoc())
            {
                if ($row['item_key'] === '')
                {
                    continue;
                }

                if (!isset($exitItemListItemKey[$row['item_key']]) && $row['id'] > self::SENTRY_ITEM_LIST_ID)
                {
                    // 将挂单表没的key转为冷数据
                    if ($row['is_hot'])
                    {
                        $mysql->query("UPDATE `trade_item_list` SET `is_hot` = 0, `is_publicity` = 0, `pub_buy_people` = 0, `pub_price` = 0, `start_time` = 0, `end_time` = 0, `stock` = 0, `delay_time` = 0 WHERE `id` = '{$row['id']}'");
                    }
                    continue;
                }

                $itemId           = (int)$row['item_id'];
                $row['id']        = (int)$row['id'];
                $row['isOverlap'] = $overlap[$itemId] ?? 0;

                if ($row['is_publicity'] == 0)
                {
                    $row['end_time']       = 0;
                    $row['pub_buy_people'] = 0;
                    $row['pub_price']      = 0;
                }

                # 更新库存
                if (!isset($itemStocks[$itemId]))
                {
                    $itemStocks[$itemId] = $row['stock'];
                }
                else
                {
                    $itemStocks[$itemId] += $row['stock'];
                }

                if ($row['is_publicity'] == 1)
                {
                    if (!isset($pubNum[$itemId]))
                    {
                        $pubNum[$itemId]         = 1;
                        $pubLastEndTime[$itemId] = $row['end_time'] + $row['delay_time'];
                    }
                    else
                    {
                        $pubNum[$itemId] += 1;
                        $newEndTime = $row['end_time'] + $row['delay_time'];
                        if ($newEndTime > $pubLastEndTime[$itemId])
                        {
                            $pubLastEndTime[$itemId] = $newEndTime;
                        }
                    }
                }

                if (!self::$itemList->set($row['item_key'], $row))
                {
                    self::$instance->warn("[初始化数据] 存入itemList内存表失败, 已存数量:" . self::$itemList->count());
                    exit;
                }

                self::$itemListId2KeyMap->set($row['id'], ['key' => $row['item_key']]);
                self::$itemListId2KeyMapCached[intval($row['id'])] = $row['item_key'];

                # 初始化摆摊库存
                $boothStockSql = "SELECT SUM(`stock`) as `total_stock` FROM `booth_order` WHERE `is_publicity` = 0 AND `status` = 1 AND `item_key` = '{$row['item_key']}'";
                $boothRs       = $mysql->query($boothStockSql);
                if ($boothRs === false)
                {
                    self::$instance->warn("SQL: {$mysql->last_query}, Error: {$mysql->error}");
                    exit;
                }
                else
                {
                    $boothRow = $boothRs->fetch_assoc();
                }
                $boothTotalStock = $boothRow['total_stock'] ?? 0;
                self::$itemList->set($row['item_key'], ['boothStock' => $boothTotalStock]);
                if (!isset($itemBoothStocks[$itemId]))
                {
                    $itemBoothStocks[$itemId] = $boothTotalStock;
                }
                else
                {
                    $itemBoothStocks[$itemId] += $boothTotalStock;
                }
            }

            if ($result->num_rows == 0)
            {
                $mysql->query('ALTER TABLE `trade_item_list` AUTO_INCREMENT = ' . self::SENTRY_ITEM_LIST_ID);
                $mysql->query("INSERT INTO `trade_item_list` (`id`, `item_key`, `item_id`, `is_publicity`, `pub_buy_people`, `pub_price`, `end_time`, `refine_lv`, `is_damage`, `is_good_enchant`, `stock`) VALUES ('" . self::SENTRY_ITEM_LIST_ID . "', '', '0', '0', '0', '0', '0', '0', '0', '0', '0')");
            }

            $result->free();
            unset($result);
        }
        else
        {
            self::$instance->warn("SQL: {$mysql->last_query}, Error: {$mysql->error}");
            exit;
        }
        self::$instance->info("初始化ItemList. 耗时: " . (microtime(1) - $begin) . ', 总物品数:' . self::$itemList->count());

        # 更新 Item 表的库存
        foreach ($itemStocks as $itemId => $value)
        {
            self::$item->set($itemId, ['stock' => $value]);
        }

        # 更新 item 摆摊库存
        foreach ($itemBoothStocks as $itemId => $value)
        {
            self::$item->set($itemId, ['boothStock' => $value]);
        }

        # 查找公示物品最后的结束时间
        $sql = "SELECT `end_time`, `item_id` FROM `booth_order` WHERE `status` = 1 AND `is_publicity` = 1 GROUP BY `item_id` ORDER BY `end_time` DESC";
        $boothRs = $mysql->query($sql);
        if ($boothRs && $boothRs->num_rows > 0)
        {
            while ($row = $boothRs->fetch_assoc())
            {
                $pubNum[$row['item_id']]         = max(0, $pubNum[$row['item_id']] ?? 0);
                $pubLastEndTime[$row['item_id']] = max($row['end_time'], $pubLastEndTime[$row['item_id']] ?? 0);
            }
        }

        foreach ($pubNum as $itemId => $value)
        {
            self::$item->set($itemId, [
                'publicityNum'   => $value,
                'lastPubEndTime' => $pubLastEndTime[$itemId]
            ]);
        }

        self::doDelayPublicity($mysql, false);

        $mysql->close();
        $redis->close();

        unset($mysql, $result, $exchange, $equip, $itemConfig, $redis);

        # 加载策划表配置
        self::loadTableConfig();

        self::$instance->info('交易服务器初始化总耗时: ' . (microtime(1) - $sTime) . 's');
    }

//    public static function clearItemList(MySQLi $mysql)
//    {
//        $t1 = microtime(true);
//        $exitItemListIdRs = $mysql->query("SELECT `item_list_id` from `trade_goods` WHERE `item_list_id` > " . self::SENTRY_ITEM_LIST_ID . " GROUP BY `item_list_id`");
//        $exitItemListIds = [];
//        if ($exitItemListIdRs)
//        {
//            while ($row = $exitItemListIdRs->fetch_assoc())
//            {
//                $exitItemListIds[$row['item_list_id']] = 1;
//            }
//        }
//
//        $result  = $mysql->query('SELECT * FROM `trade_item_list` WHERE `id` > ' . self::SENTRY_ITEM_LIST_ID);
//        $counter = 0;
//        if ($result)
//        {
//            while ($row = $result->fetch_assoc())
//            {
//                $id = $row['id'];
//                if (isset($exitItemListIds[$id]))
//                {
//                    continue;
//                }
//
//                $mysql->query("DELETE FROM `trade_item_list` WHERE `id` = {$id}");
//                $counter++;
//            }
//        }
//
//        $t2 = microtime(true) - $t1;
//        Server::$instance->info("[清理数据] 清理itemList表完毕. 耗时:{$t2}, 数量:{$counter}");
//    }

    /**
     * 返回一个 Redis 对象
     *
     * @return \RO\Redis|\Redis|\RedisCluster|bool
     */
    public static function createRedis()
    {
        try
        {
            if (count(self::$instance->config['redis']) == 1)
            {
                list($host, $port) = explode(':', self::$instance->config['redis'][0]);
                $redis = new \RO\Redis();
                $rs    = $redis->connect($host, $port, 1);
                if (false === $rs)
                {
                    throw new \Exception("连接Redis服务器失败，{$host}:{$port}");
                }
            }
            else
            {
                $redis = new \RO\Redis(null, self::$instance->config['redis']);
            }
            $redis->setOption(\Redis::OPT_PREFIX, 'ro-trade-' . self::$regionId . ':');
            $redis->setOption(\Redis::OPT_READ_TIMEOUT, -1);
            return $redis;
        }
        catch (\Exception $e)
        {
            self::$instance->warn("[Redis] 创建失败, host: " . json_encode(self::$instance->config['redis']) . ", err: " . $e->getMessage());
            return false;
        }
    }

    /**
     * 创建一个MySQL
     *
     * @param bool $master 是否主数据库
     * @return bool|\RO\MySQLi
     */
    public static function createMySQL($master = true)
    {
        $obj = new \RO\MySQLi(self::$instance->config['mysql'], $master);

        if ($obj->connect_errno)
        {
            self::$instance->warn($obj->connect_error);
            return false;
        }

        return $obj;
    }

    /**
     * 加载一个json配置
     *
     * @param string $file 不需要带 .json 后缀
     * @return array|false
     */
    public static function loadConfig($file)
    {
        return @json_decode(file_get_contents(self::$configPath . $file . '.json'), true);
    }

    /**
     * 获取Config文件Hash
     *
     * @param string $file 不需要带 .json 后缀
     * @return array|false
     */
    public static function getConfigHash($file)
    {
        return md5_file(self::$configPath . $file . '.json');
    }

    /**
     * 获取玩家信息
     *
     * @param $charId
     * @return bool|null|string
     */
//    public static function getPlayerInfo($charId)
//    {
//        $key = sprintf('%d:%s:%d:default:data', self::$regionId, self::EREDISKEYTYPE_GLOBAL_CHAR_DATA, $charId);
//        if ($info = Server::$redis->hGetAll($key))
//        {
//            return $info;
//        }
//        else
//        {
//            return null;
//        }
//    }

    /**
     * 是否开启
     *
     * @return bool
     */
    public static function isOpen()
    {
        return self::$isOpen;
    }

    /**
     * 关闭交易所
     */
    public static function closeTrade()
    {
        self::$isOpen = false;
        Server::$instance->info("[交易所关闭] WorkerId:" . self::$instance->server->worker_id . "进程关闭交易所,并通知其他Worker进程关闭");
        $myId = self::$instance->server->worker_id;
        for ($i = 0; $i < self::$instance->config['swoole']['worker_num']; $i++)
        {
            if ($myId != $i)
            {
                while (true)
                {
                    if (false !== self::$instance->server->sendMessage('close', $i))
                    {
                        break;
                    }
                    else
                    {
                        Server::$instance->warn('[交易所关闭] 发送close消息失败');
                        usleep(300);
                    }
                }
            }
        }
        return true;
    }

    /**
     * 开启交易所
     */
    public static function openTrade()
    {
        self::$isOpen = true;
        Server::$instance->info("[交易所开启] WorkerId:" . self::$instance->server->worker_id . "进程开启交易所,并通知其他Worker进程开启");
        $myId = self::$instance->server->worker_id;
        for ($i = 0; $i < self::$instance->config['swoole']['worker_num']; $i++)
        {
            if ($myId != $i)
            {
                while (true)
                {
                    if (false !== self::$instance->server->sendMessage('open', $i))
                    {
                        break;
                    }
                    else
                    {
                        Server::$instance->warn('[交易所开启] 发送开启消息失败');
                        usleep(300);
                    }
                }
            }
        }
    }

    /**
     * 根据ItemListId获取通道对象
     * @param $itemListId
     * @return \Swoole\Channel
     */
    public static function getChannelByListId($itemListId)
    {
        return self::$taskChannel->get(self::getTaskIdById($itemListId));
    }

    /**
     * 根据列表ID获取分配的进程ID
     *
     * @param $id
     * @return int
     */
    public static function getTaskIdById($id)
    {
        return $id % self::$itemListTaskCount + self::$itemListTaskBegin;
    }

    protected static $_connectTick;

    public static function initConnection()
    {
        Server::$mysqlMaster = Server::createMySQL(true);
        Server::$redis       = Server::createRedis();

        if (null === self::$_connectTick)
        {
            # 连接检测
            self::$_connectTick = swoole_timer_tick(60 * 1000 + mt_rand(-1000, 10000), function()
            {
                if (Server::$mysqlMaster && true !== Server::$mysqlMaster->ping())
                {
                    $rs = Server::$mysqlMaster->_connect();
                    Server::$instance->debug('数据库连接断开，重新连接' . (true === $rs ? '成功' : '失败'));
                }

                try
                {
                    if (Server::$redis === false)
                    {
                        Server::$redis = Server::createRedis();
                    }
                    else
                    {
                        Server::$redis->ping();
                    }
                }
                catch (\RedisException $e)
                {
                    $redis = self::createRedis();

                    if (false !== $redis)
                    {
                        Server::$redis = $redis;
                        Server::$instance->debug('Redis连接断开，重新连接成功');
                    }
                }
            });
        }
    }

    public static function delayExecute($data, $callback, $fileName = null)
    {
        if (!is_string($callback) || !is_callable($callback))
        {
            throw new \Exception('设定的回调必须是字符串且可被回调');
        }

        if (is_array($data))
        {
            $data = json_encode($data);
        }
        $data = "\n" . date('Y-m-d H:i:s') . "\t" . $callback . "\t" . strlen($data) . "\t" . $data . "\0\0\0";

        $rs = file_put_contents(self::$dataDir . $fileName, $data, FILE_APPEND | LOCK_EX);
        if (strlen($data) === $rs)
        {
            return true;
        }
        return false;
    }

    /**
     * 写入延迟执行的SQL语句
     *
     * @param $sql
     * @return int|false
     */
    public static function saveDelaySQL($sql, $file = 'delay_sql.sql')
    {
        $rs = file_put_contents(self::$dataDir . $file, "/* " . date('Y-m-d H:i:s') . ' */ ' . $sql . ";\n", FILE_APPEND | LOCK_EX);
        if (false !== $rs)
        {
            self::$instance->server->task("sql|{$file}", 1);
        }

        return $rs;
    }

    /**
     * 执行sql文件
     *
     * @param $filename
     * @param \RO\MySQLi $mysql
     * @return bool
     */
    public static function sqlFileExecute($sqlFile, $mysql)
    {
        if (is_file($sqlFile))
        {
            $handle = fopen($sqlFile, "r");
            if (false === $handle)
            {
                Server::$instance->warn("打开 {$sqlFile} 文件失败");
                return false;
            }

            $posFile = dirname($sqlFile) . '/' . basename($sqlFile) . '.pos';
            if (is_file($posFile))
            {
                $pos = (int)file_get_contents($posFile);
            }
            else
            {
                $pos = 0;
            }

            if ($pos > 0)
            {
                fseek($handle, $pos);
            }

            $sql = '';
            if (!flock($handle, LOCK_EX))
            {
                Server::$instance->warn("文件加锁 {$sqlFile} 失败");
                return false;
            }
            while (!feof($handle))
            {
                $sql .= fgets($handle);

                if (substr(rtrim($sql), -1) == ";")
                {
                    $rs = $mysql->query($sql);

                    if (!$rs)
                    {
                        if ($mysql->errno === 1062)
                        {
                            # Duplicate entry
                            Server::$instance->warn("[insertIgnore] 忽略插入冲突数据 {$sql}");
                            $rs = true;
                        }
                    }

                    if ($rs)
                    {
                        $pos += strlen($sql);
                        $sql = '';
                        file_put_contents($posFile, $pos);
                    }
                    else
                    {
                        Server::$instance->warn("执行 {$sqlFile} 中SQL错误：" . $mysql->error);
                        Server::$instance->warn($mysql->last_query);
                        Server::$instance->warn("需要忽略本此sql请执行： echo " . ($pos + strlen($sql)) . " > $posFile 后重新执行");
                        flock($handle, LOCK_UN);
                        return false;
                    }
                }
            }

            flock($handle, LOCK_UN);
            fclose($handle);
            unlink($sqlFile);
            unlink($posFile);
            return true;
        }

        return true;
    }

    /**
     * 编译lua文件到json配置
     *
     * @param bool $checkLuaPath 是否检测Lua目录
     * @return bool
     * @throws \Exception
     */
    public static function formatLua2json($checkLuaPath = false)
    {
        $lua2bin  = escapeshellarg(realpath(__DIR__ . '/../../bin/lua2json'));
        $codePath = escapeshellarg(realpath(__DIR__ . '/../../../') . '/');
        $luaPath  = escapeshellarg(realpath(__DIR__ . '/../../../') . '/Lua/');
        if ($checkLuaPath && is_dir(trim($luaPath, "'")) === false)
        {
            return false;
        }

        $confPath = escapeshellarg(self::$configPath);
        exec("php -f $lua2bin $codePath $luaPath $confPath", $rs);
        end($rs);

        if (current($rs) == 'ok')
        {
            clearstatcache();
            return true;
        }

        throw new \Exception(implode("\n", $rs));
    }

    public static function loadBranchConfig()
    {
        global $options;
        $config    = json_decode(file_get_contents(self::$configPath . 'BranchConfig.json'), true);
        $branchKey = self::$branchKeys[self::$branchID];
        if (isset($options['t']))
        {
            $branchKey .= '_Test';
        }
        if (!isset($config[$branchKey]))
        {
            throw new \Exception("指定的分支 {$branchKey} 配置不存在");
        }

        $config = $config[$branchKey];
        $rs     = [
            'mysql'    => [],
            'redis'    => [],
            'globalDB' => [],
            'gameDb'   => [],
        ];

        $myDb = 'ro_' . self::$platName . '_' . self::$regionName;
        $list = [];
        foreach ($config['TradeDataBase'] as $st)
        {
            # 遍历一遍获取所有含 database 的配置，过滤掉不需要的
            if (isset($st['database']) && $st['database'] === $myDb)
            {
                $list[] = $st;
            }
        }
        # 如果一个都没有，说明是老的配置，使用全部的配置
        if (!$list)
        {
            $list = $config['TradeDataBase'];
        }
        foreach ($list as $st)
        {
            if (!isset($rs['mysql']['master']))
            {
                $rs['mysql'] = [
                    'user'    => $st['user'],
                    'pass'    => $st['password'],
                    'master'  => "{$st['ip']}:{$st['port']}",
                    'slave'   => [],
                    'charset' => 'utf8mb4',
                    'db'      => 'ro_' . self::$platName . '_' . self::$regionName,
                ];
            }
            else
            {
                $rs['mysql']['slave'][] = "{$st['ip']}:{$st['port']}";
            }
        }
        unset($myDb, $list);

        # Global 数据库
        $st = null;
        foreach ($config['DataBase'] as $tmp)
        {
            if (isset($tmp['database']) && $tmp['database'] === 'ro_global')
            {
                $st = $tmp;
            }
        }

        if (!$st)
        {
            reset($config['DataBase']);
            $st = current($config['DataBase']);
        }

        $rs['globalDB'] = [
            'user'    => $st['user'],
            'pass'    => $st['password'],
            'master'  => "{$st['ip']}:{$st['port']}",
            'slave'   => [],
            'charset' => 'utf8mb4',
            'db'      => 'ro_global',
        ];

        // 游戏数据库
        reset($config['DataBase']);
        $st           = current($config['DataBase']);
        $rs['gameDb'] = [
            'user'    => $st['user'],
            'pass'    => $st['password'],
            'master'  => "{$st['ip']}:{$st['port']}",
            'slave'   => [],
            'charset' => 'utf8mb4',
            'db'      => 'ro_' . self::$platName . '_' . self::$regionName,
        ];

        $rs['redis'][] = "{$config['Redis']['ip']}:{$config['Redis']['port']}";

        return $rs;
    }

    public function onShutdown($server)
    {
        $this->info("交易所已关闭");
    }

    /**
     * 启动的时候执行异步任务，启动时自动运行，请勿手动执行
     */
    protected static function doDelayJobAtStart()
    {
        # 开启一个子进程运行
        $process = new \Swoole\Process(function($process)
        {
            self::$mysqlMaster = self::createMySQL();
            while (true)
            {
                $list = glob(self::$dataDir . 'delay-*.job');
                if (!$list)
                {
                    break;
                }
                foreach ($list as $item)
                {
                    $delayJob = JobFifo::instance(basename($item));
                    $delayJob->exec(true);
                }
            }
            self::$mysqlMaster->close();
            exit;
        });
        $process->start();
        \Swoole\Process::wait(true);
        $file = self::$dataDir . 'delay-*.job';
        clearstatcache(false);
        if ($rs = glob($file))
        {
            $name = [];
            foreach ($rs as $item)
            {
                $name[] = basename($item);
            }
            Server::$instance->warn("异步任务没有执行完毕，不能启动，除非你移除此文件(以及 同名.idx 文件), Path: " . self::$dataDir . ", Files: " . implode(', ', $name));
            exit;
        }
    }

    /**
     * 公示延期处理
     *
     * @param \mysqli $mysql
     * @param bool $isLock
     */
    public static function doDelayPublicity($mysql, $isLock = true)
    {
        $globalMysql = new \RO\MySQLi(Server::$instance->config['globalDB']);
        if ($globalMysql->connect_errno)
        {
            self::$instance->warn('[公示延长] 连接global—db失败 错误信息:' . $globalMysql->connect_error);
            return;
        }

        $sql = "SELECT `regionid`, `platid`, `maintainstart`, `maintainend` FROM `region` WHERE `regionid` = " . $globalMysql->quote(self::$regionId);
        if ($rs = $globalMysql->query($sql))
        {
            if ($rs->num_rows > 0)
            {
                $row   = $rs->fetch_object();
                $begin = (int)strtotime($row->maintainstart);
                $end   = (int)strtotime($row->maintainend);
            }
            else
            {
                Server::$instance->warn("[公示延长] 不存在指定的RegionId: " . self::$platName);
                return;
            }
            $rs->free();
        }
        else
        {
            Server::$instance->warn("[公示延长] SQL: $sql, Error: " . $mysql->error);
            return;
        }

        $beginStr = date('Y-m-d H:i:s', $begin);
        $endStr   = date('Y-m-d H:i:s', $end);

        $fileBegin = 0;
        $fileEnd   = 0;
        $isFinish  = 0;
        $file      = self::$dataDir . self::$platName . '_' . self::$regionName . '_maintain.json';
        if (file_exists($file))
        {
            if (!($data = file_get_contents($file)))
            {
                return;
            }

            # 格式: {"begin": "1494233064","end": "1494233184", "is_finish": 0}
            $maintenance = @json_decode($data, true);
            $fileBegin   = isset($maintenance['begin']) ? (int)$maintenance['begin'] : 0;
            $fileEnd     = isset($maintenance['end']) ? (int)$maintenance['end'] : 0;
            $isFinish    = isset($maintenance['is_finish']) ? (int)$maintenance['is_finish'] : 0;
        }

        if ($fileBegin == $begin && $fileEnd == $end)
        {
            if ($isFinish)
            {
                return;
            }
        }
        else
        {
            $data = json_encode([
                                    'begin'     => $begin,
                                    'end'       => $end,
                                    'is_finish' => 0
                                ]);
            Server::$instance->info("[公示延长] 发现维护时间变化, 执行修改维护时间。开始时间:{$beginStr}, 结束时间:{$endStr}");
            if (!file_put_contents($file, $data))
            {
                return;
            }
        }

        if ($begin === 0 && $end === 0)
        {
            return;
        }

        $current = time();

        # 没进入维护期间不处理公示延期
        if ($current < $begin)
        {
            return;
        }

        if ($isLock && !self::$publicityLock->trylock())
        {
            return;
        }

        do
        {
            if ($begin > $end)
            {
                Server::$instance->warn("[公示延长] 维护开始时间不能大于维护结束时间");
                break;
            }

            $pubLastEndTime = [];
            $delayTime      = $end - $begin;
            foreach (Server::$itemList as $key => $itemList)
            {
                if (!$itemList['is_publicity'])
                {
                    continue;
                }

                # 公示结束时间早于维护开始时间不处理延迟
                # 在维护期间进入公示期的不处理延迟
                if ($itemList['end_time'] > $begin && $begin > $itemList['start_time'])
                {
                    $newEndTime = $itemList['end_time'] + $delayTime;

                    $sql = "UPDATE `trade_item_list` SET `delay_time` = '{$delayTime}' WHERE `id` = '{$itemList['id']}'";
                    $rs  = $mysql->query($sql);
                    if ($rs)
                    {
                        $rs = Server::$itemList->set($key, ['delay_time' => $delayTime]);
                        if ($rs)
                        {
                            // 更新正在出售状态下挂单表的公示期结束时间
                            $updateSql = "UPDATE `trade_goods` SET `end_time` = '{$newEndTime}' WHERE `item_list_id` = '{$itemList['id']}' AND `status` = 1 AND `is_publicity` = 1";
                            if (!$mysql->query($updateSql))
                            {
                                Server::$instance->warn("[公示延长] 更新失败, key: {$key} Sql Error:{$mysql->error}");
                            }

                            // 更新正在抢购的买家记录的公示期结束时间
                            $updateSql = "UPDATE `trade_record_bought` SET `end_time` = '{$newEndTime}' WHERE `publicity_id` = '{$itemList['id']}' AND `status` = 2";
                            if (!$mysql->query($updateSql))
                            {
                                Server::$instance->warn("[公示延长] 更新失败, key: {$key} Sql Error:{$mysql->error}");
                            }
                        }
                        else
                        {
                            Server::$instance->warn('[公示延长] 设置swoole table失败, key:' . $key);
                        }

                        $pubLastEndTime[$itemList['item_id']] = $newEndTime;
                    }
                    else
                    {
                        Server::$instance->warn("[公示延长] 更新失败, key: {$key} Sql Error:{$mysql->error}");
                    }
                }
                else
                {
                    Server::$instance->info("[公示延长] 发现公示物品不符合延长条件. item_list_id:{$itemList['id']}, item_list_key:{$key}, item_id:{$itemList['item_id']}");
                }
            }

            $sql = "SELECT `id`, `end_time`, `item_id`, `time` FROM `booth_order` WHERE `status` = 1 AND `is_publicity` = 1";
            $boothRs = $mysql->query($sql);
            if ($boothRs && $boothRs->num_rows > 0)
            {
                while($row = $boothRs->fetch_assoc())
                {
                    if ($row['end_time'] > $begin && $begin > $row['time'])
                    {
                        $boothNewEndTime = intval($row['end_time']) + $delayTime;
                        // 更新正在出售状态下的摆摊挂单表的公示期结束时间
                        $updateSql = "UPDATE `booth_order` SET `end_time` = {$boothNewEndTime} WHERE `id` = {$row['id']}";
                        if (!$mysql->query($updateSql))
                        {
                            Server::$instance->warn("[摆摊公示延长] 更新失败, order_id: {$row['id']} Sql Error:{$mysql->error}");
                        }

                        // 更新摆摊正在抢购的买家记录的公示期结束时间
                        $updateSql = "UPDATE `booth_record_bought` SET `end_time` = {$boothNewEndTime} WHERE `order_id` = '{$row['id']}'";
                        if (!$mysql->query($updateSql))
                        {
                            Server::$instance->warn("[摆摊公示延长] 更新失败, order_id: {$row['id']} Sql Error:{$mysql->error}");
                        }

                        $lastEndTime = $pubLastEndTime[$row['item_id']] ?? 0;
                        $pubLastEndTime[$row['item_id']] = max($boothNewEndTime, $lastEndTime);
                    }
                }
            }

            if (!empty($pubLastEndTime))
            {
                foreach ($pubLastEndTime as $itemId => $v)
                {
                    Server::$item->set($itemId, ['lastPubEndTime' => $v]);
                }
            }

            Server::$instance->info("[公示延长] 公示期延长执行成功! 开始时间:{$beginStr}, 结束时间:{$endStr}, 延长时间为: {$delayTime}秒");
            @file_put_contents($file, @json_encode([
                                                       'begin'     => $begin,
                                                       'end'       => $end,
                                                       'is_finish' => 1
                                                   ]));
        }
        while (0);

        if ($isLock)
        {
            self::$publicityLock->unlock();
        }
        $globalMysql->close();
    }
}
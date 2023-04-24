<?php

namespace RO\Trade;

use RO\MySQLi;
use \Exception;

include __DIR__.'/../vendor/autoload.php';

$shortOpts = '';
$shortOpts .= 'p:';     // -p 平台名
$shortOpts .= 'r:';     // -r regionname
$shortOpts .= 'v:';     // -v 版本

$longOpts = [
    'local',        // --local 本地开发模式，不重新编译文件
    'timezone',     // --timezone 时区
    'debug',        // --debug 使用默认配置参数
];

$options = getopt($shortOpts, $longOpts);


function getInsertSql($table, $data)
{
    $fields = [];
    $values = [];
    foreach ($data as $key => $field)
    {
        $values[] = quoteValue($field);
        $fields[] = $key;
    }

    return "INSERT INTO `" . $table . "` (`" . implode('`, `', $fields) . "`) VALUES (" . implode(", ", $values) . ")";
}

function quoteValue($value)
{
    global $myDb;

    if($value instanceof Hex)
    {
        return $value->data;
    }
    elseif (is_string($value))
    {
        return "'" . $myDb->real_escape_string($value) . "'";
    }
    elseif (is_null($value))
    {
        return 'NULL';
    }
    else
    {
        # int, float
        return "'$value'";
    }
}

class Hex
{
    public $data;

    public function __construct($data)
    {
        $this->data = '0x' . bin2hex($data);
    }
}

class BatchInsert
{
    public $tableName;

    /** @var MySQLi */
    public $db;

    public $values = [];

    public $fields = [];

    public $batchNum = 1000;

    private $lastSql = '';

    private $_count = 0;

    public function __construct($db, $tableName, $fields)
    {
        $this->db = $db;
        $this->tableName = $tableName;
        $this->fields = $fields;
    }

    /**
     * 添加并批量插入sql, 当添加的数据量达到设置的批量数值则执行插入
     *
     * @param $values
     * @param null $batchNum
     * @return bool|\mysqli_result
     */
    public function addAndBatchExecute($values, $batchNum = null)
    {
        if ($batchNum === null) $batchNum = $this->batchNum;

        $this->add($values);

        if (++$this->_count % $batchNum === 0)
        {
            $rs = $this->execute();
            $this->values = [];
            $this->_count = 0;
            return $rs;
        }

        return true;
    }

    /**
     * 执行批量插入
     *
     * @return bool|\mysqli_result
     * @throws \Exception
     */
    public function execute()
    {
        if(empty($this->values)) return true;

        $rs = $this->db->query($this->getBatchInsertSql());
        if (!$rs)
        {
            throw new \Exception($this->tableName . '批量插入失败 Error: ' . $this->getError());
        }

        return $rs;
    }

    public function getError()
    {
        return $this->db->error;
    }

    public function add($values)
    {
        $newValues = [];
        foreach ($values as $value)
        {
            $newValues[] = $this->quoteValue($value);
        }

        $this->values[] = $newValues;
    }

    public function getBatchInsertSql()
    {
        $fields = "(`". implode('`, `', $this->fields) ."`)";
        $refundSql = 'INSERT INTO `' . $this->tableName . '` ' . $fields . ' VALUES ';
        foreach ($this->values as $values)
        {
            $refundSql .= "(" . implode(',', $values) . "), ";
        }

        $this->lastSql = rtrim($refundSql, ', ') . ';';
        return $this->lastSql;
    }

    public function getLastSql()
    {
        return $this->lastSql;
    }

    function quoteValue($value)
    {
        if($value instanceof Hex)
        {
            return $value->data;
        }
        elseif (is_string($value))
        {
            return "'" . $this->db->real_escape_string($value) . "'";
        }
        elseif (is_null($value))
        {
            return 'NULL';
        }
        else
        {
            # int, float
            return "'$value'";
        }
    }
}

function logger($label, $warp = false, $color = '[36m')
{
    $beg = "\x1b{$color}";
    $end = "\x1b[39m";
    list($f, $t) = explode(' ', microtime());
    $f   = substr($f, 1, 4);
    $type = 'info';
    $str = $beg .'['. date("Y-m-d H:i:s", $t) . "{$f}][{$type}]{$end} " . $label;
    echo (!$warp ? "\r" : "\n") . $str . ($warp ? "\n" : '');
}

function warn($label)
{
    logger($label, true, '[31m');
}

class Server
{
    /**
     * 物品信息多进程共享表，key 为 itemid
     *
     * @var \Swoole\Table
     */
    public static $item;

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
     * 服务器配置
     *
     * （从游戏服务器的 GameConfig 文件中读取）
     *
     * @var array
     */
    public static $configExchange;

    /**
     * 当前数据目录
     *
     * @var string
     */
    public static $dataDir;

    /**
     * JSON配置目录
     *
     * @var string
     */
    public static $configPath;

    /**
     * 有--debug参数时,读下面的配置。
     *
     * @var array
     */
    public static $config = [
        'destination' => [
            "master" => "172.26.24.113:3306",
            "user" => "root",
            "pass" => "admin",
            "db"   => "ro_xd_r14",
            "charset" => "utf8mb4"
        ],
        'source'      => [
            "master" => "172.26.24.113:3306",
            "user" => "root",
            "pass" => "admin",
            "db"   => "ro_xd_r9",
            "charset" => "utf8mb4"
        ],
//        'source'      => [
//            "master" => "10.1.54.109:5222",
//            "user" => "backup",
//            "pass" => "CZ3wMrfGbs3XqU",
//            "db"   => "ro_xd_r1",
//            "charset" => "utf8mb4"
//        ],
        'redis' => [
            "127.0.0.1:6379"
        ],
        'regionId' => 0,
        'trade' => [
            'goods' => [
                'open' => true,
                'id'   => 0,
            ],
            'buy' => [
                'open' => true,
                'id'   => 0,
            ],
            'sell' => [
                'open' => true,
                'id'   => 0,
            ],
            'pub' => [
                'open' => true,
                'id'   => 0,
            ],
            'item' => [
                'open' => true,
            ]
        ]
    ];

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
        0 => 'Debug',   // 内网
        1 => 'TF',      // 预言之地
        2 => 'Publish', // 正式服
    ];

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

    public static $redis;

    public function __construct()
    {
        $this->checkConfig();
        $this->onBeforeStart();
    }

    protected function checkConfig()
    {
        if (version_compare(SWOOLE_VERSION, '1.9.6', '<'))
        {
            echo '必须 1.9.6 以上版本 Swoole';
            exit;
        }

        if (!extension_loaded('ds'))
        {
            echo '必须安装ds扩展';
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


        if (isset($options['local']))
        {
            # 读取配置文件
            self::$configPath = realpath(__DIR__.'/../config-dev/') .'/';
            self::$branchID   = 0;
        }
        else
        {
            if (!is_file(__DIR__ .'/../../branch.lua'))
            {
                echo '配置 '. realpath(__DIR__ .'/../../../') .'/branch.lua 文件不存在，无法启动';
                exit;
            }

            try
            {
                $lua = new \Lua();
                self::$branchID = intval($lua->eval(file_get_contents(__DIR__ . '/../../branch.lua') . "\nreturn BranchID.id"));
                unset($lua);
            }
            catch (\Exception $e)
            {
                echo '解析 branch.lua 配置失败, '. $e->getMessage();
                exit;
            }

            $dataDir          = "/data/rogame/data/" . self::$platName . '/' . self::$regionName . '/';
            self::$configPath = $dataDir . (isset($options['v']) ? $options['v'] : 'unknown') . '/';

            try
            {
                # 重新生成配置
                self::formatLua2json();
            }
            catch (\Exception $e)
            {
                echo '解析失败: '. $e->getMessage();
                exit;
            }
        }

        if (!isset(self::$branchKeys[self::$branchID]))
        {
            echo "未知 BranchID.id (". self::$branchID .")\n";exit;
        }

        if (0 === self::$branchID)
        {
            logger("当前启动的模式是开发模式", true);
        }


        self::$regionId = self::$config['regionId'];

        if (!isset($options['debug']))
        {
            $config = self::loadBranchConfig();
            self::$config['destination'] = $config['mysql'];
            self::$config['source'] = $config['mysql'];
            self::$config['redis'] = $config['redis'];

            $mysql = new \RO\MySQLi($config['globalDB']);
            $sql   = "SELECT `regionid`, `platid` FROM `region` WHERE `regionname` = ". $mysql->quote(self::$regionName);
            if ($rs = $mysql->query($sql))
            {
                if ($rs->num_rows > 0)
                {
                    $row = $rs->fetch_object();
                    self::$regionId = (int)$row->regionid;
                }
                else
                {
                    exit;
                }
                $rs->free();
            }
        }
    }

    public function onBeforeStart()
    {
        self::$configExchange = self::loadConfig('ExchangeConfig');

        if (!self::$configExchange)
        {
            warn('加载交易所配置 ExchangeConfig.json 失败');
            exit;
        }

        # 物品表信息
        self::$item = new \Swoole\Table(1024 * 16);
        self::$item->column('isTrade',           \Swoole\Table::TYPE_INT, 1);        # 是否售卖
        self::$item->column('isOverlap',         \Swoole\Table::TYPE_INT, 1);        # 是否堆叠物品
        self::$item->column('buying',            \Swoole\Table::TYPE_INT, 4);        # 有人正在购买
        self::$item->column('sellingNum',        \Swoole\Table::TYPE_INT, 2);        # 正在卖的数量
        self::$item->column('soldNum',           \Swoole\Table::TYPE_INT, 8);        # 近期成交数,该数值会由一个进程管理
        self::$item->column('price',             \Swoole\Table::TYPE_INT, 4);        # 单价
        self::$item->column('stock',             \Swoole\Table::TYPE_INT, 4);        # 总库存
        self::$item->column('priceUpdateTime',   \Swoole\Table::TYPE_INT, 4);        # 价格最后更新时间
        self::$item->column('cycle',             \Swoole\Table::TYPE_INT, 4);        # 价格调价周期
        self::$item->column('exchangeNum',       \Swoole\Table::TYPE_INT, 4);        # 公示期交易量，在售物品少于此量的将为公示物品
        self::$item->column('equipType',         \Swoole\Table::TYPE_INT, 4);        # 装备类型, 不是装备为0;
        self::$item->column('itemType',          \Swoole\Table::TYPE_INT, 4);        # 物品类型
        self::$item->column('publicityShowTime', \Swoole\Table::TYPE_INT, 4);        # 公示物品公示时间
        self::$item->column('category',          \Swoole\Table::TYPE_INT, 4);        # 物品分类ID
        self::$item->column('publicityNum',      \Swoole\Table::TYPE_INT, 1);        # 公示数
        self::$item->column('name',              \Swoole\Table::TYPE_STRING, 128);   # 名称
        self::$item->create();

        # 物品列表信息
        self::$itemList = new \Swoole\Table(1024 * 256);
        self::$itemList->column('id',             \Swoole\Table::TYPE_INT, 4);       # ID
        self::$itemList->column('item_id',        \Swoole\Table::TYPE_INT, 4);       # 物品id
        self::$itemList->column('stock',          \Swoole\Table::TYPE_INT, 4);       # 库存
        self::$itemList->column('is_publicity',   \Swoole\Table::TYPE_INT, 1);       # 是否公示
        self::$itemList->column('pub_buy_people', \Swoole\Table::TYPE_INT, 4);       # 抢购人数
        self::$itemList->column('pub_price',      \Swoole\Table::TYPE_INT, 8);       # 公示价格
        self::$itemList->column('end_time',       \Swoole\Table::TYPE_INT, 4);       # 公示期结束时间
        self::$itemList->column('refine_lv',      \Swoole\Table::TYPE_INT, 1);       # 精炼等级
        self::$itemList->column('is_damage',      \Swoole\Table::TYPE_INT, 1);       # 是否损坏
        self::$itemList->column('isOverlap',      \Swoole\Table::TYPE_INT, 1);       # 精炼等级
        self::$itemList->column('item_data',      \Swoole\Table::TYPE_STRING, 512);  # itemData
        self::$itemList->create();

        # 加载RO配置
        try
        {
            $this->transferItemInfo();
            self::initData();
        }
        catch (\Exception $e)
        {
            warn('[服务启动] 加载配置发生错误, 错误信息:' . $e->getMessage() . ' 错误行数:'. $e->getLine() .' 错误文件:' . $e->getFile());
            if (IS_DEBUG)
            {
                throw $e;
            }

            exit;
        }
    }

    public static function loadBranchConfig()
    {
        $config = json_decode(file_get_contents(self::$configPath . 'BranchConfig.json'), true);
        $branchKey = self::$branchKeys[self::$branchID];
        if (!isset($config[$branchKey]))
        {
            throw new \Exception("指定的分支 {$branchKey} 配置不存在");
        }

        $config = $config[$branchKey];
        $rs = [
            'mysql'    => [],
            'redis'    => [],
            'globalDB' => [],
        ];

        foreach ($config['TradeDataBase'] as $st)
        {
            if (!isset($rs['mysql']['master']))
            {
                $rs['mysql'] = [
                    'user'    => $st['user'],
                    'pass'    => $st['password'],
                    'master'  => "{$st['ip']}:{$st['port']}",
                    'slave'   => [],
                    'charset' => 'utf8mb4',
                    'db'      => 'ro_'. self::$platName .'_'. self::$regionName,
                ];
            }
            else
            {
                $rs['mysql']['slave'][] = "{$st['ip']}:{$st['port']}";
            }
        }

        # ro 数据库
        $st = current($config['DataBase']);
        $rs['globalDB'] = [
            'user'    => $st['user'],
            'pass'    => $st['password'],
            'master'  => "{$st['ip']}:{$st['port']}",
            'slave'   => [],
            'charset' => 'utf8mb4',
            'db'      => 'ro_global',
        ];

        $rs['redis'][] = "{$config['Redis']['ip']}:{$config['Redis']['port']}";

        return $rs;
    }

    /**
     * 重新加载策划表
     *
     */
    public static function loadTableConfig()
    {
        $list = [
            'RoleData'          => 'RoleDataConfig',
            'EquipRefine'       => 'RefineTradeConfig',
            'Equip'             => 'EquipConfig',
            'EquipEnchant'      => 'EnchantConfig',
            'EquipEnchantPrice' => 'EnchantPriceConfig',
            'EquipUpgrade'      => 'EquipUpgradeConfig',
        ];

        foreach ($list as $name => $fun)
        {
            $funF = "\\RO\\Trade\\$fun";
            /**
             * @var mixed $funF
             */
            $funF::loadConfig(self::loadConfig($name));
        }
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
            warn('读取 Exchange.json 配置失败');
            exit;
        }

        $equip = self::loadConfig('Equip');
        if (!$equip)
        {
            warn('读取 Equip.json 配置失败');
            exit;
        }

        $item = self::loadConfig('Item');
        if (!$item)
        {
            warn('读取 Item.json 配置失败');
            exit;
        }

        # 加载策划表配置
        RoleDataConfig::loadConfig(self::loadConfig('RoleData'));
        RefineTradeConfig::loadConfig(self::loadConfig('EquipRefine'));
        EquipConfig::loadConfig($equip);
        EnchantConfig::loadConfig(self::loadConfig('EquipEnchant'));
        EnchantPriceConfig::loadConfig(self::loadConfig('EquipEnchantPrice'));

        $mysql = self::createMySQL(true);
        $redis = self::createRedis();
        if (!$mysql || !$redis)
        {
            exit;
        }

        # 物品是否堆叠信息
        $overlap  = [];
        $category = [];
        $begin    = microtime(1);
        # 读取物品信息
        $result   = $mysql->query('SELECT * FROM `trade_item_info`');
        if ($result)
        {
            while ($row = $result->fetch_assoc())
            {
                $itemId = $row['itemid'];

                if (!isset($exchange[$itemId]))
                {
                    continue;
                }

                if (!isset($item[$itemId]))
                {
                    unset($exchange[$itemId]);
                    warn("[加载配置] Exchange 策划表配置异常，{$itemId} 在 Item 中不存在");
                }

                if (!isset($exchange[$itemId]['Overlap']))
                {
                    logger('Exchange配置表 itemId:' . $itemId . '缺少Overlap参数', true);
                }

                $itemExc = $exchange[$itemId];

                $data = [
                    'isTrade'           => (int)$itemExc['Trade'] ?: 0,
                    'isOverlap'         => isset($itemExc['Overlap']) ? intval($itemExc['Overlap']) : 0,
                    'name'              => $itemExc['NameZh'],
                    'buying'            => 0,
                    'soldNum'           => isset($soldNum[$itemId]) ? $soldNum[$itemId] : 0,
                    'price'             => (int)$row['last_server_price'],
                    'priceUpdateTime'   => (int)$row['last_calc_price_time'],
                    'cycle'             => (int)$row['t'],
                    'exchangeNum'       => isset($itemExc['ExchangeNum']) ? intval($itemExc['ExchangeNum']) : 0,
                    'equipType'         => isset($equip[$itemId]['EquipType']) ? (int)$equip[$itemId]['EquipType'] : 0,
                    'itemType'          => isset($item[$itemId]['Type']) ? (int)$item[$itemId]['Type'] : 0,
                    'publicityShowTime' => isset($itemExc['ShowTime']) ? $itemExc['ShowTime'] : 0,
                    'category'          => isset($itemExc['Category']) ? $itemExc['Category'] : 0,
                ];
                $overlap[$itemId]  = $data['isOverlap'];
                $category[$itemId] = $data['category'];

                self::$item->set($itemId, $data);

                unset($exchange[$itemId]);
            }

            # 释放对象
            $result->free();
            unset($result);
        }
        else
        {
            warn("SQL: {$mysql->last_query}, Error: {$mysql->error}");
            exit;
        }

        foreach ($exchange as $itemId => $itemExc)
        {
            if (!isset($item[$itemId]))
            {
                warn("[加载配置] Exchange 策划表配置异常，{$itemId} 在 Item 中不存在");
            }
            $data = [
                'isTrade'           => $itemExc['Trade'] == 1 ?: 0,
                'name'              => $itemExc['NameZh'],
                'isOverlap'         => isset($itemExc['Overlap']) ? intval($itemExc['Overlap']) : 0,
                'exchangeNum'       => isset($itemExc['ExchangeNum']) ? intval($itemExc['ExchangeNum']) : 0,
                'equipType'         => isset($equip[$itemId]['EquipType']) ? $equip[$itemId]['EquipType'] : 0,
                'itemType'          => isset($item[$itemId]['Type']) ? (int)$item[$itemId]['Type'] : 0,
                'soldNum'           => 0,
                'publicityShowTime' => isset($itemExc['ShowTime']) ? $itemExc['ShowTime'] : 0,
                'category'          => isset($itemExc['Category']) ? $itemExc['Category'] : 0,
            ];
            $overlap[$itemId]  = $data['isOverlap'];
            $category[$itemId] = $data['category'];

            self::$item->set($itemId, $data);
        }

        $begin      = microtime(1);
        $result     = $mysql->query('SELECT * FROM `trade_item_list`');
        $itemStocks = [];
        $pubNum     = [];
        if ($result)
        {
            while ($row = $result->fetch_assoc())
            {
                $itemId           = (int)$row['item_id'];
                $row['id']        = (int)$row['id'];
                $row['isOverlap'] = $overlap[$itemId] ?: 0;
                $row['category']  = $category[$itemId] ?: 0;

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
                        $pubNum[$itemId] = 1;
                    }
                    else
                    {
                        $pubNum[$itemId] += 1;
                    }
                }

                self::$itemList->set($row['item_key'], $row);
            }

            if ($result->num_rows == 0)
            {
                $mysql->query('ALTER TABLE `trade_item_list` AUTO_INCREMENT = 1000000001');
            }

            $result->free();
            unset($result);
        }
        else
        {
            warn("SQL: {$mysql->last_query}, Error: {$mysql->error}");
            exit;
        }

        # 更新 Item 表的库存
        foreach ($itemStocks as $itemId => $value)
        {
            self::$item->set($itemId, ['stock' => $value]);
        }
        foreach ($pubNum as $itemId => $value)
        {
            self::$item->set($itemId, ['publicityNum' => $value]);
        }

        $mysql->close();
        $redis->close();

        self::loadTableConfig();

        unset($mysql, $result, $exchange, $equip, $item, $redis);
    }


    /**
     * 加载一个json配置
     *
     * @param string $file 不需要带 .json 后缀
     * @return array|false
     */
    public static function loadConfig($file)
    {
        return @json_decode(file_get_contents(self::$configPath . $file .'.json'), true);
    }

    /**
     * 编译lua文件到json配置
     *
     * @return bool
     * @throws \Exception
     */
    public static function formatLua2json()
    {
        $lua2bin  = escapeshellarg(realpath(__DIR__.'/lua2json'));
        $codePath = escapeshellarg(realpath(__DIR__.'/../../') .'/');
        $confPath = escapeshellarg(self::$configPath);
        exec("php -f $lua2bin $codePath $confPath", $rs);
        end($rs);

        if (current($rs) == 'ok')return true;

        throw new \Exception(implode("\n", $rs));
    }


    /**
     * 返回一个 Redis 对象
     *
     * @return \RO\Redis|\Redis|\RedisCluster|bool
     */
    public static function createRedis()
    {
        try
        {
            if (count(self::$config['redis']) == 1)
            {
                list($host, $port) = explode(':', self::$config['redis'][0]);
                $redis = new \RO\Redis();
                $redis->connect($host, $port);
//                $redis->setOption(\Redis::OPT_PREFIX, 'ro-trade:' . self::$regionId . ':');
            }
            else
            {
                $redis = new \RO\Redis(null, self::$config['redis']);
//                $redis->setOption(\Redis::OPT_PREFIX, 'ro_trade:' . self::$regionId . ':');
            }

            return $redis;
        }
        catch (\Exception $e)
        {
            warn($e->getMessage());
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
        $obj = new \RO\MySQLi(self::$config['destination'], $master);

        if ($obj->connect_errno)
        {
            warn($obj->connect_error);
            return false;
        }

        return $obj;
    }

    /**
     * 执行itemInfo迁移
     * @throws \Exception
     */
    private function transferItemInfo()
    {
        if (!self::$config['trade']['item']['open'])
        {
            return;
        }

        $oldDb = new \RO\MySQLi(self::$config['source'], true);
        if ($oldDb->connect_errno)
        {
            throw new \Exception("error: {$oldDb->connect_error}");
        }

        $mysql = self::createMySQL(true);

        $rs = $oldDb->query("SELECT * FROM `trade_item_info`");
        if ($rs)
        {
            while($data = $rs->fetch_object())
            {
                $sql = "REPLACE INTO `trade_item_info` (`itemid`, `last_server_price`, `last_calc_price_time`, `t`) VALUES ('{$data->itemid}', '{$data->last_server_price}', '{$data->last_calc_price_time}', '{$data->t}')";
                $rs2 = $mysql->query($sql);
                if (!$rs2)
                {
                    throw new \Exception("sql: {$sql} error: {$mysql->error}");
                }
            }

            logger('迁移ItemInfo完毕', true);
            $rs->free();
        }
        else
        {
            throw new \Exception("sql: SELECT * FROM `trade_item_info` error: {$oldDb->error}");
        }

        $mysql->close();
        $oldDb->close();
    }
}

function generateUniqueId(\RO\Cmd\ItemData $itemData)
{
    $key = "itemid:{$itemData->base->id}refinelv:{$itemData->equip->refinelv}damage:{$itemData->equip->damage}enchant:";

    if (\RO\Utils\ROConfig::isGoodEnchant($itemData))
    {
        if (!empty($itemData->enchant->attrs))
        {
            $map = [];
            foreach ($itemData->enchant->attrs as $attr)
            {
                $map[$attr->getType()] = $attr->getValue();
            }

            foreach ($map as $type => $value)
            {
                $key .= "{$type},{$value}";
            }
        }

        if (!empty($itemData->enchant->extras))
        {
            $key .= "extras:";
            $set = new \Ds\Set();
            foreach ($itemData->enchant->extras as $extra)
            {
                $set->add($extra->buffid);
            }
            foreach ($set as $v)
            {
                $key .= "b{$v},";
            }
        }
    }

    if ($itemData->equip->lv)
    {
        $key .= "lv:{$itemData->equip->lv}";
    }

    return $key;
}

$server = new Server();
global $redis;
$redis = Server::createRedis();
Server::$redis = $redis;

ini_set('memory_limit', '16G');

$db = new \RO\MySQLi(Server::$config['source'], true);
if ($db->connect_errno)
{
    throw new \Exception("error: {$db->connect_error}");
}

global $myDb;
$myDb = new \RO\MySQLi(Server::$config['destination'], true);
if ($myDb->connect_errno)
{
    throw new \Exception("error: {$db->connect_error}");
}

//------------------------------- 公示迁移 ------------------------------------

$pubSwitch = Server::$config['trade']['pub']['open'];
if ($pubSwitch === false)
    goto pubEnd;

//$itemListIdToPublicityIdMap = [];
$lastItemListId = 0;
try
{
    $spendTime = microtime(1);

    $rs = $db->query("SELECT * FROM `trade_item_list` WHERE `id` > {$lastItemListId} AND `is_publicity` = 1");
    while ($data = $rs->fetch_object())
    {
        $lastItemListId = $data->id;

        if ($data->item_data)
        {
            $itemDataObj = new \RO\Cmd\ItemData($data->item_data);
        }
        else
        {
            $itemDataObj                  = new \RO\Cmd\ItemData();
            $itemInfo = new \RO\Cmd\ItemInfo();
            $itemInfo->id = $data->item_id;
            $equipData = new \RO\Cmd\EquipData();
            $equipData->refinelv = 0;
            $equipData->damage = 0;
            $itemDataObj->base = $itemInfo;
            $itemDataObj->equip = $equipData;
        }

        $uniqueId = generateUniqueId($itemDataObj);

        $fields = [
            'id' => $data->id,
            'uniqueid'   => $uniqueId,
            'itemid'   =>$data->item_id,
            'endtime' => $data->end_time,
            'price' => $data->pub_price,
            'buy_people' => $data->pub_buy_people
        ];

        $sql = getInsertSql('trade_publicity', $fields);
        $insertRs = $myDb->query($sql);
        if (!$insertRs)
        {
            throw new Exception("sql: {$sql} error: {$myDb->error}");
        }

        $buyRs = $db->query("SELECT * FROM `trade_record_bought` WHERE `status` = 2 AND `publicity_id` = {$data->id}");
        if ($buyRs && $buyRs->num_rows)
        {
            while($data = $buyRs->fetch_object())
            {
                $fields = [
                    'publicity_id' => $data->publicity_id,
                    'buyerid' => $data->char_id,
                    'count' => $data->total_count,
                ];

                $sql = getInsertSql('trade_publicity_buy', $fields);
                $insertRs = $myDb->query($sql);
                if (!$insertRs)
                {
                    throw new Exception("sql: {$sql} error: {$myDb->error}");
                }
            }
            $buyRs->free();
        }

        logger("trade_item_list数据迁移成功, 迁移最后的id为:{$lastItemListId}");


//        $pubRs = $myDb->query("SELECT * FROM `trade_publicity` WHERE `uniqueid` = {$uniqueId}");
//        if ($pubRs->num_rows)
//        {
//            $pubData = $pubRs->fetch_object();
//            $sql = "UPDATE `trade_publicity` SET `endtime` = {$data->end_time}, `price` = {$data->pub_price}, `buy_people` = {$data->pub_buy_people} WHERE `id` = {$pubData->id}";
//            $updateRs = $myDb->query($sql);
//            if (!$updateRs)
//            {
//                throw new Exception("sql: {$sql} error: {$myDb->error}");
//            }
//
//            $itemListIdToPublicityIdMap[$data->id] = $pubData->id;
//        }
//        else
//        {
//            $sql = "INSERT INTO `trade_publicity` (`uniqueid`, `itemid`, `endtime`, `price`, `buy_people`) VALUES ('{$uniqueId}', '{$data->item_id}', '{$data->end_time}', '{$data->pub_price}', '{$data->pub_buy_people}')";
//            $insertRs = $myDb->query($sql);
//            if (!$insertRs)
//            {
//                throw new Exception("sql: {$sql} error: {$myDb->error}");
//            }
//
//            $itemListIdToPublicityIdMap[$data->id] = $myDb->insert_id;
//        }

    }

    $rs->free();
    logger('公示记录迁移耗时:' . (microtime(1) - $spendTime), true);
}
catch (Exception $e)
{
    warn($e->getMessage());
    warn('trade_item_list失败的最后id为:' . $lastItemListId);
}

pubEnd:

// ---------------------------- 挂单表迁移 ------------------------------
$goodsSwitch = Server::$config['trade']['goods']['open'];
if ($goodsSwitch === false)
{
    goto goodsEnd;
}

$goodsId  = 0;
$limit    = 10000;
$batchObj = new BatchInsert($myDb, 'trade_pending_list', [
    'id',
    'itemid',
    'price',
    'count',
    'sellerid',
    'name',
    'pendingtime',
    'refine_lv',
    'itemdata',
    'is_overlap',
    'publicity_id',
    'endtime'
]);

try
{
    $spendTime = microtime(1);

    while (true)
    {
        $rs = $db->query("SELECT * FROM `trade_goods` WHERE `id` > {$goodsId} LIMIT {$limit}");
        if (!$rs || !$rs->num_rows)
        {
            break;
        }

        while ($data = $rs->fetch_object())
        {
            $goodsId = $data->id;

            if (!in_array($data->status, [\RO\Trade\Dao\Goods::STATUS_SELLING, \RO\Trade\Dao\Goods::STATUS_EXPIRED]))
            {
                continue;
            }

            $itemData = null;
            $itemDataObj = null;

            if ($data->is_overlap == 0)
            {
                $itemData    = !$data->item_data ? null : $data->item_data;
                $itemDataObj = !$data->item_data ? null : new \RO\Cmd\ItemData($data->item_data);
            }

            $refineLv    = $itemDataObj === null ? 0 : $itemDataObj->equip->refinelv;
            if (isset($itemDataObj) && !isset($itemDataObj->equip))
            {
                var_dump($itemData);
                var_dump($goodsId);
            }

            $publicityId = 0;
            if ($data->is_publicity)
            {
                $publicityId = $data->item_list_id;
            }

            $item = \RO\Trade\Item::get($data->item_id, $itemDataObj);

            $batchObj->addAndBatchExecute([
                                              $data->id,
                                              $data->item_id,
                                              $item->getPrice(),
                                              $data->stock,
                                              $data->char_id,
                                              $data->player_name,
                                              $data->time,
                                              $refineLv,
                                              $itemData === null ? null : new Hex($itemData),
                                              $data->is_overlap,
                                              $publicityId,
                                              $data->end_time
                                          ]);
        }

        $rs->free();
        logger("trade_goods数据迁移成功, 迁移最后的id为:{$goodsId}");
    }

    $batchObj->execute();
    unset($batchObj);
    logger('挂单记录迁移耗时:' . (microtime(1) - $spendTime), true);
}
catch (Exception $e)
{
    warn($e->getMessage());
    warn('trade_goods失败的最后id为:' . $goodsId);
}

goodsEnd:

// ---------------------------- 卖家记录迁移 --------------------------

$sellSwitch = Server::$config['trade']['sell']['open'];
if ($sellSwitch === false)
{
    goto sellEnd;
}

$soldId        = 0;
$limit         = 10000;
$expiredTime   = time() - (3 * 86400);
$expired30time = time() - (30 * 86400);

$batchObj = new BatchInsert($myDb, 'trade_saled_list', [
    'id',
    'itemid',
    'price',
    'count',
    'sellerid',
    'buyerid',
    'pendingtime',
    'tradetime',
    'refine_lv',
    'itemdata',
    'logtype',
    'tax',
    'status',
    'damage',
    'buyerinfo',
]);

try
{
    $spendTime = microtime(1);

    while (true)
    {
        $rs = $db->query("SELECT `s`.*, `g`.`item_data` AS `goods_item_data` FROM `trade_record_sold` AS `s` LEFT JOIN `trade_goods` AS `g` ON `s`.`goods_id` = `g`.`id` WHERE `s`.`id` > {$soldId} LIMIT {$limit}");
        if (!$rs || !$rs->num_rows)
        {
            break;
        }

        while ($data = $rs->fetch_object())
        {
            $soldId = $data->id;

            // 已领取且已超过3天的去掉
            if ($data->take_status == 1 && $data->time <= $expiredTime)
            {
                var_dump($soldId);
                continue;
            }

            // 未领取的记录且超过30天的去掉
            if ($data->take_status == 0 && $data->time <= $expired30time)
            {
                continue;
            }

            if ($data->goods_id == 0)
            {
                $itemData = empty($data->item_data) ? null : $data->item_data;
            }
            else
            {
                $itemData = empty($data->goods_item_data) ? null : $data->goods_item_data;
            }

            $itemDataObj = empty($itemData) ? null : new \RO\Cmd\ItemData($itemData);
            $damage      = $itemDataObj === null ? 0 : $itemDataObj->equip->damage ? 1 : 0;
            $refineLv    = $itemDataObj === null ? 0 : $itemDataObj->equip->refinelv;

            $buyerInfo = null;
            if ($data->is_many_people)
            {
                $buyerInfo = $data->buyers_info;
            }
            else
            {
                $info = new \RO\Cmd\NameInfo($data->buyer_info);
                $buyerInfo = new \RO\Cmd\NameInfoList();
                $buyerInfo->addNameInfos($info);
                $buyerInfo = $buyerInfo->serialize();
            }

            $batchObj->addAndBatchExecute([
                                              $soldId,
                                              $data->item_id,
                                              $data->price,
                                              $data->count,
                                              $data->char_id,
                                              0,
                                              $data->time,
                                              $data->time,
                                              $refineLv,
                                              $itemData === null ? null : new Hex($itemData),
                                              \RO\Cmd\EOperType::EOperType_NormalSell,
                                              $data->tax,
                                              $data->take_status,
                                              $damage,
                                              empty($buyerInfo) ? null : new Hex($buyerInfo)
                                          ]);

            logger("trade_record_sold数据迁移成功, 迁移最后的id为:{$soldId}");
        }
    }

    $batchObj->execute();
    unset($batchObj);
    logger('卖家记录迁移耗时:' . (microtime(1) - $spendTime), true);
}
catch (Exception $e)
{
    warn($e->getMessage());
    warn('trade_goods失败的最后id为:' . $soldId);
}

sellEnd:

//----------------------------- 买家交易记录迁移 ------------------------------
$buySwitch = Server::$config['trade']['buy']['open'];
if ($buySwitch === false)
{
    goto buyEnd;
}

$limit         = 10000;
$expiredTime   = time() - (3 * 86400);
$buyId         = 0;
$expired30time = time() - (30 * 86400);
$batchObj      = new BatchInsert($myDb, 'trade_buyed_list', [
    'id',
    'itemid',
    'price',
    'count',
    'sellerid',
    'buyerid',
    'buyername',
    'pendingtime',
    'tradetime',
    'refine_lv',
    'itemdata',
    'logtype',
    'failcount',
    'status',
    'endtime',
    'damage',
    'totalcount',
    'sellerinfo',
]);

try
{
    $spendTime = microtime(1);

    while (true)
    {
        $rs = $db->query("SELECT * FROM `trade_record_bought` WHERE `id` > {$buyId} LIMIT {$limit}");
        if (!$rs || !$rs->num_rows)
        {
            break;
        }

        while ($data = $rs->fetch_object())
        {
            $buyId = $data->id;

            // 已领取且已超过3天的去掉
            if ($data->take_status == 1 && $data->time <= $expiredTime)
            {
                continue;
            }

            // 未领取的记录且超过30天的去掉
            if ($data->time <= $expired30time)
            {
                continue;
            }

            switch ($data->status)
            {
                case \RO\Trade\Dao\RecordBought::STATUS_PUBLICITY_PAY_SUCCESS:
                    $logType = \RO\Cmd\EOperType::EOperType_PublicityBuying;
                    break;
                case \RO\Trade\Dao\RecordBought::STATUS_PUBLICITY_CANCEL:
                    $logType = \RO\Cmd\EOperType::EOperType_PublicityBuyFail;
                    break;
                case \RO\Trade\Dao\RecordBought::STATUS_PUBLICITY_SUCCESS:
                    $logType = \RO\Cmd\EOperType::EOperType_PublicityBuySuccess;
                    break;
                default:
                    $logType = \RO\Cmd\EOperType::EOperType_NoramlBuy;
                    break;
            }

            $totalCount = $data->total_count;
            $failCount  = 0;
            $count      = $data->count;
            switch ($logType)
            {
                case \RO\Cmd\EOperType::EOperType_NoramlBuy:
                    $totalCount = 0;
                    break;
                case \RO\Cmd\EOperType::EOperType_PublicityBuying:
                    $totalCount = 0;
                    break;
                case \RO\Cmd\EOperType::EOperType_PublicityBuyFail:
                    $failCount = $count;
                    break;
            }

            $itemData    = empty($data->item_data) ? null : $data->item_data;
            $itemDataObj = empty($itemData) ? null : new \RO\Cmd\ItemData($itemData);
            $damage      = $itemDataObj === null ? 0 : $itemDataObj->equip->damage ? 1 : 0;
            $refineLv    = $itemDataObj === null ? 0 : $itemDataObj->equip->refinelv;

            $sellInfo = null;
            if ($data->is_many_people)
            {
                $sellInfo = $data->sellers_info;
            }
            else
            {
                $info = new \RO\Cmd\NameInfo($data->seller_info);
                $sellInfo = new \RO\Cmd\NameInfoList();
                $sellInfo->addNameInfos($info);
                $sellInfo = $sellInfo->serialize();
            }

            $batchObj->addAndBatchExecute([
                                              $data->id,
                                              $data->item_id,
                                              $data->price,
                                              $count,
                                              0,
                                              $data->char_id,
                                              $data->player_name,
                                              $data->time,
                                              $data->time,
                                              $refineLv,
                                              $itemData === null ? null : new Hex($itemData),
                                              $logType,
                                              $failCount,
                                              $data->take_status,
                                              $data->end_time,
                                              $damage,
                                              $totalCount,
                                              empty($sellInfo) ? null : new Hex($sellInfo)
                                          ]);

            logger("trade_record_bought数据迁移成功, 迁移最后的id为:{$buyId}");
        }
    }

    $batchObj->execute();
    unset($batchObj);
    logger('买家记录迁移耗时:' . (microtime(1) - $spendTime), true);
}
catch (Exception $e)
{
    warn($e->getMessage());
    warn('trade_goods失败的最后id为:' . $buyId);
}

buyEnd:
<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordTrade.proto

namespace RO\Cmd {

  class ItemSellInfoRecordTradeCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::RECORD_USER_TRADE_PROTOCMD;
    
    /**  @var int - \RO\Cmd\RecordUserTradeParam */
    public $param = \RO\Cmd\RecordUserTradeParam::ITEM_SELL_INFO_RECORDTRADE;
    
    /**  @var int */
    public $charid = null;
    
    /**  @var int */
    public $itemid = null;
    
    /**  @var int */
    public $refine_lv = null;
    
    /**  @var int */
    public $publicity_id = null;
    
    /**  @var int - \RO\Cmd\StateType */
    public $statetype = null;
    
    /**  @var int */
    public $count = 0;
    
    /**  @var int */
    public $buyer_count = 0;
    
    /**  @var \RO\Cmd\BriefBuyInfo[]  */
    public $buy_info = array();
    
    /**  @var int */
    public $order_id = null;
    
    /**  @var int - \RO\Cmd\ETradeType */
    public $type = \RO\Cmd\ETradeType::ETRADETYPE_TRADE;
    
    /**  @var int */
    public $quota = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.ItemSellInfoRecordTradeCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::RECORD_USER_TRADE_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\RecordUserTradeParam';
      $f->default   = \RO\Cmd\RecordUserTradeParam::ITEM_SELL_INFO_RECORDTRADE;
      $descriptor->addField($f);

      // OPTIONAL UINT64 charid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "charid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 itemid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "itemid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 refine_lv = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "refine_lv";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 publicity_id = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "publicity_id";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL ENUM statetype = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "statetype";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\StateType';
      $descriptor->addField($f);

      // OPTIONAL UINT32 count = 8
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 8;
      $f->name      = "count";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 buyer_count = 9
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 9;
      $f->name      = "buyer_count";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // REPEATED MESSAGE buy_info = 10
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 10;
      $f->name      = "buy_info";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\BriefBuyInfo';
      $descriptor->addField($f);

      // OPTIONAL UINT64 order_id = 11
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 11;
      $f->name      = "order_id";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL ENUM type = 12
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 12;
      $f->name      = "type";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ETradeType';
      $f->default   = \RO\Cmd\ETradeType::ETRADETYPE_TRADE;
      $descriptor->addField($f);

      // OPTIONAL UINT64 quota = 13
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 13;
      $f->name      = "quota";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <cmd> has a value
     *
     * @return boolean
     */
    public function hasCmd(){
      return $this->_has(1);
    }
    
    /**
     * Clear <cmd> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearCmd(){
      return $this->_clear(1);
    }
    
    /**
     * Get <cmd> value
     *
     * @return int - \RO\Cmd\Command
     */
    public function getCmd(){
      return $this->_get(1);
    }
    
    /**
     * Set <cmd> value
     *
     * @param int - \RO\Cmd\Command $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setCmd( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <param> has a value
     *
     * @return boolean
     */
    public function hasParam(){
      return $this->_has(2);
    }
    
    /**
     * Clear <param> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\RecordUserTradeParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\RecordUserTradeParam $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <charid> has a value
     *
     * @return boolean
     */
    public function hasCharid(){
      return $this->_has(3);
    }
    
    /**
     * Clear <charid> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearCharid(){
      return $this->_clear(3);
    }
    
    /**
     * Get <charid> value
     *
     * @return int
     */
    public function getCharid(){
      return $this->_get(3);
    }
    
    /**
     * Set <charid> value
     *
     * @param int $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setCharid( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <itemid> has a value
     *
     * @return boolean
     */
    public function hasItemid(){
      return $this->_has(4);
    }
    
    /**
     * Clear <itemid> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearItemid(){
      return $this->_clear(4);
    }
    
    /**
     * Get <itemid> value
     *
     * @return int
     */
    public function getItemid(){
      return $this->_get(4);
    }
    
    /**
     * Set <itemid> value
     *
     * @param int $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setItemid( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <refine_lv> has a value
     *
     * @return boolean
     */
    public function hasRefineLv(){
      return $this->_has(5);
    }
    
    /**
     * Clear <refine_lv> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearRefineLv(){
      return $this->_clear(5);
    }
    
    /**
     * Get <refine_lv> value
     *
     * @return int
     */
    public function getRefineLv(){
      return $this->_get(5);
    }
    
    /**
     * Set <refine_lv> value
     *
     * @param int $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setRefineLv( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <publicity_id> has a value
     *
     * @return boolean
     */
    public function hasPublicityId(){
      return $this->_has(6);
    }
    
    /**
     * Clear <publicity_id> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearPublicityId(){
      return $this->_clear(6);
    }
    
    /**
     * Get <publicity_id> value
     *
     * @return int
     */
    public function getPublicityId(){
      return $this->_get(6);
    }
    
    /**
     * Set <publicity_id> value
     *
     * @param int $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setPublicityId( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <statetype> has a value
     *
     * @return boolean
     */
    public function hasStatetype(){
      return $this->_has(7);
    }
    
    /**
     * Clear <statetype> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearStatetype(){
      return $this->_clear(7);
    }
    
    /**
     * Get <statetype> value
     *
     * @return int - \RO\Cmd\StateType
     */
    public function getStatetype(){
      return $this->_get(7);
    }
    
    /**
     * Set <statetype> value
     *
     * @param int - \RO\Cmd\StateType $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setStatetype( $value){
      return $this->_set(7, $value);
    }
    
    /**
     * Check if <count> has a value
     *
     * @return boolean
     */
    public function hasCount(){
      return $this->_has(8);
    }
    
    /**
     * Clear <count> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearCount(){
      return $this->_clear(8);
    }
    
    /**
     * Get <count> value
     *
     * @return int
     */
    public function getCount(){
      return $this->_get(8);
    }
    
    /**
     * Set <count> value
     *
     * @param int $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setCount( $value){
      return $this->_set(8, $value);
    }
    
    /**
     * Check if <buyer_count> has a value
     *
     * @return boolean
     */
    public function hasBuyerCount(){
      return $this->_has(9);
    }
    
    /**
     * Clear <buyer_count> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearBuyerCount(){
      return $this->_clear(9);
    }
    
    /**
     * Get <buyer_count> value
     *
     * @return int
     */
    public function getBuyerCount(){
      return $this->_get(9);
    }
    
    /**
     * Set <buyer_count> value
     *
     * @param int $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setBuyerCount( $value){
      return $this->_set(9, $value);
    }
    
    /**
     * Check if <buy_info> has a value
     *
     * @return boolean
     */
    public function hasBuyInfo(){
      return $this->_has(10);
    }
    
    /**
     * Clear <buy_info> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearBuyInfo(){
      return $this->_clear(10);
    }
    
    /**
     * Get <buy_info> value
     *
     * @param int $idx
     * @return \RO\Cmd\BriefBuyInfo
     */
    public function getBuyInfo($idx = NULL){
      return $this->_get(10, $idx);
    }
    
    /**
     * Set <buy_info> value
     *
     * @param \RO\Cmd\BriefBuyInfo $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setBuyInfo(\RO\Cmd\BriefBuyInfo $value, $idx = NULL){
      return $this->_set(10, $value, $idx);
    }
    
    /**
     * Get all elements of <buy_info>
     *
     * @return \RO\Cmd\BriefBuyInfo[]
     */
    public function getBuyInfoList(){
     return $this->_get(10);
    }
    
    /**
     * Add a new element to <buy_info>
     *
     * @param \RO\Cmd\BriefBuyInfo $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function addBuyInfo(\RO\Cmd\BriefBuyInfo $value){
     return $this->_add(10, $value);
    }
    
    /**
     * Check if <order_id> has a value
     *
     * @return boolean
     */
    public function hasOrderId(){
      return $this->_has(11);
    }
    
    /**
     * Clear <order_id> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearOrderId(){
      return $this->_clear(11);
    }
    
    /**
     * Get <order_id> value
     *
     * @return int
     */
    public function getOrderId(){
      return $this->_get(11);
    }
    
    /**
     * Set <order_id> value
     *
     * @param int $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setOrderId( $value){
      return $this->_set(11, $value);
    }
    
    /**
     * Check if <type> has a value
     *
     * @return boolean
     */
    public function hasType(){
      return $this->_has(12);
    }
    
    /**
     * Clear <type> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearType(){
      return $this->_clear(12);
    }
    
    /**
     * Get <type> value
     *
     * @return int - \RO\Cmd\ETradeType
     */
    public function getType(){
      return $this->_get(12);
    }
    
    /**
     * Set <type> value
     *
     * @param int - \RO\Cmd\ETradeType $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setType( $value){
      return $this->_set(12, $value);
    }
    
    /**
     * Check if <quota> has a value
     *
     * @return boolean
     */
    public function hasQuota(){
      return $this->_has(13);
    }
    
    /**
     * Clear <quota> value
     *
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function clearQuota(){
      return $this->_clear(13);
    }
    
    /**
     * Get <quota> value
     *
     * @return int
     */
    public function getQuota(){
      return $this->_get(13);
    }
    
    /**
     * Set <quota> value
     *
     * @param int $value
     * @return \RO\Cmd\ItemSellInfoRecordTradeCmd
     */
    public function setQuota( $value){
      return $this->_set(13, $value);
    }
  }
}


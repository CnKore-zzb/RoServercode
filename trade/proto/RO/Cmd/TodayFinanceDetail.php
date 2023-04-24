<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordTrade.proto

namespace RO\Cmd {

  class TodayFinanceDetail extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::RECORD_USER_TRADE_PROTOCMD;
    
    /**  @var int - \RO\Cmd\RecordUserTradeParam */
    public $param = \RO\Cmd\RecordUserTradeParam::QUERY_SERVANT_FINANCE_DETAIL;
    
    /**  @var int */
    public $item_id = null;
    
    /**  @var int - \RO\Cmd\EFinanceRankType */
    public $rank_type = \RO\Cmd\EFinanceRankType::EFINANCE_RANK_DEALCOUNT;
    
    /**  @var int - \RO\Cmd\EFinanceDateType */
    public $date_type = \RO\Cmd\EFinanceDateType::EFINANCE_DATE_THREE;
    
    /**  @var \RO\Cmd\TodayFinanceItem[]  */
    public $lists = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.TodayFinanceDetail');

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
      $f->default   = \RO\Cmd\RecordUserTradeParam::QUERY_SERVANT_FINANCE_DETAIL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 item_id = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "item_id";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL ENUM rank_type = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "rank_type";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EFinanceRankType';
      $f->default   = \RO\Cmd\EFinanceRankType::EFINANCE_RANK_DEALCOUNT;
      $descriptor->addField($f);

      // OPTIONAL ENUM date_type = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "date_type";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EFinanceDateType';
      $f->default   = \RO\Cmd\EFinanceDateType::EFINANCE_DATE_THREE;
      $descriptor->addField($f);

      // REPEATED MESSAGE lists = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "lists";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\TodayFinanceItem';
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
     * @return \RO\Cmd\TodayFinanceDetail
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
     * @return \RO\Cmd\TodayFinanceDetail
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
     * @return \RO\Cmd\TodayFinanceDetail
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
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <item_id> has a value
     *
     * @return boolean
     */
    public function hasItemId(){
      return $this->_has(3);
    }
    
    /**
     * Clear <item_id> value
     *
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function clearItemId(){
      return $this->_clear(3);
    }
    
    /**
     * Get <item_id> value
     *
     * @return int
     */
    public function getItemId(){
      return $this->_get(3);
    }
    
    /**
     * Set <item_id> value
     *
     * @param int $value
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function setItemId( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <rank_type> has a value
     *
     * @return boolean
     */
    public function hasRankType(){
      return $this->_has(4);
    }
    
    /**
     * Clear <rank_type> value
     *
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function clearRankType(){
      return $this->_clear(4);
    }
    
    /**
     * Get <rank_type> value
     *
     * @return int - \RO\Cmd\EFinanceRankType
     */
    public function getRankType(){
      return $this->_get(4);
    }
    
    /**
     * Set <rank_type> value
     *
     * @param int - \RO\Cmd\EFinanceRankType $value
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function setRankType( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <date_type> has a value
     *
     * @return boolean
     */
    public function hasDateType(){
      return $this->_has(5);
    }
    
    /**
     * Clear <date_type> value
     *
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function clearDateType(){
      return $this->_clear(5);
    }
    
    /**
     * Get <date_type> value
     *
     * @return int - \RO\Cmd\EFinanceDateType
     */
    public function getDateType(){
      return $this->_get(5);
    }
    
    /**
     * Set <date_type> value
     *
     * @param int - \RO\Cmd\EFinanceDateType $value
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function setDateType( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <lists> has a value
     *
     * @return boolean
     */
    public function hasLists(){
      return $this->_has(6);
    }
    
    /**
     * Clear <lists> value
     *
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function clearLists(){
      return $this->_clear(6);
    }
    
    /**
     * Get <lists> value
     *
     * @param int $idx
     * @return \RO\Cmd\TodayFinanceItem
     */
    public function getLists($idx = NULL){
      return $this->_get(6, $idx);
    }
    
    /**
     * Set <lists> value
     *
     * @param \RO\Cmd\TodayFinanceItem $value
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function setLists(\RO\Cmd\TodayFinanceItem $value, $idx = NULL){
      return $this->_set(6, $value, $idx);
    }
    
    /**
     * Get all elements of <lists>
     *
     * @return \RO\Cmd\TodayFinanceItem[]
     */
    public function getListsList(){
     return $this->_get(6);
    }
    
    /**
     * Add a new element to <lists>
     *
     * @param \RO\Cmd\TodayFinanceItem $value
     * @return \RO\Cmd\TodayFinanceDetail
     */
    public function addLists(\RO\Cmd\TodayFinanceItem $value){
     return $this->_add(6, $value);
    }
  }
}


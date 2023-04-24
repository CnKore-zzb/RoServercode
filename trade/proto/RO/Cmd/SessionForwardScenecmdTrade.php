<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneTrade.proto

namespace RO\Cmd {

  class SessionForwardScenecmdTrade extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::TRADE_PROTOCMD;
    
    /**  @var int - \RO\Cmd\RecordServerTradeParam */
    public $param = \RO\Cmd\RecordServerTradeParam::SESSION_FORWARD_SCENECMD_TRADE;
    
    /**  @var int */
    public $charid = null;
    
    /**  @var int */
    public $zoneid = null;
    
    /**  @var string */
    public $name = null;
    
    /**  @var string */
    public $data = null;
    
    /**  @var int */
    public $len = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.SessionForwardScenecmdTrade');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::TRADE_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\RecordServerTradeParam';
      $f->default   = \RO\Cmd\RecordServerTradeParam::SESSION_FORWARD_SCENECMD_TRADE;
      $descriptor->addField($f);

      // OPTIONAL UINT64 charid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "charid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 zoneid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "zoneid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL STRING name = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "name";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL BYTES data = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "data";
      $f->type      = \DrSlump\Protobuf::TYPE_BYTES;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 len = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "len";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
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
     * @return \RO\Cmd\SessionForwardScenecmdTrade
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
     * @return \RO\Cmd\SessionForwardScenecmdTrade
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
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\RecordServerTradeParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\RecordServerTradeParam $value
     * @return \RO\Cmd\SessionForwardScenecmdTrade
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
     * @return \RO\Cmd\SessionForwardScenecmdTrade
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
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function setCharid( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <zoneid> has a value
     *
     * @return boolean
     */
    public function hasZoneid(){
      return $this->_has(4);
    }
    
    /**
     * Clear <zoneid> value
     *
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function clearZoneid(){
      return $this->_clear(4);
    }
    
    /**
     * Get <zoneid> value
     *
     * @return int
     */
    public function getZoneid(){
      return $this->_get(4);
    }
    
    /**
     * Set <zoneid> value
     *
     * @param int $value
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function setZoneid( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <name> has a value
     *
     * @return boolean
     */
    public function hasName(){
      return $this->_has(5);
    }
    
    /**
     * Clear <name> value
     *
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function clearName(){
      return $this->_clear(5);
    }
    
    /**
     * Get <name> value
     *
     * @return string
     */
    public function getName(){
      return $this->_get(5);
    }
    
    /**
     * Set <name> value
     *
     * @param string $value
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function setName( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <data> has a value
     *
     * @return boolean
     */
    public function hasData(){
      return $this->_has(6);
    }
    
    /**
     * Clear <data> value
     *
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function clearData(){
      return $this->_clear(6);
    }
    
    /**
     * Get <data> value
     *
     * @return string
     */
    public function getData(){
      return $this->_get(6);
    }
    
    /**
     * Set <data> value
     *
     * @param string $value
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function setData( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <len> has a value
     *
     * @return boolean
     */
    public function hasLen(){
      return $this->_has(7);
    }
    
    /**
     * Clear <len> value
     *
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function clearLen(){
      return $this->_clear(7);
    }
    
    /**
     * Get <len> value
     *
     * @return int
     */
    public function getLen(){
      return $this->_get(7);
    }
    
    /**
     * Set <len> value
     *
     * @param int $value
     * @return \RO\Cmd\SessionForwardScenecmdTrade
     */
    public function setLen( $value){
      return $this->_set(7, $value);
    }
  }
}


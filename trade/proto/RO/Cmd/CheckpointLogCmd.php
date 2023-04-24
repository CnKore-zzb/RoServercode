<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: LogCmd.proto

namespace RO\Cmd {

  class CheckpointLogCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::LOG_PROTOCMD;
    
    /**  @var int - \RO\Cmd\LogParam */
    public $param = \RO\Cmd\LogParam::CHECKPOINT_LOG_CMD;
    
    /**  @var int */
    public $cid = null;
    
    /**  @var int */
    public $sid = null;
    
    /**  @var int */
    public $hid = null;
    
    /**  @var string */
    public $account = null;
    
    /**  @var int */
    public $pid = null;
    
    /**  @var int */
    public $eid = null;
    
    /**  @var int */
    public $time = null;
    
    /**  @var int */
    public $type = null;
    
    /**  @var int */
    public $cpid = null;
    
    /**  @var int */
    public $result = null;
    
    /**  @var int */
    public $star = null;
    
    /**  @var int */
    public $ispay = null;
    
    /**  @var int */
    public $vip = null;
    
    /**  @var string */
    public $logid = null;
    
    /**  @var int */
    public $isfirst = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.CheckpointLogCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::LOG_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\LogParam';
      $f->default   = \RO\Cmd\LogParam::CHECKPOINT_LOG_CMD;
      $descriptor->addField($f);

      // OPTIONAL UINT32 cid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "cid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 sid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "sid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 hid = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "hid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL STRING account = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "account";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT64 pid = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "pid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT64 eid = 8
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 8;
      $f->name      = "eid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 time = 9
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 9;
      $f->name      = "time";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 type = 10
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 10;
      $f->name      = "type";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 cpid = 11
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 11;
      $f->name      = "cpid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 result = 12
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 12;
      $f->name      = "result";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 star = 13
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 13;
      $f->name      = "star";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 ispay = 14
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 14;
      $f->name      = "ispay";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 vip = 15
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 15;
      $f->name      = "vip";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL STRING logid = 16
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 16;
      $f->name      = "logid";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 isfirst = 17
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 17;
      $f->name      = "isfirst";
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
     * @return \RO\Cmd\CheckpointLogCmd
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
     * @return \RO\Cmd\CheckpointLogCmd
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
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\LogParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\LogParam $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <cid> has a value
     *
     * @return boolean
     */
    public function hasCid(){
      return $this->_has(3);
    }
    
    /**
     * Clear <cid> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearCid(){
      return $this->_clear(3);
    }
    
    /**
     * Get <cid> value
     *
     * @return int
     */
    public function getCid(){
      return $this->_get(3);
    }
    
    /**
     * Set <cid> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setCid( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <sid> has a value
     *
     * @return boolean
     */
    public function hasSid(){
      return $this->_has(4);
    }
    
    /**
     * Clear <sid> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearSid(){
      return $this->_clear(4);
    }
    
    /**
     * Get <sid> value
     *
     * @return int
     */
    public function getSid(){
      return $this->_get(4);
    }
    
    /**
     * Set <sid> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setSid( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <hid> has a value
     *
     * @return boolean
     */
    public function hasHid(){
      return $this->_has(5);
    }
    
    /**
     * Clear <hid> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearHid(){
      return $this->_clear(5);
    }
    
    /**
     * Get <hid> value
     *
     * @return int
     */
    public function getHid(){
      return $this->_get(5);
    }
    
    /**
     * Set <hid> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setHid( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <account> has a value
     *
     * @return boolean
     */
    public function hasAccount(){
      return $this->_has(6);
    }
    
    /**
     * Clear <account> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearAccount(){
      return $this->_clear(6);
    }
    
    /**
     * Get <account> value
     *
     * @return string
     */
    public function getAccount(){
      return $this->_get(6);
    }
    
    /**
     * Set <account> value
     *
     * @param string $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setAccount( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <pid> has a value
     *
     * @return boolean
     */
    public function hasPid(){
      return $this->_has(7);
    }
    
    /**
     * Clear <pid> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearPid(){
      return $this->_clear(7);
    }
    
    /**
     * Get <pid> value
     *
     * @return int
     */
    public function getPid(){
      return $this->_get(7);
    }
    
    /**
     * Set <pid> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setPid( $value){
      return $this->_set(7, $value);
    }
    
    /**
     * Check if <eid> has a value
     *
     * @return boolean
     */
    public function hasEid(){
      return $this->_has(8);
    }
    
    /**
     * Clear <eid> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearEid(){
      return $this->_clear(8);
    }
    
    /**
     * Get <eid> value
     *
     * @return int
     */
    public function getEid(){
      return $this->_get(8);
    }
    
    /**
     * Set <eid> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setEid( $value){
      return $this->_set(8, $value);
    }
    
    /**
     * Check if <time> has a value
     *
     * @return boolean
     */
    public function hasTime(){
      return $this->_has(9);
    }
    
    /**
     * Clear <time> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearTime(){
      return $this->_clear(9);
    }
    
    /**
     * Get <time> value
     *
     * @return int
     */
    public function getTime(){
      return $this->_get(9);
    }
    
    /**
     * Set <time> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setTime( $value){
      return $this->_set(9, $value);
    }
    
    /**
     * Check if <type> has a value
     *
     * @return boolean
     */
    public function hasType(){
      return $this->_has(10);
    }
    
    /**
     * Clear <type> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearType(){
      return $this->_clear(10);
    }
    
    /**
     * Get <type> value
     *
     * @return int
     */
    public function getType(){
      return $this->_get(10);
    }
    
    /**
     * Set <type> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setType( $value){
      return $this->_set(10, $value);
    }
    
    /**
     * Check if <cpid> has a value
     *
     * @return boolean
     */
    public function hasCpid(){
      return $this->_has(11);
    }
    
    /**
     * Clear <cpid> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearCpid(){
      return $this->_clear(11);
    }
    
    /**
     * Get <cpid> value
     *
     * @return int
     */
    public function getCpid(){
      return $this->_get(11);
    }
    
    /**
     * Set <cpid> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setCpid( $value){
      return $this->_set(11, $value);
    }
    
    /**
     * Check if <result> has a value
     *
     * @return boolean
     */
    public function hasResult(){
      return $this->_has(12);
    }
    
    /**
     * Clear <result> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearResult(){
      return $this->_clear(12);
    }
    
    /**
     * Get <result> value
     *
     * @return int
     */
    public function getResult(){
      return $this->_get(12);
    }
    
    /**
     * Set <result> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setResult( $value){
      return $this->_set(12, $value);
    }
    
    /**
     * Check if <star> has a value
     *
     * @return boolean
     */
    public function hasStar(){
      return $this->_has(13);
    }
    
    /**
     * Clear <star> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearStar(){
      return $this->_clear(13);
    }
    
    /**
     * Get <star> value
     *
     * @return int
     */
    public function getStar(){
      return $this->_get(13);
    }
    
    /**
     * Set <star> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setStar( $value){
      return $this->_set(13, $value);
    }
    
    /**
     * Check if <ispay> has a value
     *
     * @return boolean
     */
    public function hasIspay(){
      return $this->_has(14);
    }
    
    /**
     * Clear <ispay> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearIspay(){
      return $this->_clear(14);
    }
    
    /**
     * Get <ispay> value
     *
     * @return int
     */
    public function getIspay(){
      return $this->_get(14);
    }
    
    /**
     * Set <ispay> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setIspay( $value){
      return $this->_set(14, $value);
    }
    
    /**
     * Check if <vip> has a value
     *
     * @return boolean
     */
    public function hasVip(){
      return $this->_has(15);
    }
    
    /**
     * Clear <vip> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearVip(){
      return $this->_clear(15);
    }
    
    /**
     * Get <vip> value
     *
     * @return int
     */
    public function getVip(){
      return $this->_get(15);
    }
    
    /**
     * Set <vip> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setVip( $value){
      return $this->_set(15, $value);
    }
    
    /**
     * Check if <logid> has a value
     *
     * @return boolean
     */
    public function hasLogid(){
      return $this->_has(16);
    }
    
    /**
     * Clear <logid> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearLogid(){
      return $this->_clear(16);
    }
    
    /**
     * Get <logid> value
     *
     * @return string
     */
    public function getLogid(){
      return $this->_get(16);
    }
    
    /**
     * Set <logid> value
     *
     * @param string $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setLogid( $value){
      return $this->_set(16, $value);
    }
    
    /**
     * Check if <isfirst> has a value
     *
     * @return boolean
     */
    public function hasIsfirst(){
      return $this->_has(17);
    }
    
    /**
     * Clear <isfirst> value
     *
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function clearIsfirst(){
      return $this->_clear(17);
    }
    
    /**
     * Get <isfirst> value
     *
     * @return int
     */
    public function getIsfirst(){
      return $this->_get(17);
    }
    
    /**
     * Set <isfirst> value
     *
     * @param int $value
     * @return \RO\Cmd\CheckpointLogCmd
     */
    public function setIsfirst( $value){
      return $this->_set(17, $value);
    }
  }
}


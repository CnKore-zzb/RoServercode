<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SessionCmd.proto

namespace RO\Cmd {

  class SyncOperateRewardSessionCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SESSION_PROTOCMD;
    
    /**  @var int - \RO\Cmd\SessionParam */
    public $param = \RO\Cmd\SessionParam::SESSIONPARAM_OPERATE_REWARD;
    
    /**  @var int */
    public $charid = 0;
    
    /**  @var int */
    public $var = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.SyncOperateRewardSessionCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SESSION_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\SessionParam';
      $f->default   = \RO\Cmd\SessionParam::SESSIONPARAM_OPERATE_REWARD;
      $descriptor->addField($f);

      // OPTIONAL UINT64 charid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "charid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 var = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "var";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
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
     * @return \RO\Cmd\SyncOperateRewardSessionCmd
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
     * @return \RO\Cmd\SyncOperateRewardSessionCmd
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
     * @return \RO\Cmd\SyncOperateRewardSessionCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\SessionParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\SessionParam $value
     * @return \RO\Cmd\SyncOperateRewardSessionCmd
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
     * @return \RO\Cmd\SyncOperateRewardSessionCmd
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
     * @return \RO\Cmd\SyncOperateRewardSessionCmd
     */
    public function setCharid( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <var> has a value
     *
     * @return boolean
     */
    public function hasVar(){
      return $this->_has(4);
    }
    
    /**
     * Clear <var> value
     *
     * @return \RO\Cmd\SyncOperateRewardSessionCmd
     */
    public function clearVar(){
      return $this->_clear(4);
    }
    
    /**
     * Get <var> value
     *
     * @return int
     */
    public function getVar(){
      return $this->_get(4);
    }
    
    /**
     * Set <var> value
     *
     * @param int $value
     * @return \RO\Cmd\SyncOperateRewardSessionCmd
     */
    public function setVar( $value){
      return $this->_set(4, $value);
    }
  }
}


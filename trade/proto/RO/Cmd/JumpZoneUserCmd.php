<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneUser2.proto

namespace RO\Cmd {

  class JumpZoneUserCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER2_PROTOCMD;
    
    /**  @var int - \RO\Cmd\User2Param */
    public $param = \RO\Cmd\User2Param::USER2PARAM_JUMP_ZONE;
    
    /**  @var int */
    public $npcid = 0;
    
    /**  @var int */
    public $zoneid = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.JumpZoneUserCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SCENE_USER2_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\User2Param';
      $f->default   = \RO\Cmd\User2Param::USER2PARAM_JUMP_ZONE;
      $descriptor->addField($f);

      // OPTIONAL UINT64 npcid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "npcid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 zoneid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "zoneid";
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
     * @return \RO\Cmd\JumpZoneUserCmd
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
     * @return \RO\Cmd\JumpZoneUserCmd
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
     * @return \RO\Cmd\JumpZoneUserCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\User2Param
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\User2Param $value
     * @return \RO\Cmd\JumpZoneUserCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <npcid> has a value
     *
     * @return boolean
     */
    public function hasNpcid(){
      return $this->_has(3);
    }
    
    /**
     * Clear <npcid> value
     *
     * @return \RO\Cmd\JumpZoneUserCmd
     */
    public function clearNpcid(){
      return $this->_clear(3);
    }
    
    /**
     * Get <npcid> value
     *
     * @return int
     */
    public function getNpcid(){
      return $this->_get(3);
    }
    
    /**
     * Set <npcid> value
     *
     * @param int $value
     * @return \RO\Cmd\JumpZoneUserCmd
     */
    public function setNpcid( $value){
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
     * @return \RO\Cmd\JumpZoneUserCmd
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
     * @return \RO\Cmd\JumpZoneUserCmd
     */
    public function setZoneid( $value){
      return $this->_set(4, $value);
    }
  }
}

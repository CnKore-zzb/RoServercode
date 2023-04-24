<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneMap.proto

namespace RO\Cmd {

  class GingerBreadNpcCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER_MAP_PROTOCMD;
    
    /**  @var int - \RO\Cmd\MapParam */
    public $param = \RO\Cmd\MapParam::MAPPARAM_GINGERBREAD_NPC;
    
    /**  @var \RO\Cmd\GingerBreadNpcData */
    public $data = null;
    
    /**  @var boolean */
    public $isadd = true;
    
    /**  @var int */
    public $userid = 0;
    
    /**  @var \RO\Cmd\ScenePos */
    public $bornpos = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.GingerBreadNpcCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SCENE_USER_MAP_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\MapParam';
      $f->default   = \RO\Cmd\MapParam::MAPPARAM_GINGERBREAD_NPC;
      $descriptor->addField($f);

      // OPTIONAL MESSAGE data = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "data";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\GingerBreadNpcData';
      $descriptor->addField($f);

      // OPTIONAL BOOL isadd = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "isadd";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = true;
      $descriptor->addField($f);

      // OPTIONAL UINT64 userid = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "userid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL MESSAGE bornpos = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "bornpos";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ScenePos';
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
     * @return \RO\Cmd\GingerBreadNpcCmd
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
     * @return \RO\Cmd\GingerBreadNpcCmd
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
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\MapParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\MapParam $value
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <data> has a value
     *
     * @return boolean
     */
    public function hasData(){
      return $this->_has(3);
    }
    
    /**
     * Clear <data> value
     *
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function clearData(){
      return $this->_clear(3);
    }
    
    /**
     * Get <data> value
     *
     * @return \RO\Cmd\GingerBreadNpcData
     */
    public function getData(){
      return $this->_get(3);
    }
    
    /**
     * Set <data> value
     *
     * @param \RO\Cmd\GingerBreadNpcData $value
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function setData(\RO\Cmd\GingerBreadNpcData $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <isadd> has a value
     *
     * @return boolean
     */
    public function hasIsadd(){
      return $this->_has(4);
    }
    
    /**
     * Clear <isadd> value
     *
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function clearIsadd(){
      return $this->_clear(4);
    }
    
    /**
     * Get <isadd> value
     *
     * @return boolean
     */
    public function getIsadd(){
      return $this->_get(4);
    }
    
    /**
     * Set <isadd> value
     *
     * @param boolean $value
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function setIsadd( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <userid> has a value
     *
     * @return boolean
     */
    public function hasUserid(){
      return $this->_has(5);
    }
    
    /**
     * Clear <userid> value
     *
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function clearUserid(){
      return $this->_clear(5);
    }
    
    /**
     * Get <userid> value
     *
     * @return int
     */
    public function getUserid(){
      return $this->_get(5);
    }
    
    /**
     * Set <userid> value
     *
     * @param int $value
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function setUserid( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <bornpos> has a value
     *
     * @return boolean
     */
    public function hasBornpos(){
      return $this->_has(6);
    }
    
    /**
     * Clear <bornpos> value
     *
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function clearBornpos(){
      return $this->_clear(6);
    }
    
    /**
     * Get <bornpos> value
     *
     * @return \RO\Cmd\ScenePos
     */
    public function getBornpos(){
      return $this->_get(6);
    }
    
    /**
     * Set <bornpos> value
     *
     * @param \RO\Cmd\ScenePos $value
     * @return \RO\Cmd\GingerBreadNpcCmd
     */
    public function setBornpos(\RO\Cmd\ScenePos $value){
      return $this->_set(6, $value);
    }
  }
}

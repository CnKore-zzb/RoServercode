<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneUser2.proto

namespace RO\Cmd {

  class DressUpStageUserCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER2_PROTOCMD;
    
    /**  @var int - \RO\Cmd\User2Param */
    public $param = \RO\Cmd\User2Param::USER2PARAM_DRESSUP_STAGE;
    
    /**  @var int[]  */
    public $userid = array();
    
    /**  @var int */
    public $stageid = 0;
    
    /**  @var \RO\Cmd\StageUserDataType[]  */
    public $datas = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.DressUpStageUserCmd');

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
      $f->default   = \RO\Cmd\User2Param::USER2PARAM_DRESSUP_STAGE;
      $descriptor->addField($f);

      // REPEATED UINT64 userid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "userid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      // OPTIONAL UINT32 stageid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "stageid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // REPEATED MESSAGE datas = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "datas";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\StageUserDataType';
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
     * @return \RO\Cmd\DressUpStageUserCmd
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
     * @return \RO\Cmd\DressUpStageUserCmd
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
     * @return \RO\Cmd\DressUpStageUserCmd
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
     * @return \RO\Cmd\DressUpStageUserCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <userid> has a value
     *
     * @return boolean
     */
    public function hasUserid(){
      return $this->_has(3);
    }
    
    /**
     * Clear <userid> value
     *
     * @return \RO\Cmd\DressUpStageUserCmd
     */
    public function clearUserid(){
      return $this->_clear(3);
    }
    
    /**
     * Get <userid> value
     *
     * @param int $idx
     * @return int
     */
    public function getUserid($idx = NULL){
      return $this->_get(3, $idx);
    }
    
    /**
     * Set <userid> value
     *
     * @param int $value
     * @return \RO\Cmd\DressUpStageUserCmd
     */
    public function setUserid( $value, $idx = NULL){
      return $this->_set(3, $value, $idx);
    }
    
    /**
     * Get all elements of <userid>
     *
     * @return int[]
     */
    public function getUseridList(){
     return $this->_get(3);
    }
    
    /**
     * Add a new element to <userid>
     *
     * @param int $value
     * @return \RO\Cmd\DressUpStageUserCmd
     */
    public function addUserid( $value){
     return $this->_add(3, $value);
    }
    
    /**
     * Check if <stageid> has a value
     *
     * @return boolean
     */
    public function hasStageid(){
      return $this->_has(4);
    }
    
    /**
     * Clear <stageid> value
     *
     * @return \RO\Cmd\DressUpStageUserCmd
     */
    public function clearStageid(){
      return $this->_clear(4);
    }
    
    /**
     * Get <stageid> value
     *
     * @return int
     */
    public function getStageid(){
      return $this->_get(4);
    }
    
    /**
     * Set <stageid> value
     *
     * @param int $value
     * @return \RO\Cmd\DressUpStageUserCmd
     */
    public function setStageid( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <datas> has a value
     *
     * @return boolean
     */
    public function hasDatas(){
      return $this->_has(5);
    }
    
    /**
     * Clear <datas> value
     *
     * @return \RO\Cmd\DressUpStageUserCmd
     */
    public function clearDatas(){
      return $this->_clear(5);
    }
    
    /**
     * Get <datas> value
     *
     * @param int $idx
     * @return \RO\Cmd\StageUserDataType
     */
    public function getDatas($idx = NULL){
      return $this->_get(5, $idx);
    }
    
    /**
     * Set <datas> value
     *
     * @param \RO\Cmd\StageUserDataType $value
     * @return \RO\Cmd\DressUpStageUserCmd
     */
    public function setDatas(\RO\Cmd\StageUserDataType $value, $idx = NULL){
      return $this->_set(5, $value, $idx);
    }
    
    /**
     * Get all elements of <datas>
     *
     * @return \RO\Cmd\StageUserDataType[]
     */
    public function getDatasList(){
     return $this->_get(5);
    }
    
    /**
     * Add a new element to <datas>
     *
     * @param \RO\Cmd\StageUserDataType $value
     * @return \RO\Cmd\DressUpStageUserCmd
     */
    public function addDatas(\RO\Cmd\StageUserDataType $value){
     return $this->_add(5, $value);
    }
  }
}


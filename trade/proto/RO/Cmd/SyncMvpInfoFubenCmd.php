<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: FuBenCmd.proto

namespace RO\Cmd {

  class SyncMvpInfoFubenCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::FUBEN_PROTOCMD;
    
    /**  @var int - \RO\Cmd\FuBenParam */
    public $param = \RO\Cmd\FuBenParam::MVPBATTLE_SYNC_MVPINFO;
    
    /**  @var int */
    public $usernum = 0;
    
    /**  @var int[]  */
    public $liveboss = array();
    
    /**  @var int[]  */
    public $dieboss = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.SyncMvpInfoFubenCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::FUBEN_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\FuBenParam';
      $f->default   = \RO\Cmd\FuBenParam::MVPBATTLE_SYNC_MVPINFO;
      $descriptor->addField($f);

      // OPTIONAL UINT32 usernum = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "usernum";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // REPEATED UINT32 liveboss = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "liveboss";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      // REPEATED UINT32 dieboss = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "dieboss";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
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
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
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
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
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
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\FuBenParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\FuBenParam $value
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <usernum> has a value
     *
     * @return boolean
     */
    public function hasUsernum(){
      return $this->_has(3);
    }
    
    /**
     * Clear <usernum> value
     *
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function clearUsernum(){
      return $this->_clear(3);
    }
    
    /**
     * Get <usernum> value
     *
     * @return int
     */
    public function getUsernum(){
      return $this->_get(3);
    }
    
    /**
     * Set <usernum> value
     *
     * @param int $value
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function setUsernum( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <liveboss> has a value
     *
     * @return boolean
     */
    public function hasLiveboss(){
      return $this->_has(4);
    }
    
    /**
     * Clear <liveboss> value
     *
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function clearLiveboss(){
      return $this->_clear(4);
    }
    
    /**
     * Get <liveboss> value
     *
     * @param int $idx
     * @return int
     */
    public function getLiveboss($idx = NULL){
      return $this->_get(4, $idx);
    }
    
    /**
     * Set <liveboss> value
     *
     * @param int $value
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function setLiveboss( $value, $idx = NULL){
      return $this->_set(4, $value, $idx);
    }
    
    /**
     * Get all elements of <liveboss>
     *
     * @return int[]
     */
    public function getLivebossList(){
     return $this->_get(4);
    }
    
    /**
     * Add a new element to <liveboss>
     *
     * @param int $value
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function addLiveboss( $value){
     return $this->_add(4, $value);
    }
    
    /**
     * Check if <dieboss> has a value
     *
     * @return boolean
     */
    public function hasDieboss(){
      return $this->_has(5);
    }
    
    /**
     * Clear <dieboss> value
     *
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function clearDieboss(){
      return $this->_clear(5);
    }
    
    /**
     * Get <dieboss> value
     *
     * @param int $idx
     * @return int
     */
    public function getDieboss($idx = NULL){
      return $this->_get(5, $idx);
    }
    
    /**
     * Set <dieboss> value
     *
     * @param int $value
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function setDieboss( $value, $idx = NULL){
      return $this->_set(5, $value, $idx);
    }
    
    /**
     * Get all elements of <dieboss>
     *
     * @return int[]
     */
    public function getDiebossList(){
     return $this->_get(5);
    }
    
    /**
     * Add a new element to <dieboss>
     *
     * @param int $value
     * @return \RO\Cmd\SyncMvpInfoFubenCmd
     */
    public function addDieboss( $value){
     return $this->_add(5, $value);
    }
  }
}

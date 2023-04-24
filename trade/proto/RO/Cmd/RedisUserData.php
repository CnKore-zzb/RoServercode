<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordCmd.proto

namespace RO\Cmd {

  class RedisUserData extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $portrait = 0;
    
    /**  @var int */
    public $clothcolor = 0;
    
    /**  @var int */
    public $manuallv = 0;
    
    /**  @var int */
    public $manualexp = 0;
    
    /**  @var int */
    public $querytype = 0;
    
    /**  @var int */
    public $profic = 0;
    
    /**  @var boolean */
    public $blink = false;
    
    /**  @var boolean */
    public $canbetutor = false;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.RedisUserData');

      // OPTIONAL UINT32 portrait = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "portrait";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 clothcolor = 13
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 13;
      $f->name      = "clothcolor";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 manuallv = 20
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 20;
      $f->name      = "manuallv";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 manualexp = 21
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 21;
      $f->name      = "manualexp";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 querytype = 23
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 23;
      $f->name      = "querytype";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 profic = 24
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 24;
      $f->name      = "profic";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL BOOL blink = 26
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 26;
      $f->name      = "blink";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = false;
      $descriptor->addField($f);

      // OPTIONAL BOOL canbetutor = 27
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 27;
      $f->name      = "canbetutor";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = false;
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <portrait> has a value
     *
     * @return boolean
     */
    public function hasPortrait(){
      return $this->_has(5);
    }
    
    /**
     * Clear <portrait> value
     *
     * @return \RO\Cmd\RedisUserData
     */
    public function clearPortrait(){
      return $this->_clear(5);
    }
    
    /**
     * Get <portrait> value
     *
     * @return int
     */
    public function getPortrait(){
      return $this->_get(5);
    }
    
    /**
     * Set <portrait> value
     *
     * @param int $value
     * @return \RO\Cmd\RedisUserData
     */
    public function setPortrait( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <clothcolor> has a value
     *
     * @return boolean
     */
    public function hasClothcolor(){
      return $this->_has(13);
    }
    
    /**
     * Clear <clothcolor> value
     *
     * @return \RO\Cmd\RedisUserData
     */
    public function clearClothcolor(){
      return $this->_clear(13);
    }
    
    /**
     * Get <clothcolor> value
     *
     * @return int
     */
    public function getClothcolor(){
      return $this->_get(13);
    }
    
    /**
     * Set <clothcolor> value
     *
     * @param int $value
     * @return \RO\Cmd\RedisUserData
     */
    public function setClothcolor( $value){
      return $this->_set(13, $value);
    }
    
    /**
     * Check if <manuallv> has a value
     *
     * @return boolean
     */
    public function hasManuallv(){
      return $this->_has(20);
    }
    
    /**
     * Clear <manuallv> value
     *
     * @return \RO\Cmd\RedisUserData
     */
    public function clearManuallv(){
      return $this->_clear(20);
    }
    
    /**
     * Get <manuallv> value
     *
     * @return int
     */
    public function getManuallv(){
      return $this->_get(20);
    }
    
    /**
     * Set <manuallv> value
     *
     * @param int $value
     * @return \RO\Cmd\RedisUserData
     */
    public function setManuallv( $value){
      return $this->_set(20, $value);
    }
    
    /**
     * Check if <manualexp> has a value
     *
     * @return boolean
     */
    public function hasManualexp(){
      return $this->_has(21);
    }
    
    /**
     * Clear <manualexp> value
     *
     * @return \RO\Cmd\RedisUserData
     */
    public function clearManualexp(){
      return $this->_clear(21);
    }
    
    /**
     * Get <manualexp> value
     *
     * @return int
     */
    public function getManualexp(){
      return $this->_get(21);
    }
    
    /**
     * Set <manualexp> value
     *
     * @param int $value
     * @return \RO\Cmd\RedisUserData
     */
    public function setManualexp( $value){
      return $this->_set(21, $value);
    }
    
    /**
     * Check if <querytype> has a value
     *
     * @return boolean
     */
    public function hasQuerytype(){
      return $this->_has(23);
    }
    
    /**
     * Clear <querytype> value
     *
     * @return \RO\Cmd\RedisUserData
     */
    public function clearQuerytype(){
      return $this->_clear(23);
    }
    
    /**
     * Get <querytype> value
     *
     * @return int
     */
    public function getQuerytype(){
      return $this->_get(23);
    }
    
    /**
     * Set <querytype> value
     *
     * @param int $value
     * @return \RO\Cmd\RedisUserData
     */
    public function setQuerytype( $value){
      return $this->_set(23, $value);
    }
    
    /**
     * Check if <profic> has a value
     *
     * @return boolean
     */
    public function hasProfic(){
      return $this->_has(24);
    }
    
    /**
     * Clear <profic> value
     *
     * @return \RO\Cmd\RedisUserData
     */
    public function clearProfic(){
      return $this->_clear(24);
    }
    
    /**
     * Get <profic> value
     *
     * @return int
     */
    public function getProfic(){
      return $this->_get(24);
    }
    
    /**
     * Set <profic> value
     *
     * @param int $value
     * @return \RO\Cmd\RedisUserData
     */
    public function setProfic( $value){
      return $this->_set(24, $value);
    }
    
    /**
     * Check if <blink> has a value
     *
     * @return boolean
     */
    public function hasBlink(){
      return $this->_has(26);
    }
    
    /**
     * Clear <blink> value
     *
     * @return \RO\Cmd\RedisUserData
     */
    public function clearBlink(){
      return $this->_clear(26);
    }
    
    /**
     * Get <blink> value
     *
     * @return boolean
     */
    public function getBlink(){
      return $this->_get(26);
    }
    
    /**
     * Set <blink> value
     *
     * @param boolean $value
     * @return \RO\Cmd\RedisUserData
     */
    public function setBlink( $value){
      return $this->_set(26, $value);
    }
    
    /**
     * Check if <canbetutor> has a value
     *
     * @return boolean
     */
    public function hasCanbetutor(){
      return $this->_has(27);
    }
    
    /**
     * Clear <canbetutor> value
     *
     * @return \RO\Cmd\RedisUserData
     */
    public function clearCanbetutor(){
      return $this->_clear(27);
    }
    
    /**
     * Get <canbetutor> value
     *
     * @return boolean
     */
    public function getCanbetutor(){
      return $this->_get(27);
    }
    
    /**
     * Set <canbetutor> value
     *
     * @param boolean $value
     * @return \RO\Cmd\RedisUserData
     */
    public function setCanbetutor( $value){
      return $this->_set(27, $value);
    }
  }
}

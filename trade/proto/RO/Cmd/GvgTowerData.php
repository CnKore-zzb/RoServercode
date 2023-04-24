<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: FuBenCmd.proto

namespace RO\Cmd {

  class GvgTowerData extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\EGvgTowerType */
    public $etype = null;
    
    /**  @var int - \RO\Cmd\EGvgTowerState */
    public $estate = null;
    
    /**  @var int */
    public $owner_guild = null;
    
    /**  @var \RO\Cmd\GvgTowerValue[]  */
    public $info = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.GvgTowerData');

      // OPTIONAL ENUM etype = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "etype";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EGvgTowerType';
      $descriptor->addField($f);

      // OPTIONAL ENUM estate = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "estate";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EGvgTowerState';
      $descriptor->addField($f);

      // OPTIONAL UINT64 owner_guild = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "owner_guild";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // REPEATED MESSAGE info = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "info";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\GvgTowerValue';
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <etype> has a value
     *
     * @return boolean
     */
    public function hasEtype(){
      return $this->_has(1);
    }
    
    /**
     * Clear <etype> value
     *
     * @return \RO\Cmd\GvgTowerData
     */
    public function clearEtype(){
      return $this->_clear(1);
    }
    
    /**
     * Get <etype> value
     *
     * @return int - \RO\Cmd\EGvgTowerType
     */
    public function getEtype(){
      return $this->_get(1);
    }
    
    /**
     * Set <etype> value
     *
     * @param int - \RO\Cmd\EGvgTowerType $value
     * @return \RO\Cmd\GvgTowerData
     */
    public function setEtype( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <estate> has a value
     *
     * @return boolean
     */
    public function hasEstate(){
      return $this->_has(2);
    }
    
    /**
     * Clear <estate> value
     *
     * @return \RO\Cmd\GvgTowerData
     */
    public function clearEstate(){
      return $this->_clear(2);
    }
    
    /**
     * Get <estate> value
     *
     * @return int - \RO\Cmd\EGvgTowerState
     */
    public function getEstate(){
      return $this->_get(2);
    }
    
    /**
     * Set <estate> value
     *
     * @param int - \RO\Cmd\EGvgTowerState $value
     * @return \RO\Cmd\GvgTowerData
     */
    public function setEstate( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <owner_guild> has a value
     *
     * @return boolean
     */
    public function hasOwnerGuild(){
      return $this->_has(3);
    }
    
    /**
     * Clear <owner_guild> value
     *
     * @return \RO\Cmd\GvgTowerData
     */
    public function clearOwnerGuild(){
      return $this->_clear(3);
    }
    
    /**
     * Get <owner_guild> value
     *
     * @return int
     */
    public function getOwnerGuild(){
      return $this->_get(3);
    }
    
    /**
     * Set <owner_guild> value
     *
     * @param int $value
     * @return \RO\Cmd\GvgTowerData
     */
    public function setOwnerGuild( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <info> has a value
     *
     * @return boolean
     */
    public function hasInfo(){
      return $this->_has(4);
    }
    
    /**
     * Clear <info> value
     *
     * @return \RO\Cmd\GvgTowerData
     */
    public function clearInfo(){
      return $this->_clear(4);
    }
    
    /**
     * Get <info> value
     *
     * @param int $idx
     * @return \RO\Cmd\GvgTowerValue
     */
    public function getInfo($idx = NULL){
      return $this->_get(4, $idx);
    }
    
    /**
     * Set <info> value
     *
     * @param \RO\Cmd\GvgTowerValue $value
     * @return \RO\Cmd\GvgTowerData
     */
    public function setInfo(\RO\Cmd\GvgTowerValue $value, $idx = NULL){
      return $this->_set(4, $value, $idx);
    }
    
    /**
     * Get all elements of <info>
     *
     * @return \RO\Cmd\GvgTowerValue[]
     */
    public function getInfoList(){
     return $this->_get(4);
    }
    
    /**
     * Add a new element to <info>
     *
     * @param \RO\Cmd\GvgTowerValue $value
     * @return \RO\Cmd\GvgTowerData
     */
    public function addInfo(\RO\Cmd\GvgTowerValue $value){
     return $this->_add(4, $value);
    }
  }
}

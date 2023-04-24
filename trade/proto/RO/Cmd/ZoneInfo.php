<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneUser2.proto

namespace RO\Cmd {

  class ZoneInfo extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $zoneid = 0;
    
    /**  @var int */
    public $maxbaselv = 0;
    
    /**  @var int - \RO\Cmd\EZoneStatus */
    public $status = \RO\Cmd\EZoneStatus::EZONESTATUS_MIN;
    
    /**  @var int - \RO\Cmd\EZoneState */
    public $state = \RO\Cmd\EZoneState::EZONESTATE_MIN;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.ZoneInfo');

      // OPTIONAL UINT32 zoneid = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "zoneid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 maxbaselv = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "maxbaselv";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL ENUM status = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "status";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EZoneStatus';
      $f->default   = \RO\Cmd\EZoneStatus::EZONESTATUS_MIN;
      $descriptor->addField($f);

      // OPTIONAL ENUM state = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "state";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EZoneState';
      $f->default   = \RO\Cmd\EZoneState::EZONESTATE_MIN;
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <zoneid> has a value
     *
     * @return boolean
     */
    public function hasZoneid(){
      return $this->_has(1);
    }
    
    /**
     * Clear <zoneid> value
     *
     * @return \RO\Cmd\ZoneInfo
     */
    public function clearZoneid(){
      return $this->_clear(1);
    }
    
    /**
     * Get <zoneid> value
     *
     * @return int
     */
    public function getZoneid(){
      return $this->_get(1);
    }
    
    /**
     * Set <zoneid> value
     *
     * @param int $value
     * @return \RO\Cmd\ZoneInfo
     */
    public function setZoneid( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <maxbaselv> has a value
     *
     * @return boolean
     */
    public function hasMaxbaselv(){
      return $this->_has(2);
    }
    
    /**
     * Clear <maxbaselv> value
     *
     * @return \RO\Cmd\ZoneInfo
     */
    public function clearMaxbaselv(){
      return $this->_clear(2);
    }
    
    /**
     * Get <maxbaselv> value
     *
     * @return int
     */
    public function getMaxbaselv(){
      return $this->_get(2);
    }
    
    /**
     * Set <maxbaselv> value
     *
     * @param int $value
     * @return \RO\Cmd\ZoneInfo
     */
    public function setMaxbaselv( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <status> has a value
     *
     * @return boolean
     */
    public function hasStatus(){
      return $this->_has(3);
    }
    
    /**
     * Clear <status> value
     *
     * @return \RO\Cmd\ZoneInfo
     */
    public function clearStatus(){
      return $this->_clear(3);
    }
    
    /**
     * Get <status> value
     *
     * @return int - \RO\Cmd\EZoneStatus
     */
    public function getStatus(){
      return $this->_get(3);
    }
    
    /**
     * Set <status> value
     *
     * @param int - \RO\Cmd\EZoneStatus $value
     * @return \RO\Cmd\ZoneInfo
     */
    public function setStatus( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <state> has a value
     *
     * @return boolean
     */
    public function hasState(){
      return $this->_has(4);
    }
    
    /**
     * Clear <state> value
     *
     * @return \RO\Cmd\ZoneInfo
     */
    public function clearState(){
      return $this->_clear(4);
    }
    
    /**
     * Get <state> value
     *
     * @return int - \RO\Cmd\EZoneState
     */
    public function getState(){
      return $this->_get(4);
    }
    
    /**
     * Set <state> value
     *
     * @param int - \RO\Cmd\EZoneState $value
     * @return \RO\Cmd\ZoneInfo
     */
    public function setState( $value){
      return $this->_set(4, $value);
    }
  }
}


<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: ActivityEvent.proto

namespace RO\Cmd {

  class AERewardInfo extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\EAERewardMode */
    public $mode = \RO\Cmd\EAERewardMode::EAEREWARDMODE_MIN;
    
    /**  @var \RO\Cmd\AERewardExtraInfo */
    public $extrareward = null;
    
    /**  @var \RO\Cmd\AERewardMultipleInfo */
    public $multiplereward = null;
    
    /**  @var int */
    public $extratimes = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.AERewardInfo');

      // OPTIONAL ENUM mode = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "mode";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EAERewardMode';
      $f->default   = \RO\Cmd\EAERewardMode::EAEREWARDMODE_MIN;
      $descriptor->addField($f);

      // OPTIONAL MESSAGE extrareward = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "extrareward";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\AERewardExtraInfo';
      $descriptor->addField($f);

      // OPTIONAL MESSAGE multiplereward = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "multiplereward";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\AERewardMultipleInfo';
      $descriptor->addField($f);

      // OPTIONAL UINT32 extratimes = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "extratimes";
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
     * Check if <mode> has a value
     *
     * @return boolean
     */
    public function hasMode(){
      return $this->_has(1);
    }
    
    /**
     * Clear <mode> value
     *
     * @return \RO\Cmd\AERewardInfo
     */
    public function clearMode(){
      return $this->_clear(1);
    }
    
    /**
     * Get <mode> value
     *
     * @return int - \RO\Cmd\EAERewardMode
     */
    public function getMode(){
      return $this->_get(1);
    }
    
    /**
     * Set <mode> value
     *
     * @param int - \RO\Cmd\EAERewardMode $value
     * @return \RO\Cmd\AERewardInfo
     */
    public function setMode( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <extrareward> has a value
     *
     * @return boolean
     */
    public function hasExtrareward(){
      return $this->_has(2);
    }
    
    /**
     * Clear <extrareward> value
     *
     * @return \RO\Cmd\AERewardInfo
     */
    public function clearExtrareward(){
      return $this->_clear(2);
    }
    
    /**
     * Get <extrareward> value
     *
     * @return \RO\Cmd\AERewardExtraInfo
     */
    public function getExtrareward(){
      return $this->_get(2);
    }
    
    /**
     * Set <extrareward> value
     *
     * @param \RO\Cmd\AERewardExtraInfo $value
     * @return \RO\Cmd\AERewardInfo
     */
    public function setExtrareward(\RO\Cmd\AERewardExtraInfo $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <multiplereward> has a value
     *
     * @return boolean
     */
    public function hasMultiplereward(){
      return $this->_has(3);
    }
    
    /**
     * Clear <multiplereward> value
     *
     * @return \RO\Cmd\AERewardInfo
     */
    public function clearMultiplereward(){
      return $this->_clear(3);
    }
    
    /**
     * Get <multiplereward> value
     *
     * @return \RO\Cmd\AERewardMultipleInfo
     */
    public function getMultiplereward(){
      return $this->_get(3);
    }
    
    /**
     * Set <multiplereward> value
     *
     * @param \RO\Cmd\AERewardMultipleInfo $value
     * @return \RO\Cmd\AERewardInfo
     */
    public function setMultiplereward(\RO\Cmd\AERewardMultipleInfo $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <extratimes> has a value
     *
     * @return boolean
     */
    public function hasExtratimes(){
      return $this->_has(4);
    }
    
    /**
     * Clear <extratimes> value
     *
     * @return \RO\Cmd\AERewardInfo
     */
    public function clearExtratimes(){
      return $this->_clear(4);
    }
    
    /**
     * Get <extratimes> value
     *
     * @return int
     */
    public function getExtratimes(){
      return $this->_get(4);
    }
    
    /**
     * Set <extratimes> value
     *
     * @param int $value
     * @return \RO\Cmd\AERewardInfo
     */
    public function setExtratimes( $value){
      return $this->_set(4, $value);
    }
  }
}


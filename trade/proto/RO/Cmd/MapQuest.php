<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordCmd.proto

namespace RO\Cmd {

  class MapQuest extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $mapid = 0;
    
    /**  @var int[]  */
    public $questids = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.MapQuest');

      // OPTIONAL UINT32 mapid = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "mapid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // REPEATED UINT32 questids = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "questids";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <mapid> has a value
     *
     * @return boolean
     */
    public function hasMapid(){
      return $this->_has(1);
    }
    
    /**
     * Clear <mapid> value
     *
     * @return \RO\Cmd\MapQuest
     */
    public function clearMapid(){
      return $this->_clear(1);
    }
    
    /**
     * Get <mapid> value
     *
     * @return int
     */
    public function getMapid(){
      return $this->_get(1);
    }
    
    /**
     * Set <mapid> value
     *
     * @param int $value
     * @return \RO\Cmd\MapQuest
     */
    public function setMapid( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <questids> has a value
     *
     * @return boolean
     */
    public function hasQuestids(){
      return $this->_has(2);
    }
    
    /**
     * Clear <questids> value
     *
     * @return \RO\Cmd\MapQuest
     */
    public function clearQuestids(){
      return $this->_clear(2);
    }
    
    /**
     * Get <questids> value
     *
     * @param int $idx
     * @return int
     */
    public function getQuestids($idx = NULL){
      return $this->_get(2, $idx);
    }
    
    /**
     * Set <questids> value
     *
     * @param int $value
     * @return \RO\Cmd\MapQuest
     */
    public function setQuestids( $value, $idx = NULL){
      return $this->_set(2, $value, $idx);
    }
    
    /**
     * Get all elements of <questids>
     *
     * @return int[]
     */
    public function getQuestidsList(){
     return $this->_get(2);
    }
    
    /**
     * Add a new element to <questids>
     *
     * @param int $value
     * @return \RO\Cmd\MapQuest
     */
    public function addQuestids( $value){
     return $this->_add(2, $value);
    }
  }
}


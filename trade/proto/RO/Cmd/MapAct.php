<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneMap.proto

namespace RO\Cmd {

  class MapAct extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $id = 0;
    
    /**  @var int */
    public $range = 0;
    
    /**  @var int */
    public $masterid = 0;
    
    /**  @var int - \RO\Cmd\EActType */
    public $type = \RO\Cmd\EActType::EACTTYPE_MIN;
    
    /**  @var int */
    public $actvalue = 0;
    
    /**  @var \RO\Cmd\ScenePos */
    public $pos = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.MapAct');

      // OPTIONAL UINT64 id = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "id";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 range = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "range";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT64 masterid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "masterid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL ENUM type = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "type";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EActType';
      $f->default   = \RO\Cmd\EActType::EACTTYPE_MIN;
      $descriptor->addField($f);

      // OPTIONAL UINT32 actvalue = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "actvalue";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL MESSAGE pos = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "pos";
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
     * Check if <id> has a value
     *
     * @return boolean
     */
    public function hasId(){
      return $this->_has(1);
    }
    
    /**
     * Clear <id> value
     *
     * @return \RO\Cmd\MapAct
     */
    public function clearId(){
      return $this->_clear(1);
    }
    
    /**
     * Get <id> value
     *
     * @return int
     */
    public function getId(){
      return $this->_get(1);
    }
    
    /**
     * Set <id> value
     *
     * @param int $value
     * @return \RO\Cmd\MapAct
     */
    public function setId( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <range> has a value
     *
     * @return boolean
     */
    public function hasRange(){
      return $this->_has(2);
    }
    
    /**
     * Clear <range> value
     *
     * @return \RO\Cmd\MapAct
     */
    public function clearRange(){
      return $this->_clear(2);
    }
    
    /**
     * Get <range> value
     *
     * @return int
     */
    public function getRange(){
      return $this->_get(2);
    }
    
    /**
     * Set <range> value
     *
     * @param int $value
     * @return \RO\Cmd\MapAct
     */
    public function setRange( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <masterid> has a value
     *
     * @return boolean
     */
    public function hasMasterid(){
      return $this->_has(3);
    }
    
    /**
     * Clear <masterid> value
     *
     * @return \RO\Cmd\MapAct
     */
    public function clearMasterid(){
      return $this->_clear(3);
    }
    
    /**
     * Get <masterid> value
     *
     * @return int
     */
    public function getMasterid(){
      return $this->_get(3);
    }
    
    /**
     * Set <masterid> value
     *
     * @param int $value
     * @return \RO\Cmd\MapAct
     */
    public function setMasterid( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <type> has a value
     *
     * @return boolean
     */
    public function hasType(){
      return $this->_has(4);
    }
    
    /**
     * Clear <type> value
     *
     * @return \RO\Cmd\MapAct
     */
    public function clearType(){
      return $this->_clear(4);
    }
    
    /**
     * Get <type> value
     *
     * @return int - \RO\Cmd\EActType
     */
    public function getType(){
      return $this->_get(4);
    }
    
    /**
     * Set <type> value
     *
     * @param int - \RO\Cmd\EActType $value
     * @return \RO\Cmd\MapAct
     */
    public function setType( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <actvalue> has a value
     *
     * @return boolean
     */
    public function hasActvalue(){
      return $this->_has(5);
    }
    
    /**
     * Clear <actvalue> value
     *
     * @return \RO\Cmd\MapAct
     */
    public function clearActvalue(){
      return $this->_clear(5);
    }
    
    /**
     * Get <actvalue> value
     *
     * @return int
     */
    public function getActvalue(){
      return $this->_get(5);
    }
    
    /**
     * Set <actvalue> value
     *
     * @param int $value
     * @return \RO\Cmd\MapAct
     */
    public function setActvalue( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <pos> has a value
     *
     * @return boolean
     */
    public function hasPos(){
      return $this->_has(6);
    }
    
    /**
     * Clear <pos> value
     *
     * @return \RO\Cmd\MapAct
     */
    public function clearPos(){
      return $this->_clear(6);
    }
    
    /**
     * Get <pos> value
     *
     * @return \RO\Cmd\ScenePos
     */
    public function getPos(){
      return $this->_get(6);
    }
    
    /**
     * Set <pos> value
     *
     * @param \RO\Cmd\ScenePos $value
     * @return \RO\Cmd\MapAct
     */
    public function setPos(\RO\Cmd\ScenePos $value){
      return $this->_set(6, $value);
    }
  }
}


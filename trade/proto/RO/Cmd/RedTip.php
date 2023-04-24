<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneTip.proto

namespace RO\Cmd {

  class RedTip extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\ERedSys */
    public $redsys = \RO\Cmd\ERedSys::EREDSYS_MIN;
    
    /**  @var int - \RO\Cmd\ETipItemOpt */
    public $optItem = \RO\Cmd\ETipItemOpt::ETIPITEMOPT_ADD;
    
    /**  @var int[]  */
    public $tipid = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.RedTip');

      // OPTIONAL ENUM redsys = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "redsys";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ERedSys';
      $f->default   = \RO\Cmd\ERedSys::EREDSYS_MIN;
      $descriptor->addField($f);

      // OPTIONAL ENUM optItem = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "optItem";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ETipItemOpt';
      $f->default   = \RO\Cmd\ETipItemOpt::ETIPITEMOPT_ADD;
      $descriptor->addField($f);

      // REPEATED UINT64 tipid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "tipid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <redsys> has a value
     *
     * @return boolean
     */
    public function hasRedsys(){
      return $this->_has(1);
    }
    
    /**
     * Clear <redsys> value
     *
     * @return \RO\Cmd\RedTip
     */
    public function clearRedsys(){
      return $this->_clear(1);
    }
    
    /**
     * Get <redsys> value
     *
     * @return int - \RO\Cmd\ERedSys
     */
    public function getRedsys(){
      return $this->_get(1);
    }
    
    /**
     * Set <redsys> value
     *
     * @param int - \RO\Cmd\ERedSys $value
     * @return \RO\Cmd\RedTip
     */
    public function setRedsys( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <optItem> has a value
     *
     * @return boolean
     */
    public function hasOptItem(){
      return $this->_has(2);
    }
    
    /**
     * Clear <optItem> value
     *
     * @return \RO\Cmd\RedTip
     */
    public function clearOptItem(){
      return $this->_clear(2);
    }
    
    /**
     * Get <optItem> value
     *
     * @return int - \RO\Cmd\ETipItemOpt
     */
    public function getOptItem(){
      return $this->_get(2);
    }
    
    /**
     * Set <optItem> value
     *
     * @param int - \RO\Cmd\ETipItemOpt $value
     * @return \RO\Cmd\RedTip
     */
    public function setOptItem( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <tipid> has a value
     *
     * @return boolean
     */
    public function hasTipid(){
      return $this->_has(3);
    }
    
    /**
     * Clear <tipid> value
     *
     * @return \RO\Cmd\RedTip
     */
    public function clearTipid(){
      return $this->_clear(3);
    }
    
    /**
     * Get <tipid> value
     *
     * @param int $idx
     * @return int
     */
    public function getTipid($idx = NULL){
      return $this->_get(3, $idx);
    }
    
    /**
     * Set <tipid> value
     *
     * @param int $value
     * @return \RO\Cmd\RedTip
     */
    public function setTipid( $value, $idx = NULL){
      return $this->_set(3, $value, $idx);
    }
    
    /**
     * Get all elements of <tipid>
     *
     * @return int[]
     */
    public function getTipidList(){
     return $this->_get(3);
    }
    
    /**
     * Add a new element to <tipid>
     *
     * @param int $value
     * @return \RO\Cmd\RedTip
     */
    public function addTipid( $value){
     return $this->_add(3, $value);
    }
  }
}


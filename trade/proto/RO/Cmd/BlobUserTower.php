<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordCmd.proto

namespace RO\Cmd {

  class BlobUserTower extends \DrSlump\Protobuf\Message {

    /**  @var \RO\Cmd\UserTowerInfo */
    public $towerinfo = null;
    
    /**  @var int */
    public $cleartime = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.BlobUserTower');

      // OPTIONAL MESSAGE towerinfo = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "towerinfo";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\UserTowerInfo';
      $descriptor->addField($f);

      // OPTIONAL UINT32 cleartime = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "cleartime";
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
     * Check if <towerinfo> has a value
     *
     * @return boolean
     */
    public function hasTowerinfo(){
      return $this->_has(1);
    }
    
    /**
     * Clear <towerinfo> value
     *
     * @return \RO\Cmd\BlobUserTower
     */
    public function clearTowerinfo(){
      return $this->_clear(1);
    }
    
    /**
     * Get <towerinfo> value
     *
     * @return \RO\Cmd\UserTowerInfo
     */
    public function getTowerinfo(){
      return $this->_get(1);
    }
    
    /**
     * Set <towerinfo> value
     *
     * @param \RO\Cmd\UserTowerInfo $value
     * @return \RO\Cmd\BlobUserTower
     */
    public function setTowerinfo(\RO\Cmd\UserTowerInfo $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <cleartime> has a value
     *
     * @return boolean
     */
    public function hasCleartime(){
      return $this->_has(2);
    }
    
    /**
     * Clear <cleartime> value
     *
     * @return \RO\Cmd\BlobUserTower
     */
    public function clearCleartime(){
      return $this->_clear(2);
    }
    
    /**
     * Get <cleartime> value
     *
     * @return int
     */
    public function getCleartime(){
      return $this->_get(2);
    }
    
    /**
     * Set <cleartime> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobUserTower
     */
    public function setCleartime( $value){
      return $this->_set(2, $value);
    }
  }
}

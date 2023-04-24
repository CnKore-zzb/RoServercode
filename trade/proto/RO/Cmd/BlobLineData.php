<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordCmd.proto

namespace RO\Cmd {

  class BlobLineData extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $otherid = null;
    
    /**  @var int */
    public $expireTime = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.BlobLineData');

      // OPTIONAL UINT64 otherid = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "otherid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 expireTime = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "expireTime";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <otherid> has a value
     *
     * @return boolean
     */
    public function hasOtherid(){
      return $this->_has(1);
    }
    
    /**
     * Clear <otherid> value
     *
     * @return \RO\Cmd\BlobLineData
     */
    public function clearOtherid(){
      return $this->_clear(1);
    }
    
    /**
     * Get <otherid> value
     *
     * @return int
     */
    public function getOtherid(){
      return $this->_get(1);
    }
    
    /**
     * Set <otherid> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobLineData
     */
    public function setOtherid( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <expireTime> has a value
     *
     * @return boolean
     */
    public function hasExpireTime(){
      return $this->_has(2);
    }
    
    /**
     * Clear <expireTime> value
     *
     * @return \RO\Cmd\BlobLineData
     */
    public function clearExpireTime(){
      return $this->_clear(2);
    }
    
    /**
     * Get <expireTime> value
     *
     * @return int
     */
    public function getExpireTime(){
      return $this->_get(2);
    }
    
    /**
     * Set <expireTime> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobLineData
     */
    public function setExpireTime( $value){
      return $this->_set(2, $value);
    }
  }
}

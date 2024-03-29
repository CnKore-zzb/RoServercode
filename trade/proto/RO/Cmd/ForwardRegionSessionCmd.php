<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SessionCmd.proto

namespace RO\Cmd {

  class ForwardRegionSessionCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SESSION_PROTOCMD;
    
    /**  @var int - \RO\Cmd\SessionParam */
    public $param = \RO\Cmd\SessionParam::SESSIONPARAM_FORWARD_REGION;
    
    /**  @var int */
    public $region_type = null;
    
    /**  @var string */
    public $data = null;
    
    /**  @var int */
    public $len = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.ForwardRegionSessionCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SESSION_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\SessionParam';
      $f->default   = \RO\Cmd\SessionParam::SESSIONPARAM_FORWARD_REGION;
      $descriptor->addField($f);

      // OPTIONAL UINT32 region_type = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "region_type";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL BYTES data = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "data";
      $f->type      = \DrSlump\Protobuf::TYPE_BYTES;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 len = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "len";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
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
     * @return \RO\Cmd\ForwardRegionSessionCmd
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
     * @return \RO\Cmd\ForwardRegionSessionCmd
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
     * @return \RO\Cmd\ForwardRegionSessionCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\SessionParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\SessionParam $value
     * @return \RO\Cmd\ForwardRegionSessionCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <region_type> has a value
     *
     * @return boolean
     */
    public function hasRegionType(){
      return $this->_has(3);
    }
    
    /**
     * Clear <region_type> value
     *
     * @return \RO\Cmd\ForwardRegionSessionCmd
     */
    public function clearRegionType(){
      return $this->_clear(3);
    }
    
    /**
     * Get <region_type> value
     *
     * @return int
     */
    public function getRegionType(){
      return $this->_get(3);
    }
    
    /**
     * Set <region_type> value
     *
     * @param int $value
     * @return \RO\Cmd\ForwardRegionSessionCmd
     */
    public function setRegionType( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <data> has a value
     *
     * @return boolean
     */
    public function hasData(){
      return $this->_has(4);
    }
    
    /**
     * Clear <data> value
     *
     * @return \RO\Cmd\ForwardRegionSessionCmd
     */
    public function clearData(){
      return $this->_clear(4);
    }
    
    /**
     * Get <data> value
     *
     * @return string
     */
    public function getData(){
      return $this->_get(4);
    }
    
    /**
     * Set <data> value
     *
     * @param string $value
     * @return \RO\Cmd\ForwardRegionSessionCmd
     */
    public function setData( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <len> has a value
     *
     * @return boolean
     */
    public function hasLen(){
      return $this->_has(5);
    }
    
    /**
     * Clear <len> value
     *
     * @return \RO\Cmd\ForwardRegionSessionCmd
     */
    public function clearLen(){
      return $this->_clear(5);
    }
    
    /**
     * Get <len> value
     *
     * @return int
     */
    public function getLen(){
      return $this->_get(5);
    }
    
    /**
     * Set <len> value
     *
     * @param int $value
     * @return \RO\Cmd\ForwardRegionSessionCmd
     */
    public function setLen( $value){
      return $this->_set(5, $value);
    }
  }
}


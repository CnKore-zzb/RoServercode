<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: PhotoCmd.proto

namespace RO\Cmd {

  class QueryUserPhotoListPhotoCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER_PHOTO_PROTOCMD;
    
    /**  @var int - \RO\Cmd\PhotoParam */
    public $param = \RO\Cmd\PhotoParam::PHOTOPARAM_QUERY_USERPHOTOLIST;
    
    /**  @var \RO\Cmd\PhotoFrame[]  */
    public $frames = array();
    
    /**  @var int */
    public $maxphoto = 0;
    
    /**  @var int */
    public $maxframe = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.QueryUserPhotoListPhotoCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SCENE_USER_PHOTO_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\PhotoParam';
      $f->default   = \RO\Cmd\PhotoParam::PHOTOPARAM_QUERY_USERPHOTOLIST;
      $descriptor->addField($f);

      // REPEATED MESSAGE frames = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "frames";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\PhotoFrame';
      $descriptor->addField($f);

      // OPTIONAL UINT32 maxphoto = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "maxphoto";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 maxframe = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "maxframe";
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
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
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
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
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
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\PhotoParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\PhotoParam $value
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <frames> has a value
     *
     * @return boolean
     */
    public function hasFrames(){
      return $this->_has(3);
    }
    
    /**
     * Clear <frames> value
     *
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
     */
    public function clearFrames(){
      return $this->_clear(3);
    }
    
    /**
     * Get <frames> value
     *
     * @param int $idx
     * @return \RO\Cmd\PhotoFrame
     */
    public function getFrames($idx = NULL){
      return $this->_get(3, $idx);
    }
    
    /**
     * Set <frames> value
     *
     * @param \RO\Cmd\PhotoFrame $value
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
     */
    public function setFrames(\RO\Cmd\PhotoFrame $value, $idx = NULL){
      return $this->_set(3, $value, $idx);
    }
    
    /**
     * Get all elements of <frames>
     *
     * @return \RO\Cmd\PhotoFrame[]
     */
    public function getFramesList(){
     return $this->_get(3);
    }
    
    /**
     * Add a new element to <frames>
     *
     * @param \RO\Cmd\PhotoFrame $value
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
     */
    public function addFrames(\RO\Cmd\PhotoFrame $value){
     return $this->_add(3, $value);
    }
    
    /**
     * Check if <maxphoto> has a value
     *
     * @return boolean
     */
    public function hasMaxphoto(){
      return $this->_has(4);
    }
    
    /**
     * Clear <maxphoto> value
     *
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
     */
    public function clearMaxphoto(){
      return $this->_clear(4);
    }
    
    /**
     * Get <maxphoto> value
     *
     * @return int
     */
    public function getMaxphoto(){
      return $this->_get(4);
    }
    
    /**
     * Set <maxphoto> value
     *
     * @param int $value
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
     */
    public function setMaxphoto( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <maxframe> has a value
     *
     * @return boolean
     */
    public function hasMaxframe(){
      return $this->_has(5);
    }
    
    /**
     * Clear <maxframe> value
     *
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
     */
    public function clearMaxframe(){
      return $this->_clear(5);
    }
    
    /**
     * Get <maxframe> value
     *
     * @return int
     */
    public function getMaxframe(){
      return $this->_get(5);
    }
    
    /**
     * Set <maxframe> value
     *
     * @param int $value
     * @return \RO\Cmd\QueryUserPhotoListPhotoCmd
     */
    public function setMaxframe( $value){
      return $this->_set(5, $value);
    }
  }
}


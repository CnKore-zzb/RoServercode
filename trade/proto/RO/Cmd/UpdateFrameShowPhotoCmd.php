<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: PhotoCmd.proto

namespace RO\Cmd {

  class UpdateFrameShowPhotoCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER_PHOTO_PROTOCMD;
    
    /**  @var int - \RO\Cmd\PhotoParam */
    public $param = \RO\Cmd\PhotoParam::PHOTOPARAM_UPDATE_FRAMESHOW;
    
    /**  @var \RO\Cmd\FrameShow[]  */
    public $shows = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.UpdateFrameShowPhotoCmd');

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
      $f->default   = \RO\Cmd\PhotoParam::PHOTOPARAM_UPDATE_FRAMESHOW;
      $descriptor->addField($f);

      // REPEATED MESSAGE shows = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "shows";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\FrameShow';
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
     * @return \RO\Cmd\UpdateFrameShowPhotoCmd
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
     * @return \RO\Cmd\UpdateFrameShowPhotoCmd
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
     * @return \RO\Cmd\UpdateFrameShowPhotoCmd
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
     * @return \RO\Cmd\UpdateFrameShowPhotoCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <shows> has a value
     *
     * @return boolean
     */
    public function hasShows(){
      return $this->_has(3);
    }
    
    /**
     * Clear <shows> value
     *
     * @return \RO\Cmd\UpdateFrameShowPhotoCmd
     */
    public function clearShows(){
      return $this->_clear(3);
    }
    
    /**
     * Get <shows> value
     *
     * @param int $idx
     * @return \RO\Cmd\FrameShow
     */
    public function getShows($idx = NULL){
      return $this->_get(3, $idx);
    }
    
    /**
     * Set <shows> value
     *
     * @param \RO\Cmd\FrameShow $value
     * @return \RO\Cmd\UpdateFrameShowPhotoCmd
     */
    public function setShows(\RO\Cmd\FrameShow $value, $idx = NULL){
      return $this->_set(3, $value, $idx);
    }
    
    /**
     * Get all elements of <shows>
     *
     * @return \RO\Cmd\FrameShow[]
     */
    public function getShowsList(){
     return $this->_get(3);
    }
    
    /**
     * Add a new element to <shows>
     *
     * @param \RO\Cmd\FrameShow $value
     * @return \RO\Cmd\UpdateFrameShowPhotoCmd
     */
    public function addShows(\RO\Cmd\FrameShow $value){
     return $this->_add(3, $value);
    }
  }
}


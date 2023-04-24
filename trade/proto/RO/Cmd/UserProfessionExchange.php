<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneUser.proto

namespace RO\Cmd {

  class UserProfessionExchange extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER_PROTOCMD;
    
    /**  @var int */
    public $param = 4;
    
    /**  @var int - \RO\Cmd\EProfession */
    public $profession = \RO\Cmd\EProfession::EPROFESSION_MIN;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.UserProfessionExchange');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SCENE_USER_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL UINT32 param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 4;
      $descriptor->addField($f);

      // OPTIONAL ENUM profession = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "profession";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EProfession';
      $f->default   = \RO\Cmd\EProfession::EPROFESSION_MIN;
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
     * @return \RO\Cmd\UserProfessionExchange
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
     * @return \RO\Cmd\UserProfessionExchange
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
     * @return \RO\Cmd\UserProfessionExchange
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int $value
     * @return \RO\Cmd\UserProfessionExchange
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <profession> has a value
     *
     * @return boolean
     */
    public function hasProfession(){
      return $this->_has(3);
    }
    
    /**
     * Clear <profession> value
     *
     * @return \RO\Cmd\UserProfessionExchange
     */
    public function clearProfession(){
      return $this->_clear(3);
    }
    
    /**
     * Get <profession> value
     *
     * @return int - \RO\Cmd\EProfession
     */
    public function getProfession(){
      return $this->_get(3);
    }
    
    /**
     * Set <profession> value
     *
     * @param int - \RO\Cmd\EProfession $value
     * @return \RO\Cmd\UserProfessionExchange
     */
    public function setProfession( $value){
      return $this->_set(3, $value);
    }
  }
}

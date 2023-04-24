<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneChatRoom.proto

namespace RO\Cmd {

  class EnterChatRoom extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER_CHATROOM_PROTOCMD;
    
    /**  @var int - \RO\Cmd\EChatRoomParam */
    public $param = \RO\Cmd\EChatRoomParam::ECHATROOMPARAM_ENTERROOM;
    
    /**  @var \RO\Cmd\ChatRoomData */
    public $data = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.EnterChatRoom');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SCENE_USER_CHATROOM_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EChatRoomParam';
      $f->default   = \RO\Cmd\EChatRoomParam::ECHATROOMPARAM_ENTERROOM;
      $descriptor->addField($f);

      // OPTIONAL MESSAGE data = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "data";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ChatRoomData';
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
     * @return \RO\Cmd\EnterChatRoom
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
     * @return \RO\Cmd\EnterChatRoom
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
     * @return \RO\Cmd\EnterChatRoom
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\EChatRoomParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\EChatRoomParam $value
     * @return \RO\Cmd\EnterChatRoom
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <data> has a value
     *
     * @return boolean
     */
    public function hasData(){
      return $this->_has(3);
    }
    
    /**
     * Clear <data> value
     *
     * @return \RO\Cmd\EnterChatRoom
     */
    public function clearData(){
      return $this->_clear(3);
    }
    
    /**
     * Get <data> value
     *
     * @return \RO\Cmd\ChatRoomData
     */
    public function getData(){
      return $this->_get(3);
    }
    
    /**
     * Set <data> value
     *
     * @param \RO\Cmd\ChatRoomData $value
     * @return \RO\Cmd\EnterChatRoom
     */
    public function setData(\RO\Cmd\ChatRoomData $value){
      return $this->_set(3, $value);
    }
  }
}


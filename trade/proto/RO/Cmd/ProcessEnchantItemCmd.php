<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneItem.proto

namespace RO\Cmd {

  class ProcessEnchantItemCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER_ITEM_PROTOCMD;
    
    /**  @var int - \RO\Cmd\ItemParam */
    public $param = \RO\Cmd\ItemParam::ITEMPARAM_PROCESSENCHANT;
    
    /**  @var boolean */
    public $save = false;
    
    /**  @var string */
    public $itemid = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.ProcessEnchantItemCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SCENE_USER_ITEM_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ItemParam';
      $f->default   = \RO\Cmd\ItemParam::ITEMPARAM_PROCESSENCHANT;
      $descriptor->addField($f);

      // OPTIONAL BOOL save = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "save";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = false;
      $descriptor->addField($f);

      // OPTIONAL STRING itemid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "itemid";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
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
     * @return \RO\Cmd\ProcessEnchantItemCmd
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
     * @return \RO\Cmd\ProcessEnchantItemCmd
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
     * @return \RO\Cmd\ProcessEnchantItemCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\ItemParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\ItemParam $value
     * @return \RO\Cmd\ProcessEnchantItemCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <save> has a value
     *
     * @return boolean
     */
    public function hasSave(){
      return $this->_has(3);
    }
    
    /**
     * Clear <save> value
     *
     * @return \RO\Cmd\ProcessEnchantItemCmd
     */
    public function clearSave(){
      return $this->_clear(3);
    }
    
    /**
     * Get <save> value
     *
     * @return boolean
     */
    public function getSave(){
      return $this->_get(3);
    }
    
    /**
     * Set <save> value
     *
     * @param boolean $value
     * @return \RO\Cmd\ProcessEnchantItemCmd
     */
    public function setSave( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <itemid> has a value
     *
     * @return boolean
     */
    public function hasItemid(){
      return $this->_has(4);
    }
    
    /**
     * Clear <itemid> value
     *
     * @return \RO\Cmd\ProcessEnchantItemCmd
     */
    public function clearItemid(){
      return $this->_clear(4);
    }
    
    /**
     * Get <itemid> value
     *
     * @return string
     */
    public function getItemid(){
      return $this->_get(4);
    }
    
    /**
     * Set <itemid> value
     *
     * @param string $value
     * @return \RO\Cmd\ProcessEnchantItemCmd
     */
    public function setItemid( $value){
      return $this->_set(4, $value);
    }
  }
}


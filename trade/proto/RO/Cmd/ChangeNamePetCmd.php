<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: ScenePet.proto

namespace RO\Cmd {

  class ChangeNamePetCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER_PET_PROTOCMD;
    
    /**  @var int - \RO\Cmd\PetParam */
    public $param = \RO\Cmd\PetParam::PETPARAM_CHANGE_NAME;
    
    /**  @var int */
    public $petid = 0;
    
    /**  @var string */
    public $name = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.ChangeNamePetCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SCENE_USER_PET_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\PetParam';
      $f->default   = \RO\Cmd\PetParam::PETPARAM_CHANGE_NAME;
      $descriptor->addField($f);

      // OPTIONAL UINT32 petid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "petid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL STRING name = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "name";
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
     * @return \RO\Cmd\ChangeNamePetCmd
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
     * @return \RO\Cmd\ChangeNamePetCmd
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
     * @return \RO\Cmd\ChangeNamePetCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\PetParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\PetParam $value
     * @return \RO\Cmd\ChangeNamePetCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <petid> has a value
     *
     * @return boolean
     */
    public function hasPetid(){
      return $this->_has(3);
    }
    
    /**
     * Clear <petid> value
     *
     * @return \RO\Cmd\ChangeNamePetCmd
     */
    public function clearPetid(){
      return $this->_clear(3);
    }
    
    /**
     * Get <petid> value
     *
     * @return int
     */
    public function getPetid(){
      return $this->_get(3);
    }
    
    /**
     * Set <petid> value
     *
     * @param int $value
     * @return \RO\Cmd\ChangeNamePetCmd
     */
    public function setPetid( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <name> has a value
     *
     * @return boolean
     */
    public function hasName(){
      return $this->_has(4);
    }
    
    /**
     * Clear <name> value
     *
     * @return \RO\Cmd\ChangeNamePetCmd
     */
    public function clearName(){
      return $this->_clear(4);
    }
    
    /**
     * Get <name> value
     *
     * @return string
     */
    public function getName(){
      return $this->_get(4);
    }
    
    /**
     * Set <name> value
     *
     * @param string $value
     * @return \RO\Cmd\ChangeNamePetCmd
     */
    public function setName( $value){
      return $this->_set(4, $value);
    }
  }
}


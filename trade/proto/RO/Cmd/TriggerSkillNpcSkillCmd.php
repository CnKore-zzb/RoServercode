<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneSkill.proto

namespace RO\Cmd {

  class TriggerSkillNpcSkillCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER_SKILL_PROTOCMD;
    
    /**  @var int - \RO\Cmd\SkillParam */
    public $param = \RO\Cmd\SkillParam::SKILLPARAM_TRGGER_SKILLNPC;
    
    /**  @var int */
    public $npcguid = null;
    
    /**  @var int - \RO\Cmd\ETrigSkillType */
    public $etype = \RO\Cmd\ETrigSkillType::ETRIGTSKILL_MIN;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.TriggerSkillNpcSkillCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SCENE_USER_SKILL_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\SkillParam';
      $f->default   = \RO\Cmd\SkillParam::SKILLPARAM_TRGGER_SKILLNPC;
      $descriptor->addField($f);

      // REQUIRED UINT64 npcguid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "npcguid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_REQUIRED;
      $descriptor->addField($f);

      // OPTIONAL ENUM etype = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "etype";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ETrigSkillType';
      $f->default   = \RO\Cmd\ETrigSkillType::ETRIGTSKILL_MIN;
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
     * @return \RO\Cmd\TriggerSkillNpcSkillCmd
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
     * @return \RO\Cmd\TriggerSkillNpcSkillCmd
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
     * @return \RO\Cmd\TriggerSkillNpcSkillCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\SkillParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\SkillParam $value
     * @return \RO\Cmd\TriggerSkillNpcSkillCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <npcguid> has a value
     *
     * @return boolean
     */
    public function hasNpcguid(){
      return $this->_has(3);
    }
    
    /**
     * Clear <npcguid> value
     *
     * @return \RO\Cmd\TriggerSkillNpcSkillCmd
     */
    public function clearNpcguid(){
      return $this->_clear(3);
    }
    
    /**
     * Get <npcguid> value
     *
     * @return int
     */
    public function getNpcguid(){
      return $this->_get(3);
    }
    
    /**
     * Set <npcguid> value
     *
     * @param int $value
     * @return \RO\Cmd\TriggerSkillNpcSkillCmd
     */
    public function setNpcguid( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <etype> has a value
     *
     * @return boolean
     */
    public function hasEtype(){
      return $this->_has(4);
    }
    
    /**
     * Clear <etype> value
     *
     * @return \RO\Cmd\TriggerSkillNpcSkillCmd
     */
    public function clearEtype(){
      return $this->_clear(4);
    }
    
    /**
     * Get <etype> value
     *
     * @return int - \RO\Cmd\ETrigSkillType
     */
    public function getEtype(){
      return $this->_get(4);
    }
    
    /**
     * Set <etype> value
     *
     * @param int - \RO\Cmd\ETrigSkillType $value
     * @return \RO\Cmd\TriggerSkillNpcSkillCmd
     */
    public function setEtype( $value){
      return $this->_set(4, $value);
    }
  }
}


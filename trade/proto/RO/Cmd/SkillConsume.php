<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneSkill.proto

namespace RO\Cmd {

  class SkillConsume extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $curvalue = 0;
    
    /**  @var int */
    public $maxvalue = 0;
    
    /**  @var int */
    public $nexttime = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.SkillConsume');

      // OPTIONAL UINT32 curvalue = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "curvalue";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 maxvalue = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "maxvalue";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 nexttime = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "nexttime";
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
     * Check if <curvalue> has a value
     *
     * @return boolean
     */
    public function hasCurvalue(){
      return $this->_has(1);
    }
    
    /**
     * Clear <curvalue> value
     *
     * @return \RO\Cmd\SkillConsume
     */
    public function clearCurvalue(){
      return $this->_clear(1);
    }
    
    /**
     * Get <curvalue> value
     *
     * @return int
     */
    public function getCurvalue(){
      return $this->_get(1);
    }
    
    /**
     * Set <curvalue> value
     *
     * @param int $value
     * @return \RO\Cmd\SkillConsume
     */
    public function setCurvalue( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <maxvalue> has a value
     *
     * @return boolean
     */
    public function hasMaxvalue(){
      return $this->_has(2);
    }
    
    /**
     * Clear <maxvalue> value
     *
     * @return \RO\Cmd\SkillConsume
     */
    public function clearMaxvalue(){
      return $this->_clear(2);
    }
    
    /**
     * Get <maxvalue> value
     *
     * @return int
     */
    public function getMaxvalue(){
      return $this->_get(2);
    }
    
    /**
     * Set <maxvalue> value
     *
     * @param int $value
     * @return \RO\Cmd\SkillConsume
     */
    public function setMaxvalue( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <nexttime> has a value
     *
     * @return boolean
     */
    public function hasNexttime(){
      return $this->_has(3);
    }
    
    /**
     * Clear <nexttime> value
     *
     * @return \RO\Cmd\SkillConsume
     */
    public function clearNexttime(){
      return $this->_clear(3);
    }
    
    /**
     * Get <nexttime> value
     *
     * @return int
     */
    public function getNexttime(){
      return $this->_get(3);
    }
    
    /**
     * Set <nexttime> value
     *
     * @param int $value
     * @return \RO\Cmd\SkillConsume
     */
    public function setNexttime( $value){
      return $this->_set(3, $value);
    }
  }
}


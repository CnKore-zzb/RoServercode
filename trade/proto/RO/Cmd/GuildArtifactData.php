<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: GuildCmd.proto

namespace RO\Cmd {

  class GuildArtifactData extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $type = 0;
    
    /**  @var int */
    public $producecount = 0;
    
    /**  @var int */
    public $maxlevel = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.GuildArtifactData');

      // OPTIONAL UINT32 type = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "type";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 producecount = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "producecount";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 maxlevel = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "maxlevel";
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
     * Check if <type> has a value
     *
     * @return boolean
     */
    public function hasType(){
      return $this->_has(1);
    }
    
    /**
     * Clear <type> value
     *
     * @return \RO\Cmd\GuildArtifactData
     */
    public function clearType(){
      return $this->_clear(1);
    }
    
    /**
     * Get <type> value
     *
     * @return int
     */
    public function getType(){
      return $this->_get(1);
    }
    
    /**
     * Set <type> value
     *
     * @param int $value
     * @return \RO\Cmd\GuildArtifactData
     */
    public function setType( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <producecount> has a value
     *
     * @return boolean
     */
    public function hasProducecount(){
      return $this->_has(2);
    }
    
    /**
     * Clear <producecount> value
     *
     * @return \RO\Cmd\GuildArtifactData
     */
    public function clearProducecount(){
      return $this->_clear(2);
    }
    
    /**
     * Get <producecount> value
     *
     * @return int
     */
    public function getProducecount(){
      return $this->_get(2);
    }
    
    /**
     * Set <producecount> value
     *
     * @param int $value
     * @return \RO\Cmd\GuildArtifactData
     */
    public function setProducecount( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <maxlevel> has a value
     *
     * @return boolean
     */
    public function hasMaxlevel(){
      return $this->_has(3);
    }
    
    /**
     * Clear <maxlevel> value
     *
     * @return \RO\Cmd\GuildArtifactData
     */
    public function clearMaxlevel(){
      return $this->_clear(3);
    }
    
    /**
     * Get <maxlevel> value
     *
     * @return int
     */
    public function getMaxlevel(){
      return $this->_get(3);
    }
    
    /**
     * Set <maxlevel> value
     *
     * @param int $value
     * @return \RO\Cmd\GuildArtifactData
     */
    public function setMaxlevel( $value){
      return $this->_set(3, $value);
    }
  }
}

<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: AchieveCmd.proto

namespace RO\Cmd {

  class AchieveQuest extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $id = 0;
    
    /**  @var string */
    public $name = null;
    
    /**  @var \RO\Cmd\AchieveQuest[]  */
    public $pre = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.AchieveQuest');

      // OPTIONAL UINT32 id = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "id";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL STRING name = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "name";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // REPEATED MESSAGE pre = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "pre";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\AchieveQuest';
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <id> has a value
     *
     * @return boolean
     */
    public function hasId(){
      return $this->_has(1);
    }
    
    /**
     * Clear <id> value
     *
     * @return \RO\Cmd\AchieveQuest
     */
    public function clearId(){
      return $this->_clear(1);
    }
    
    /**
     * Get <id> value
     *
     * @return int
     */
    public function getId(){
      return $this->_get(1);
    }
    
    /**
     * Set <id> value
     *
     * @param int $value
     * @return \RO\Cmd\AchieveQuest
     */
    public function setId( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <name> has a value
     *
     * @return boolean
     */
    public function hasName(){
      return $this->_has(2);
    }
    
    /**
     * Clear <name> value
     *
     * @return \RO\Cmd\AchieveQuest
     */
    public function clearName(){
      return $this->_clear(2);
    }
    
    /**
     * Get <name> value
     *
     * @return string
     */
    public function getName(){
      return $this->_get(2);
    }
    
    /**
     * Set <name> value
     *
     * @param string $value
     * @return \RO\Cmd\AchieveQuest
     */
    public function setName( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <pre> has a value
     *
     * @return boolean
     */
    public function hasPre(){
      return $this->_has(3);
    }
    
    /**
     * Clear <pre> value
     *
     * @return \RO\Cmd\AchieveQuest
     */
    public function clearPre(){
      return $this->_clear(3);
    }
    
    /**
     * Get <pre> value
     *
     * @param int $idx
     * @return \RO\Cmd\AchieveQuest
     */
    public function getPre($idx = NULL){
      return $this->_get(3, $idx);
    }
    
    /**
     * Set <pre> value
     *
     * @param \RO\Cmd\AchieveQuest $value
     * @return \RO\Cmd\AchieveQuest
     */
    public function setPre(\RO\Cmd\AchieveQuest $value, $idx = NULL){
      return $this->_set(3, $value, $idx);
    }
    
    /**
     * Get all elements of <pre>
     *
     * @return \RO\Cmd\AchieveQuest[]
     */
    public function getPreList(){
     return $this->_get(3);
    }
    
    /**
     * Add a new element to <pre>
     *
     * @param \RO\Cmd\AchieveQuest $value
     * @return \RO\Cmd\AchieveQuest
     */
    public function addPre(\RO\Cmd\AchieveQuest $value){
     return $this->_add(3, $value);
    }
  }
}


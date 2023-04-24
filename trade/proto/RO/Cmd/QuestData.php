<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneQuest.proto

namespace RO\Cmd {

  class QuestData extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $id = 0;
    
    /**  @var int */
    public $step = 0;
    
    /**  @var int */
    public $time = 0;
    
    /**  @var boolean */
    public $complete = false;
    
    /**  @var boolean */
    public $trace = true;
    
    /**  @var \RO\Cmd\QuestStep[]  */
    public $steps = array();
    
    /**  @var \RO\Cmd\ItemInfo[]  */
    public $rewards = array();
    
    /**  @var int */
    public $version = 0;
    
    /**  @var int */
    public $acceptlv = 0;
    
    /**  @var int */
    public $finishcount = 0;
    
    /**  @var int[]  */
    public $params = array();
    
    /**  @var string[]  */
    public $names = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.QuestData');

      // OPTIONAL UINT32 id = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "id";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 step = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "step";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 time = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "time";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL BOOL complete = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "complete";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = false;
      $descriptor->addField($f);

      // OPTIONAL BOOL trace = 12
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 12;
      $f->name      = "trace";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = true;
      $descriptor->addField($f);

      // REPEATED MESSAGE steps = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "steps";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\QuestStep';
      $descriptor->addField($f);

      // REPEATED MESSAGE rewards = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "rewards";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\ItemInfo';
      $descriptor->addField($f);

      // OPTIONAL UINT32 version = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "version";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 acceptlv = 8
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 8;
      $f->name      = "acceptlv";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 finishcount = 9
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 9;
      $f->name      = "finishcount";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // REPEATED UINT64 params = 10
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 10;
      $f->name      = "params";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      // REPEATED STRING names = 11
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 11;
      $f->name      = "names";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
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
     * @return \RO\Cmd\QuestData
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
     * @return \RO\Cmd\QuestData
     */
    public function setId( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <step> has a value
     *
     * @return boolean
     */
    public function hasStep(){
      return $this->_has(2);
    }
    
    /**
     * Clear <step> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearStep(){
      return $this->_clear(2);
    }
    
    /**
     * Get <step> value
     *
     * @return int
     */
    public function getStep(){
      return $this->_get(2);
    }
    
    /**
     * Set <step> value
     *
     * @param int $value
     * @return \RO\Cmd\QuestData
     */
    public function setStep( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <time> has a value
     *
     * @return boolean
     */
    public function hasTime(){
      return $this->_has(3);
    }
    
    /**
     * Clear <time> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearTime(){
      return $this->_clear(3);
    }
    
    /**
     * Get <time> value
     *
     * @return int
     */
    public function getTime(){
      return $this->_get(3);
    }
    
    /**
     * Set <time> value
     *
     * @param int $value
     * @return \RO\Cmd\QuestData
     */
    public function setTime( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <complete> has a value
     *
     * @return boolean
     */
    public function hasComplete(){
      return $this->_has(4);
    }
    
    /**
     * Clear <complete> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearComplete(){
      return $this->_clear(4);
    }
    
    /**
     * Get <complete> value
     *
     * @return boolean
     */
    public function getComplete(){
      return $this->_get(4);
    }
    
    /**
     * Set <complete> value
     *
     * @param boolean $value
     * @return \RO\Cmd\QuestData
     */
    public function setComplete( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <trace> has a value
     *
     * @return boolean
     */
    public function hasTrace(){
      return $this->_has(12);
    }
    
    /**
     * Clear <trace> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearTrace(){
      return $this->_clear(12);
    }
    
    /**
     * Get <trace> value
     *
     * @return boolean
     */
    public function getTrace(){
      return $this->_get(12);
    }
    
    /**
     * Set <trace> value
     *
     * @param boolean $value
     * @return \RO\Cmd\QuestData
     */
    public function setTrace( $value){
      return $this->_set(12, $value);
    }
    
    /**
     * Check if <steps> has a value
     *
     * @return boolean
     */
    public function hasSteps(){
      return $this->_has(5);
    }
    
    /**
     * Clear <steps> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearSteps(){
      return $this->_clear(5);
    }
    
    /**
     * Get <steps> value
     *
     * @param int $idx
     * @return \RO\Cmd\QuestStep
     */
    public function getSteps($idx = NULL){
      return $this->_get(5, $idx);
    }
    
    /**
     * Set <steps> value
     *
     * @param \RO\Cmd\QuestStep $value
     * @return \RO\Cmd\QuestData
     */
    public function setSteps(\RO\Cmd\QuestStep $value, $idx = NULL){
      return $this->_set(5, $value, $idx);
    }
    
    /**
     * Get all elements of <steps>
     *
     * @return \RO\Cmd\QuestStep[]
     */
    public function getStepsList(){
     return $this->_get(5);
    }
    
    /**
     * Add a new element to <steps>
     *
     * @param \RO\Cmd\QuestStep $value
     * @return \RO\Cmd\QuestData
     */
    public function addSteps(\RO\Cmd\QuestStep $value){
     return $this->_add(5, $value);
    }
    
    /**
     * Check if <rewards> has a value
     *
     * @return boolean
     */
    public function hasRewards(){
      return $this->_has(6);
    }
    
    /**
     * Clear <rewards> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearRewards(){
      return $this->_clear(6);
    }
    
    /**
     * Get <rewards> value
     *
     * @param int $idx
     * @return \RO\Cmd\ItemInfo
     */
    public function getRewards($idx = NULL){
      return $this->_get(6, $idx);
    }
    
    /**
     * Set <rewards> value
     *
     * @param \RO\Cmd\ItemInfo $value
     * @return \RO\Cmd\QuestData
     */
    public function setRewards(\RO\Cmd\ItemInfo $value, $idx = NULL){
      return $this->_set(6, $value, $idx);
    }
    
    /**
     * Get all elements of <rewards>
     *
     * @return \RO\Cmd\ItemInfo[]
     */
    public function getRewardsList(){
     return $this->_get(6);
    }
    
    /**
     * Add a new element to <rewards>
     *
     * @param \RO\Cmd\ItemInfo $value
     * @return \RO\Cmd\QuestData
     */
    public function addRewards(\RO\Cmd\ItemInfo $value){
     return $this->_add(6, $value);
    }
    
    /**
     * Check if <version> has a value
     *
     * @return boolean
     */
    public function hasVersion(){
      return $this->_has(7);
    }
    
    /**
     * Clear <version> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearVersion(){
      return $this->_clear(7);
    }
    
    /**
     * Get <version> value
     *
     * @return int
     */
    public function getVersion(){
      return $this->_get(7);
    }
    
    /**
     * Set <version> value
     *
     * @param int $value
     * @return \RO\Cmd\QuestData
     */
    public function setVersion( $value){
      return $this->_set(7, $value);
    }
    
    /**
     * Check if <acceptlv> has a value
     *
     * @return boolean
     */
    public function hasAcceptlv(){
      return $this->_has(8);
    }
    
    /**
     * Clear <acceptlv> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearAcceptlv(){
      return $this->_clear(8);
    }
    
    /**
     * Get <acceptlv> value
     *
     * @return int
     */
    public function getAcceptlv(){
      return $this->_get(8);
    }
    
    /**
     * Set <acceptlv> value
     *
     * @param int $value
     * @return \RO\Cmd\QuestData
     */
    public function setAcceptlv( $value){
      return $this->_set(8, $value);
    }
    
    /**
     * Check if <finishcount> has a value
     *
     * @return boolean
     */
    public function hasFinishcount(){
      return $this->_has(9);
    }
    
    /**
     * Clear <finishcount> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearFinishcount(){
      return $this->_clear(9);
    }
    
    /**
     * Get <finishcount> value
     *
     * @return int
     */
    public function getFinishcount(){
      return $this->_get(9);
    }
    
    /**
     * Set <finishcount> value
     *
     * @param int $value
     * @return \RO\Cmd\QuestData
     */
    public function setFinishcount( $value){
      return $this->_set(9, $value);
    }
    
    /**
     * Check if <params> has a value
     *
     * @return boolean
     */
    public function hasParams(){
      return $this->_has(10);
    }
    
    /**
     * Clear <params> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearParams(){
      return $this->_clear(10);
    }
    
    /**
     * Get <params> value
     *
     * @param int $idx
     * @return int
     */
    public function getParams($idx = NULL){
      return $this->_get(10, $idx);
    }
    
    /**
     * Set <params> value
     *
     * @param int $value
     * @return \RO\Cmd\QuestData
     */
    public function setParams( $value, $idx = NULL){
      return $this->_set(10, $value, $idx);
    }
    
    /**
     * Get all elements of <params>
     *
     * @return int[]
     */
    public function getParamsList(){
     return $this->_get(10);
    }
    
    /**
     * Add a new element to <params>
     *
     * @param int $value
     * @return \RO\Cmd\QuestData
     */
    public function addParams( $value){
     return $this->_add(10, $value);
    }
    
    /**
     * Check if <names> has a value
     *
     * @return boolean
     */
    public function hasNames(){
      return $this->_has(11);
    }
    
    /**
     * Clear <names> value
     *
     * @return \RO\Cmd\QuestData
     */
    public function clearNames(){
      return $this->_clear(11);
    }
    
    /**
     * Get <names> value
     *
     * @param int $idx
     * @return string
     */
    public function getNames($idx = NULL){
      return $this->_get(11, $idx);
    }
    
    /**
     * Set <names> value
     *
     * @param string $value
     * @return \RO\Cmd\QuestData
     */
    public function setNames( $value, $idx = NULL){
      return $this->_set(11, $value, $idx);
    }
    
    /**
     * Get all elements of <names>
     *
     * @return string[]
     */
    public function getNamesList(){
     return $this->_get(11);
    }
    
    /**
     * Add a new element to <names>
     *
     * @param string $value
     * @return \RO\Cmd\QuestData
     */
    public function addNames( $value){
     return $this->_add(11, $value);
    }
  }
}


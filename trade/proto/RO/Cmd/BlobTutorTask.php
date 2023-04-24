<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordCmd.proto

namespace RO\Cmd {

  class BlobTutorTask extends \DrSlump\Protobuf\Message {

    /**  @var \RO\Cmd\TutorTaskItem[]  */
    public $items = array();
    
    /**  @var int */
    public $proficiency = 0;
    
    /**  @var \RO\Cmd\TutorReward[]  */
    public $tutorrewards = array();
    
    /**  @var int[]  */
    public $growreward = array();
    
    /**  @var int[]  */
    public $tutorgrowreward = array();
    
    /**  @var int */
    public $growrewardlv = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.BlobTutorTask');

      // REPEATED MESSAGE items = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "items";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\TutorTaskItem';
      $descriptor->addField($f);

      // OPTIONAL UINT32 proficiency = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "proficiency";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // REPEATED MESSAGE tutorrewards = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "tutorrewards";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\TutorReward';
      $descriptor->addField($f);

      // REPEATED UINT64 growreward = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "growreward";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      // REPEATED UINT64 tutorgrowreward = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "tutorgrowreward";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      // OPTIONAL UINT32 growrewardlv = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "growrewardlv";
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
     * Check if <items> has a value
     *
     * @return boolean
     */
    public function hasItems(){
      return $this->_has(1);
    }
    
    /**
     * Clear <items> value
     *
     * @return \RO\Cmd\BlobTutorTask
     */
    public function clearItems(){
      return $this->_clear(1);
    }
    
    /**
     * Get <items> value
     *
     * @param int $idx
     * @return \RO\Cmd\TutorTaskItem
     */
    public function getItems($idx = NULL){
      return $this->_get(1, $idx);
    }
    
    /**
     * Set <items> value
     *
     * @param \RO\Cmd\TutorTaskItem $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function setItems(\RO\Cmd\TutorTaskItem $value, $idx = NULL){
      return $this->_set(1, $value, $idx);
    }
    
    /**
     * Get all elements of <items>
     *
     * @return \RO\Cmd\TutorTaskItem[]
     */
    public function getItemsList(){
     return $this->_get(1);
    }
    
    /**
     * Add a new element to <items>
     *
     * @param \RO\Cmd\TutorTaskItem $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function addItems(\RO\Cmd\TutorTaskItem $value){
     return $this->_add(1, $value);
    }
    
    /**
     * Check if <proficiency> has a value
     *
     * @return boolean
     */
    public function hasProficiency(){
      return $this->_has(2);
    }
    
    /**
     * Clear <proficiency> value
     *
     * @return \RO\Cmd\BlobTutorTask
     */
    public function clearProficiency(){
      return $this->_clear(2);
    }
    
    /**
     * Get <proficiency> value
     *
     * @return int
     */
    public function getProficiency(){
      return $this->_get(2);
    }
    
    /**
     * Set <proficiency> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function setProficiency( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <tutorrewards> has a value
     *
     * @return boolean
     */
    public function hasTutorrewards(){
      return $this->_has(3);
    }
    
    /**
     * Clear <tutorrewards> value
     *
     * @return \RO\Cmd\BlobTutorTask
     */
    public function clearTutorrewards(){
      return $this->_clear(3);
    }
    
    /**
     * Get <tutorrewards> value
     *
     * @param int $idx
     * @return \RO\Cmd\TutorReward
     */
    public function getTutorrewards($idx = NULL){
      return $this->_get(3, $idx);
    }
    
    /**
     * Set <tutorrewards> value
     *
     * @param \RO\Cmd\TutorReward $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function setTutorrewards(\RO\Cmd\TutorReward $value, $idx = NULL){
      return $this->_set(3, $value, $idx);
    }
    
    /**
     * Get all elements of <tutorrewards>
     *
     * @return \RO\Cmd\TutorReward[]
     */
    public function getTutorrewardsList(){
     return $this->_get(3);
    }
    
    /**
     * Add a new element to <tutorrewards>
     *
     * @param \RO\Cmd\TutorReward $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function addTutorrewards(\RO\Cmd\TutorReward $value){
     return $this->_add(3, $value);
    }
    
    /**
     * Check if <growreward> has a value
     *
     * @return boolean
     */
    public function hasGrowreward(){
      return $this->_has(4);
    }
    
    /**
     * Clear <growreward> value
     *
     * @return \RO\Cmd\BlobTutorTask
     */
    public function clearGrowreward(){
      return $this->_clear(4);
    }
    
    /**
     * Get <growreward> value
     *
     * @param int $idx
     * @return int
     */
    public function getGrowreward($idx = NULL){
      return $this->_get(4, $idx);
    }
    
    /**
     * Set <growreward> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function setGrowreward( $value, $idx = NULL){
      return $this->_set(4, $value, $idx);
    }
    
    /**
     * Get all elements of <growreward>
     *
     * @return int[]
     */
    public function getGrowrewardList(){
     return $this->_get(4);
    }
    
    /**
     * Add a new element to <growreward>
     *
     * @param int $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function addGrowreward( $value){
     return $this->_add(4, $value);
    }
    
    /**
     * Check if <tutorgrowreward> has a value
     *
     * @return boolean
     */
    public function hasTutorgrowreward(){
      return $this->_has(5);
    }
    
    /**
     * Clear <tutorgrowreward> value
     *
     * @return \RO\Cmd\BlobTutorTask
     */
    public function clearTutorgrowreward(){
      return $this->_clear(5);
    }
    
    /**
     * Get <tutorgrowreward> value
     *
     * @param int $idx
     * @return int
     */
    public function getTutorgrowreward($idx = NULL){
      return $this->_get(5, $idx);
    }
    
    /**
     * Set <tutorgrowreward> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function setTutorgrowreward( $value, $idx = NULL){
      return $this->_set(5, $value, $idx);
    }
    
    /**
     * Get all elements of <tutorgrowreward>
     *
     * @return int[]
     */
    public function getTutorgrowrewardList(){
     return $this->_get(5);
    }
    
    /**
     * Add a new element to <tutorgrowreward>
     *
     * @param int $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function addTutorgrowreward( $value){
     return $this->_add(5, $value);
    }
    
    /**
     * Check if <growrewardlv> has a value
     *
     * @return boolean
     */
    public function hasGrowrewardlv(){
      return $this->_has(6);
    }
    
    /**
     * Clear <growrewardlv> value
     *
     * @return \RO\Cmd\BlobTutorTask
     */
    public function clearGrowrewardlv(){
      return $this->_clear(6);
    }
    
    /**
     * Get <growrewardlv> value
     *
     * @return int
     */
    public function getGrowrewardlv(){
      return $this->_get(6);
    }
    
    /**
     * Set <growrewardlv> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobTutorTask
     */
    public function setGrowrewardlv( $value){
      return $this->_set(6, $value);
    }
  }
}

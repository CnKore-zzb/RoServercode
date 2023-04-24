<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneUser2.proto

namespace RO\Cmd {

  class UpdateRecordInfoUserCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER2_PROTOCMD;
    
    /**  @var int - \RO\Cmd\User2Param */
    public $param = \RO\Cmd\User2Param::USER2PARAM_UPDATE_RECORD_INFO;
    
    /**  @var \RO\Cmd\SlotInfo[]  */
    public $slots = array();
    
    /**  @var \RO\Cmd\ProfessionUserInfo[]  */
    public $records = array();
    
    /**  @var int[]  */
    public $delete_ids = array();
    
    /**  @var int */
    public $card_expiretime = null;
    
    /**  @var \RO\Cmd\UserAstrolMaterialData[]  */
    public $astrol_data = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.UpdateRecordInfoUserCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SCENE_USER2_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\User2Param';
      $f->default   = \RO\Cmd\User2Param::USER2PARAM_UPDATE_RECORD_INFO;
      $descriptor->addField($f);

      // REPEATED MESSAGE slots = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "slots";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\SlotInfo';
      $descriptor->addField($f);

      // REPEATED MESSAGE records = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "records";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\ProfessionUserInfo';
      $descriptor->addField($f);

      // REPEATED UINT32 delete_ids = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "delete_ids";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      // OPTIONAL UINT32 card_expiretime = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "card_expiretime";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // REPEATED MESSAGE astrol_data = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "astrol_data";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\UserAstrolMaterialData';
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
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
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
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
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
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\User2Param
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\User2Param $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <slots> has a value
     *
     * @return boolean
     */
    public function hasSlots(){
      return $this->_has(3);
    }
    
    /**
     * Clear <slots> value
     *
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function clearSlots(){
      return $this->_clear(3);
    }
    
    /**
     * Get <slots> value
     *
     * @param int $idx
     * @return \RO\Cmd\SlotInfo
     */
    public function getSlots($idx = NULL){
      return $this->_get(3, $idx);
    }
    
    /**
     * Set <slots> value
     *
     * @param \RO\Cmd\SlotInfo $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function setSlots(\RO\Cmd\SlotInfo $value, $idx = NULL){
      return $this->_set(3, $value, $idx);
    }
    
    /**
     * Get all elements of <slots>
     *
     * @return \RO\Cmd\SlotInfo[]
     */
    public function getSlotsList(){
     return $this->_get(3);
    }
    
    /**
     * Add a new element to <slots>
     *
     * @param \RO\Cmd\SlotInfo $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function addSlots(\RO\Cmd\SlotInfo $value){
     return $this->_add(3, $value);
    }
    
    /**
     * Check if <records> has a value
     *
     * @return boolean
     */
    public function hasRecords(){
      return $this->_has(4);
    }
    
    /**
     * Clear <records> value
     *
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function clearRecords(){
      return $this->_clear(4);
    }
    
    /**
     * Get <records> value
     *
     * @param int $idx
     * @return \RO\Cmd\ProfessionUserInfo
     */
    public function getRecords($idx = NULL){
      return $this->_get(4, $idx);
    }
    
    /**
     * Set <records> value
     *
     * @param \RO\Cmd\ProfessionUserInfo $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function setRecords(\RO\Cmd\ProfessionUserInfo $value, $idx = NULL){
      return $this->_set(4, $value, $idx);
    }
    
    /**
     * Get all elements of <records>
     *
     * @return \RO\Cmd\ProfessionUserInfo[]
     */
    public function getRecordsList(){
     return $this->_get(4);
    }
    
    /**
     * Add a new element to <records>
     *
     * @param \RO\Cmd\ProfessionUserInfo $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function addRecords(\RO\Cmd\ProfessionUserInfo $value){
     return $this->_add(4, $value);
    }
    
    /**
     * Check if <delete_ids> has a value
     *
     * @return boolean
     */
    public function hasDeleteIds(){
      return $this->_has(5);
    }
    
    /**
     * Clear <delete_ids> value
     *
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function clearDeleteIds(){
      return $this->_clear(5);
    }
    
    /**
     * Get <delete_ids> value
     *
     * @param int $idx
     * @return int
     */
    public function getDeleteIds($idx = NULL){
      return $this->_get(5, $idx);
    }
    
    /**
     * Set <delete_ids> value
     *
     * @param int $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function setDeleteIds( $value, $idx = NULL){
      return $this->_set(5, $value, $idx);
    }
    
    /**
     * Get all elements of <delete_ids>
     *
     * @return int[]
     */
    public function getDeleteIdsList(){
     return $this->_get(5);
    }
    
    /**
     * Add a new element to <delete_ids>
     *
     * @param int $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function addDeleteIds( $value){
     return $this->_add(5, $value);
    }
    
    /**
     * Check if <card_expiretime> has a value
     *
     * @return boolean
     */
    public function hasCardExpiretime(){
      return $this->_has(6);
    }
    
    /**
     * Clear <card_expiretime> value
     *
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function clearCardExpiretime(){
      return $this->_clear(6);
    }
    
    /**
     * Get <card_expiretime> value
     *
     * @return int
     */
    public function getCardExpiretime(){
      return $this->_get(6);
    }
    
    /**
     * Set <card_expiretime> value
     *
     * @param int $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function setCardExpiretime( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <astrol_data> has a value
     *
     * @return boolean
     */
    public function hasAstrolData(){
      return $this->_has(7);
    }
    
    /**
     * Clear <astrol_data> value
     *
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function clearAstrolData(){
      return $this->_clear(7);
    }
    
    /**
     * Get <astrol_data> value
     *
     * @param int $idx
     * @return \RO\Cmd\UserAstrolMaterialData
     */
    public function getAstrolData($idx = NULL){
      return $this->_get(7, $idx);
    }
    
    /**
     * Set <astrol_data> value
     *
     * @param \RO\Cmd\UserAstrolMaterialData $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function setAstrolData(\RO\Cmd\UserAstrolMaterialData $value, $idx = NULL){
      return $this->_set(7, $value, $idx);
    }
    
    /**
     * Get all elements of <astrol_data>
     *
     * @return \RO\Cmd\UserAstrolMaterialData[]
     */
    public function getAstrolDataList(){
     return $this->_get(7);
    }
    
    /**
     * Add a new element to <astrol_data>
     *
     * @param \RO\Cmd\UserAstrolMaterialData $value
     * @return \RO\Cmd\UpdateRecordInfoUserCmd
     */
    public function addAstrolData(\RO\Cmd\UserAstrolMaterialData $value){
     return $this->_add(7, $value);
    }
  }
}

<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordCmd.proto

namespace RO\Cmd {

  class BlobCredit extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $version = 0;
    
    /**  @var int */
    public $credit = 0;
    
    /**  @var int */
    public $monster_value = 0;
    
    /**  @var int */
    public $savedtime = 0;
    
    /**  @var int */
    public $forbidtime = 0;
    
    /**  @var int */
    public $auguryreward = 0;
    
    /**  @var \RO\Cmd\BlobShopGotItem */
    public $shop = null;
    
    /**  @var \RO\Cmd\BlobAccVar */
    public $var = null;
    
    /**  @var int */
    public $maxbaselv = 0;
    
    /**  @var int */
    public $maxbaselv_resettime = 0;
    
    /**  @var \RO\Cmd\BlobActivityEvent */
    public $acevent = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.BlobCredit');

      // OPTIONAL UINT32 version = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "version";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL INT32 credit = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "credit";
      $f->type      = \DrSlump\Protobuf::TYPE_INT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 monster_value = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "monster_value";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 savedtime = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "savedtime";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 forbidtime = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "forbidtime";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 auguryreward = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "auguryreward";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL MESSAGE shop = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "shop";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\BlobShopGotItem';
      $descriptor->addField($f);

      // OPTIONAL MESSAGE var = 8
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 8;
      $f->name      = "var";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\BlobAccVar';
      $descriptor->addField($f);

      // OPTIONAL UINT32 maxbaselv = 9
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 9;
      $f->name      = "maxbaselv";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 maxbaselv_resettime = 10
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 10;
      $f->name      = "maxbaselv_resettime";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL MESSAGE acevent = 11
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 11;
      $f->name      = "acevent";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\BlobActivityEvent';
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <version> has a value
     *
     * @return boolean
     */
    public function hasVersion(){
      return $this->_has(1);
    }
    
    /**
     * Clear <version> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearVersion(){
      return $this->_clear(1);
    }
    
    /**
     * Get <version> value
     *
     * @return int
     */
    public function getVersion(){
      return $this->_get(1);
    }
    
    /**
     * Set <version> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setVersion( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <credit> has a value
     *
     * @return boolean
     */
    public function hasCredit(){
      return $this->_has(2);
    }
    
    /**
     * Clear <credit> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearCredit(){
      return $this->_clear(2);
    }
    
    /**
     * Get <credit> value
     *
     * @return int
     */
    public function getCredit(){
      return $this->_get(2);
    }
    
    /**
     * Set <credit> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setCredit( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <monster_value> has a value
     *
     * @return boolean
     */
    public function hasMonsterValue(){
      return $this->_has(3);
    }
    
    /**
     * Clear <monster_value> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearMonsterValue(){
      return $this->_clear(3);
    }
    
    /**
     * Get <monster_value> value
     *
     * @return int
     */
    public function getMonsterValue(){
      return $this->_get(3);
    }
    
    /**
     * Set <monster_value> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setMonsterValue( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <savedtime> has a value
     *
     * @return boolean
     */
    public function hasSavedtime(){
      return $this->_has(4);
    }
    
    /**
     * Clear <savedtime> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearSavedtime(){
      return $this->_clear(4);
    }
    
    /**
     * Get <savedtime> value
     *
     * @return int
     */
    public function getSavedtime(){
      return $this->_get(4);
    }
    
    /**
     * Set <savedtime> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setSavedtime( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <forbidtime> has a value
     *
     * @return boolean
     */
    public function hasForbidtime(){
      return $this->_has(5);
    }
    
    /**
     * Clear <forbidtime> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearForbidtime(){
      return $this->_clear(5);
    }
    
    /**
     * Get <forbidtime> value
     *
     * @return int
     */
    public function getForbidtime(){
      return $this->_get(5);
    }
    
    /**
     * Set <forbidtime> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setForbidtime( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <auguryreward> has a value
     *
     * @return boolean
     */
    public function hasAuguryreward(){
      return $this->_has(6);
    }
    
    /**
     * Clear <auguryreward> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearAuguryreward(){
      return $this->_clear(6);
    }
    
    /**
     * Get <auguryreward> value
     *
     * @return int
     */
    public function getAuguryreward(){
      return $this->_get(6);
    }
    
    /**
     * Set <auguryreward> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setAuguryreward( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <shop> has a value
     *
     * @return boolean
     */
    public function hasShop(){
      return $this->_has(7);
    }
    
    /**
     * Clear <shop> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearShop(){
      return $this->_clear(7);
    }
    
    /**
     * Get <shop> value
     *
     * @return \RO\Cmd\BlobShopGotItem
     */
    public function getShop(){
      return $this->_get(7);
    }
    
    /**
     * Set <shop> value
     *
     * @param \RO\Cmd\BlobShopGotItem $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setShop(\RO\Cmd\BlobShopGotItem $value){
      return $this->_set(7, $value);
    }
    
    /**
     * Check if <var> has a value
     *
     * @return boolean
     */
    public function hasVar(){
      return $this->_has(8);
    }
    
    /**
     * Clear <var> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearVar(){
      return $this->_clear(8);
    }
    
    /**
     * Get <var> value
     *
     * @return \RO\Cmd\BlobAccVar
     */
    public function getVar(){
      return $this->_get(8);
    }
    
    /**
     * Set <var> value
     *
     * @param \RO\Cmd\BlobAccVar $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setVar(\RO\Cmd\BlobAccVar $value){
      return $this->_set(8, $value);
    }
    
    /**
     * Check if <maxbaselv> has a value
     *
     * @return boolean
     */
    public function hasMaxbaselv(){
      return $this->_has(9);
    }
    
    /**
     * Clear <maxbaselv> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearMaxbaselv(){
      return $this->_clear(9);
    }
    
    /**
     * Get <maxbaselv> value
     *
     * @return int
     */
    public function getMaxbaselv(){
      return $this->_get(9);
    }
    
    /**
     * Set <maxbaselv> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setMaxbaselv( $value){
      return $this->_set(9, $value);
    }
    
    /**
     * Check if <maxbaselv_resettime> has a value
     *
     * @return boolean
     */
    public function hasMaxbaselvResettime(){
      return $this->_has(10);
    }
    
    /**
     * Clear <maxbaselv_resettime> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearMaxbaselvResettime(){
      return $this->_clear(10);
    }
    
    /**
     * Get <maxbaselv_resettime> value
     *
     * @return int
     */
    public function getMaxbaselvResettime(){
      return $this->_get(10);
    }
    
    /**
     * Set <maxbaselv_resettime> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setMaxbaselvResettime( $value){
      return $this->_set(10, $value);
    }
    
    /**
     * Check if <acevent> has a value
     *
     * @return boolean
     */
    public function hasAcevent(){
      return $this->_has(11);
    }
    
    /**
     * Clear <acevent> value
     *
     * @return \RO\Cmd\BlobCredit
     */
    public function clearAcevent(){
      return $this->_clear(11);
    }
    
    /**
     * Get <acevent> value
     *
     * @return \RO\Cmd\BlobActivityEvent
     */
    public function getAcevent(){
      return $this->_get(11);
    }
    
    /**
     * Set <acevent> value
     *
     * @param \RO\Cmd\BlobActivityEvent $value
     * @return \RO\Cmd\BlobCredit
     */
    public function setAcevent(\RO\Cmd\BlobActivityEvent $value){
      return $this->_set(11, $value);
    }
  }
}


<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SceneUser2.proto

namespace RO\Cmd {

  class QueryShopGotItem extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SCENE_USER2_PROTOCMD;
    
    /**  @var int - \RO\Cmd\User2Param */
    public $param = \RO\Cmd\User2Param::USER2PARAM_QUERYSHOPGOTITEM;
    
    /**  @var \RO\Cmd\ShopGotItem[]  */
    public $items = array();
    
    /**  @var \RO\Cmd\ShopGotItem[]  */
    public $discountitems = array();
    
    /**  @var \RO\Cmd\ShopGotItem[]  */
    public $limititems = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.QueryShopGotItem');

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
      $f->default   = \RO\Cmd\User2Param::USER2PARAM_QUERYSHOPGOTITEM;
      $descriptor->addField($f);

      // REPEATED MESSAGE items = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "items";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\ShopGotItem';
      $descriptor->addField($f);

      // REPEATED MESSAGE discountitems = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "discountitems";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\ShopGotItem';
      $descriptor->addField($f);

      // REPEATED MESSAGE limititems = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "limititems";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\ShopGotItem';
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
     * @return \RO\Cmd\QueryShopGotItem
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
     * @return \RO\Cmd\QueryShopGotItem
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
     * @return \RO\Cmd\QueryShopGotItem
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
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <items> has a value
     *
     * @return boolean
     */
    public function hasItems(){
      return $this->_has(3);
    }
    
    /**
     * Clear <items> value
     *
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function clearItems(){
      return $this->_clear(3);
    }
    
    /**
     * Get <items> value
     *
     * @param int $idx
     * @return \RO\Cmd\ShopGotItem
     */
    public function getItems($idx = NULL){
      return $this->_get(3, $idx);
    }
    
    /**
     * Set <items> value
     *
     * @param \RO\Cmd\ShopGotItem $value
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function setItems(\RO\Cmd\ShopGotItem $value, $idx = NULL){
      return $this->_set(3, $value, $idx);
    }
    
    /**
     * Get all elements of <items>
     *
     * @return \RO\Cmd\ShopGotItem[]
     */
    public function getItemsList(){
     return $this->_get(3);
    }
    
    /**
     * Add a new element to <items>
     *
     * @param \RO\Cmd\ShopGotItem $value
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function addItems(\RO\Cmd\ShopGotItem $value){
     return $this->_add(3, $value);
    }
    
    /**
     * Check if <discountitems> has a value
     *
     * @return boolean
     */
    public function hasDiscountitems(){
      return $this->_has(4);
    }
    
    /**
     * Clear <discountitems> value
     *
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function clearDiscountitems(){
      return $this->_clear(4);
    }
    
    /**
     * Get <discountitems> value
     *
     * @param int $idx
     * @return \RO\Cmd\ShopGotItem
     */
    public function getDiscountitems($idx = NULL){
      return $this->_get(4, $idx);
    }
    
    /**
     * Set <discountitems> value
     *
     * @param \RO\Cmd\ShopGotItem $value
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function setDiscountitems(\RO\Cmd\ShopGotItem $value, $idx = NULL){
      return $this->_set(4, $value, $idx);
    }
    
    /**
     * Get all elements of <discountitems>
     *
     * @return \RO\Cmd\ShopGotItem[]
     */
    public function getDiscountitemsList(){
     return $this->_get(4);
    }
    
    /**
     * Add a new element to <discountitems>
     *
     * @param \RO\Cmd\ShopGotItem $value
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function addDiscountitems(\RO\Cmd\ShopGotItem $value){
     return $this->_add(4, $value);
    }
    
    /**
     * Check if <limititems> has a value
     *
     * @return boolean
     */
    public function hasLimititems(){
      return $this->_has(5);
    }
    
    /**
     * Clear <limititems> value
     *
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function clearLimititems(){
      return $this->_clear(5);
    }
    
    /**
     * Get <limititems> value
     *
     * @param int $idx
     * @return \RO\Cmd\ShopGotItem
     */
    public function getLimititems($idx = NULL){
      return $this->_get(5, $idx);
    }
    
    /**
     * Set <limititems> value
     *
     * @param \RO\Cmd\ShopGotItem $value
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function setLimititems(\RO\Cmd\ShopGotItem $value, $idx = NULL){
      return $this->_set(5, $value, $idx);
    }
    
    /**
     * Get all elements of <limititems>
     *
     * @return \RO\Cmd\ShopGotItem[]
     */
    public function getLimititemsList(){
     return $this->_get(5);
    }
    
    /**
     * Add a new element to <limititems>
     *
     * @param \RO\Cmd\ShopGotItem $value
     * @return \RO\Cmd\QueryShopGotItem
     */
    public function addLimititems(\RO\Cmd\ShopGotItem $value){
     return $this->_add(5, $value);
    }
  }
}


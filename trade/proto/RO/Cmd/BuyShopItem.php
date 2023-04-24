<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SessionShop.proto

namespace RO\Cmd {

  class BuyShopItem extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SESSION_USER_SHOP_PROTOCMD;
    
    /**  @var int - \RO\Cmd\ShopParam */
    public $param = \RO\Cmd\ShopParam::SHOPPARAM_BUYITEM;
    
    /**  @var int */
    public $id = 0;
    
    /**  @var int */
    public $count = 0;
    
    /**  @var int */
    public $price = 0;
    
    /**  @var int */
    public $price2 = 0;
    
    /**  @var boolean */
    public $success = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.BuyShopItem');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SESSION_USER_SHOP_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ShopParam';
      $f->default   = \RO\Cmd\ShopParam::SHOPPARAM_BUYITEM;
      $descriptor->addField($f);

      // OPTIONAL UINT32 id = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "id";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 count = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "count";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 price = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "price";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 price2 = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "price2";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL BOOL success = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "success";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
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
     * @return \RO\Cmd\BuyShopItem
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
     * @return \RO\Cmd\BuyShopItem
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
     * @return \RO\Cmd\BuyShopItem
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\ShopParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\ShopParam $value
     * @return \RO\Cmd\BuyShopItem
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <id> has a value
     *
     * @return boolean
     */
    public function hasId(){
      return $this->_has(3);
    }
    
    /**
     * Clear <id> value
     *
     * @return \RO\Cmd\BuyShopItem
     */
    public function clearId(){
      return $this->_clear(3);
    }
    
    /**
     * Get <id> value
     *
     * @return int
     */
    public function getId(){
      return $this->_get(3);
    }
    
    /**
     * Set <id> value
     *
     * @param int $value
     * @return \RO\Cmd\BuyShopItem
     */
    public function setId( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <count> has a value
     *
     * @return boolean
     */
    public function hasCount(){
      return $this->_has(4);
    }
    
    /**
     * Clear <count> value
     *
     * @return \RO\Cmd\BuyShopItem
     */
    public function clearCount(){
      return $this->_clear(4);
    }
    
    /**
     * Get <count> value
     *
     * @return int
     */
    public function getCount(){
      return $this->_get(4);
    }
    
    /**
     * Set <count> value
     *
     * @param int $value
     * @return \RO\Cmd\BuyShopItem
     */
    public function setCount( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <price> has a value
     *
     * @return boolean
     */
    public function hasPrice(){
      return $this->_has(5);
    }
    
    /**
     * Clear <price> value
     *
     * @return \RO\Cmd\BuyShopItem
     */
    public function clearPrice(){
      return $this->_clear(5);
    }
    
    /**
     * Get <price> value
     *
     * @return int
     */
    public function getPrice(){
      return $this->_get(5);
    }
    
    /**
     * Set <price> value
     *
     * @param int $value
     * @return \RO\Cmd\BuyShopItem
     */
    public function setPrice( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <price2> has a value
     *
     * @return boolean
     */
    public function hasPrice2(){
      return $this->_has(6);
    }
    
    /**
     * Clear <price2> value
     *
     * @return \RO\Cmd\BuyShopItem
     */
    public function clearPrice2(){
      return $this->_clear(6);
    }
    
    /**
     * Get <price2> value
     *
     * @return int
     */
    public function getPrice2(){
      return $this->_get(6);
    }
    
    /**
     * Set <price2> value
     *
     * @param int $value
     * @return \RO\Cmd\BuyShopItem
     */
    public function setPrice2( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <success> has a value
     *
     * @return boolean
     */
    public function hasSuccess(){
      return $this->_has(7);
    }
    
    /**
     * Clear <success> value
     *
     * @return \RO\Cmd\BuyShopItem
     */
    public function clearSuccess(){
      return $this->_clear(7);
    }
    
    /**
     * Get <success> value
     *
     * @return boolean
     */
    public function getSuccess(){
      return $this->_get(7);
    }
    
    /**
     * Set <success> value
     *
     * @param boolean $value
     * @return \RO\Cmd\BuyShopItem
     */
    public function setSuccess( $value){
      return $this->_set(7, $value);
    }
  }
}


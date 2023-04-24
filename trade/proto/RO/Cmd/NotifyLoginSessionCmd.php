<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: SessionCmd.proto

namespace RO\Cmd {

  class NotifyLoginSessionCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SESSION_PROTOCMD;
    
    /**  @var int - \RO\Cmd\SessionParam */
    public $param = \RO\Cmd\SessionParam::SESSIONPARAM_NOTIFY_LOGIN;
    
    /**  @var int */
    public $id = 0;
    
    /**  @var int */
    public $accid = 0;
    
    /**  @var int */
    public $mapid = 0;
    
    /**  @var boolean */
    public $ischangescene = false;
    
    /**  @var string */
    public $name = null;
    
    /**  @var string */
    public $gatename = null;
    
    /**  @var string */
    public $phone = null;
    
    /**  @var boolean */
    public $ignorepwd = null;
    
    /**  @var int */
    public $language = null;
    
    /**  @var boolean */
    public $realauthorized = null;
    
    /**  @var int */
    public $maxbaselv = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.NotifyLoginSessionCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SESSION_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\SessionParam';
      $f->default   = \RO\Cmd\SessionParam::SESSIONPARAM_NOTIFY_LOGIN;
      $descriptor->addField($f);

      // OPTIONAL UINT64 id = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "id";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT64 accid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "accid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 mapid = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "mapid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL BOOL ischangescene = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "ischangescene";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = false;
      $descriptor->addField($f);

      // OPTIONAL STRING name = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "name";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL STRING gatename = 8
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 8;
      $f->name      = "gatename";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL STRING phone = 9
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 9;
      $f->name      = "phone";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL BOOL ignorepwd = 10
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 10;
      $f->name      = "ignorepwd";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 language = 11
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 11;
      $f->name      = "language";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL BOOL realauthorized = 12
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 12;
      $f->name      = "realauthorized";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 maxbaselv = 13
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 13;
      $f->name      = "maxbaselv";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
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
     * @return \RO\Cmd\NotifyLoginSessionCmd
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
     * @return \RO\Cmd\NotifyLoginSessionCmd
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
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\SessionParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\SessionParam $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
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
     * @return \RO\Cmd\NotifyLoginSessionCmd
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
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setId( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <accid> has a value
     *
     * @return boolean
     */
    public function hasAccid(){
      return $this->_has(4);
    }
    
    /**
     * Clear <accid> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearAccid(){
      return $this->_clear(4);
    }
    
    /**
     * Get <accid> value
     *
     * @return int
     */
    public function getAccid(){
      return $this->_get(4);
    }
    
    /**
     * Set <accid> value
     *
     * @param int $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setAccid( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <mapid> has a value
     *
     * @return boolean
     */
    public function hasMapid(){
      return $this->_has(5);
    }
    
    /**
     * Clear <mapid> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearMapid(){
      return $this->_clear(5);
    }
    
    /**
     * Get <mapid> value
     *
     * @return int
     */
    public function getMapid(){
      return $this->_get(5);
    }
    
    /**
     * Set <mapid> value
     *
     * @param int $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setMapid( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <ischangescene> has a value
     *
     * @return boolean
     */
    public function hasIschangescene(){
      return $this->_has(6);
    }
    
    /**
     * Clear <ischangescene> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearIschangescene(){
      return $this->_clear(6);
    }
    
    /**
     * Get <ischangescene> value
     *
     * @return boolean
     */
    public function getIschangescene(){
      return $this->_get(6);
    }
    
    /**
     * Set <ischangescene> value
     *
     * @param boolean $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setIschangescene( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <name> has a value
     *
     * @return boolean
     */
    public function hasName(){
      return $this->_has(7);
    }
    
    /**
     * Clear <name> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearName(){
      return $this->_clear(7);
    }
    
    /**
     * Get <name> value
     *
     * @return string
     */
    public function getName(){
      return $this->_get(7);
    }
    
    /**
     * Set <name> value
     *
     * @param string $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setName( $value){
      return $this->_set(7, $value);
    }
    
    /**
     * Check if <gatename> has a value
     *
     * @return boolean
     */
    public function hasGatename(){
      return $this->_has(8);
    }
    
    /**
     * Clear <gatename> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearGatename(){
      return $this->_clear(8);
    }
    
    /**
     * Get <gatename> value
     *
     * @return string
     */
    public function getGatename(){
      return $this->_get(8);
    }
    
    /**
     * Set <gatename> value
     *
     * @param string $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setGatename( $value){
      return $this->_set(8, $value);
    }
    
    /**
     * Check if <phone> has a value
     *
     * @return boolean
     */
    public function hasPhone(){
      return $this->_has(9);
    }
    
    /**
     * Clear <phone> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearPhone(){
      return $this->_clear(9);
    }
    
    /**
     * Get <phone> value
     *
     * @return string
     */
    public function getPhone(){
      return $this->_get(9);
    }
    
    /**
     * Set <phone> value
     *
     * @param string $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setPhone( $value){
      return $this->_set(9, $value);
    }
    
    /**
     * Check if <ignorepwd> has a value
     *
     * @return boolean
     */
    public function hasIgnorepwd(){
      return $this->_has(10);
    }
    
    /**
     * Clear <ignorepwd> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearIgnorepwd(){
      return $this->_clear(10);
    }
    
    /**
     * Get <ignorepwd> value
     *
     * @return boolean
     */
    public function getIgnorepwd(){
      return $this->_get(10);
    }
    
    /**
     * Set <ignorepwd> value
     *
     * @param boolean $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setIgnorepwd( $value){
      return $this->_set(10, $value);
    }
    
    /**
     * Check if <language> has a value
     *
     * @return boolean
     */
    public function hasLanguage(){
      return $this->_has(11);
    }
    
    /**
     * Clear <language> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearLanguage(){
      return $this->_clear(11);
    }
    
    /**
     * Get <language> value
     *
     * @return int
     */
    public function getLanguage(){
      return $this->_get(11);
    }
    
    /**
     * Set <language> value
     *
     * @param int $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setLanguage( $value){
      return $this->_set(11, $value);
    }
    
    /**
     * Check if <realauthorized> has a value
     *
     * @return boolean
     */
    public function hasRealauthorized(){
      return $this->_has(12);
    }
    
    /**
     * Clear <realauthorized> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearRealauthorized(){
      return $this->_clear(12);
    }
    
    /**
     * Get <realauthorized> value
     *
     * @return boolean
     */
    public function getRealauthorized(){
      return $this->_get(12);
    }
    
    /**
     * Set <realauthorized> value
     *
     * @param boolean $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setRealauthorized( $value){
      return $this->_set(12, $value);
    }
    
    /**
     * Check if <maxbaselv> has a value
     *
     * @return boolean
     */
    public function hasMaxbaselv(){
      return $this->_has(13);
    }
    
    /**
     * Clear <maxbaselv> value
     *
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function clearMaxbaselv(){
      return $this->_clear(13);
    }
    
    /**
     * Get <maxbaselv> value
     *
     * @return int
     */
    public function getMaxbaselv(){
      return $this->_get(13);
    }
    
    /**
     * Set <maxbaselv> value
     *
     * @param int $value
     * @return \RO\Cmd\NotifyLoginSessionCmd
     */
    public function setMaxbaselv( $value){
      return $this->_set(13, $value);
    }
  }
}


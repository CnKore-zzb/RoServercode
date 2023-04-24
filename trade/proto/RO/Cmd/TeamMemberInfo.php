<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: TeamCmd.proto

namespace RO\Cmd {

  class TeamMemberInfo extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $charid = 0;
    
    /**  @var int */
    public $mapid = 0;
    
    /**  @var int */
    public $raidid = 0;
    
    /**  @var int */
    public $zoneid = 0;
    
    /**  @var int - \RO\Cmd\EGender */
    public $gender = \RO\Cmd\EGender::EGENDER_MIN;
    
    /**  @var string */
    public $name = null;
    
    /**  @var int */
    public $catid = 0;
    
    /**  @var int */
    public $guildraidindex = 0;
    
    /**  @var boolean */
    public $online = false;
    
    /**  @var int */
    public $level = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.TeamMemberInfo');

      // OPTIONAL UINT64 charid = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "charid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 mapid = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "mapid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 raidid = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "raidid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 zoneid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "zoneid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL ENUM gender = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "gender";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EGender';
      $f->default   = \RO\Cmd\EGender::EGENDER_MIN;
      $descriptor->addField($f);

      // OPTIONAL STRING name = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "name";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT32 catid = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "catid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 guildraidindex = 8
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 8;
      $f->name      = "guildraidindex";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL BOOL online = 9
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 9;
      $f->name      = "online";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = false;
      $descriptor->addField($f);

      // OPTIONAL UINT32 level = 10
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 10;
      $f->name      = "level";
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
     * Check if <charid> has a value
     *
     * @return boolean
     */
    public function hasCharid(){
      return $this->_has(1);
    }
    
    /**
     * Clear <charid> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearCharid(){
      return $this->_clear(1);
    }
    
    /**
     * Get <charid> value
     *
     * @return int
     */
    public function getCharid(){
      return $this->_get(1);
    }
    
    /**
     * Set <charid> value
     *
     * @param int $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setCharid( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <mapid> has a value
     *
     * @return boolean
     */
    public function hasMapid(){
      return $this->_has(2);
    }
    
    /**
     * Clear <mapid> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearMapid(){
      return $this->_clear(2);
    }
    
    /**
     * Get <mapid> value
     *
     * @return int
     */
    public function getMapid(){
      return $this->_get(2);
    }
    
    /**
     * Set <mapid> value
     *
     * @param int $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setMapid( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <raidid> has a value
     *
     * @return boolean
     */
    public function hasRaidid(){
      return $this->_has(3);
    }
    
    /**
     * Clear <raidid> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearRaidid(){
      return $this->_clear(3);
    }
    
    /**
     * Get <raidid> value
     *
     * @return int
     */
    public function getRaidid(){
      return $this->_get(3);
    }
    
    /**
     * Set <raidid> value
     *
     * @param int $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setRaidid( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <zoneid> has a value
     *
     * @return boolean
     */
    public function hasZoneid(){
      return $this->_has(4);
    }
    
    /**
     * Clear <zoneid> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearZoneid(){
      return $this->_clear(4);
    }
    
    /**
     * Get <zoneid> value
     *
     * @return int
     */
    public function getZoneid(){
      return $this->_get(4);
    }
    
    /**
     * Set <zoneid> value
     *
     * @param int $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setZoneid( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <gender> has a value
     *
     * @return boolean
     */
    public function hasGender(){
      return $this->_has(5);
    }
    
    /**
     * Clear <gender> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearGender(){
      return $this->_clear(5);
    }
    
    /**
     * Get <gender> value
     *
     * @return int - \RO\Cmd\EGender
     */
    public function getGender(){
      return $this->_get(5);
    }
    
    /**
     * Set <gender> value
     *
     * @param int - \RO\Cmd\EGender $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setGender( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <name> has a value
     *
     * @return boolean
     */
    public function hasName(){
      return $this->_has(6);
    }
    
    /**
     * Clear <name> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearName(){
      return $this->_clear(6);
    }
    
    /**
     * Get <name> value
     *
     * @return string
     */
    public function getName(){
      return $this->_get(6);
    }
    
    /**
     * Set <name> value
     *
     * @param string $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setName( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <catid> has a value
     *
     * @return boolean
     */
    public function hasCatid(){
      return $this->_has(7);
    }
    
    /**
     * Clear <catid> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearCatid(){
      return $this->_clear(7);
    }
    
    /**
     * Get <catid> value
     *
     * @return int
     */
    public function getCatid(){
      return $this->_get(7);
    }
    
    /**
     * Set <catid> value
     *
     * @param int $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setCatid( $value){
      return $this->_set(7, $value);
    }
    
    /**
     * Check if <guildraidindex> has a value
     *
     * @return boolean
     */
    public function hasGuildraidindex(){
      return $this->_has(8);
    }
    
    /**
     * Clear <guildraidindex> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearGuildraidindex(){
      return $this->_clear(8);
    }
    
    /**
     * Get <guildraidindex> value
     *
     * @return int
     */
    public function getGuildraidindex(){
      return $this->_get(8);
    }
    
    /**
     * Set <guildraidindex> value
     *
     * @param int $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setGuildraidindex( $value){
      return $this->_set(8, $value);
    }
    
    /**
     * Check if <online> has a value
     *
     * @return boolean
     */
    public function hasOnline(){
      return $this->_has(9);
    }
    
    /**
     * Clear <online> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearOnline(){
      return $this->_clear(9);
    }
    
    /**
     * Get <online> value
     *
     * @return boolean
     */
    public function getOnline(){
      return $this->_get(9);
    }
    
    /**
     * Set <online> value
     *
     * @param boolean $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setOnline( $value){
      return $this->_set(9, $value);
    }
    
    /**
     * Check if <level> has a value
     *
     * @return boolean
     */
    public function hasLevel(){
      return $this->_has(10);
    }
    
    /**
     * Clear <level> value
     *
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function clearLevel(){
      return $this->_clear(10);
    }
    
    /**
     * Get <level> value
     *
     * @return int
     */
    public function getLevel(){
      return $this->_get(10);
    }
    
    /**
     * Set <level> value
     *
     * @param int $value
     * @return \RO\Cmd\TeamMemberInfo
     */
    public function setLevel( $value){
      return $this->_set(10, $value);
    }
  }
}


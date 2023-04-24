<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: GuildCmd.proto

namespace RO\Cmd {

  class ArtifactOptGuildCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::SESSION_USER_GUILD_PROTOCMD;
    
    /**  @var int - \RO\Cmd\GuildParam */
    public $param = \RO\Cmd\GuildParam::GUILDPARAM_ARTIFACT_OPT;
    
    /**  @var int - \RO\Cmd\EArtifactOptType */
    public $opt = \RO\Cmd\EArtifactOptType::EARTIFACTOPTTYPE_MIN;
    
    /**  @var string[]  */
    public $guid = array();
    
    /**  @var int */
    public $charid = 0;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.ArtifactOptGuildCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::SESSION_USER_GUILD_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\GuildParam';
      $f->default   = \RO\Cmd\GuildParam::GUILDPARAM_ARTIFACT_OPT;
      $descriptor->addField($f);

      // OPTIONAL ENUM opt = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "opt";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EArtifactOptType';
      $f->default   = \RO\Cmd\EArtifactOptType::EARTIFACTOPTTYPE_MIN;
      $descriptor->addField($f);

      // REPEATED STRING guid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "guid";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      // OPTIONAL UINT64 charid = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "charid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
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
     * @return \RO\Cmd\ArtifactOptGuildCmd
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
     * @return \RO\Cmd\ArtifactOptGuildCmd
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
     * @return \RO\Cmd\ArtifactOptGuildCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\GuildParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\GuildParam $value
     * @return \RO\Cmd\ArtifactOptGuildCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <opt> has a value
     *
     * @return boolean
     */
    public function hasOpt(){
      return $this->_has(3);
    }
    
    /**
     * Clear <opt> value
     *
     * @return \RO\Cmd\ArtifactOptGuildCmd
     */
    public function clearOpt(){
      return $this->_clear(3);
    }
    
    /**
     * Get <opt> value
     *
     * @return int - \RO\Cmd\EArtifactOptType
     */
    public function getOpt(){
      return $this->_get(3);
    }
    
    /**
     * Set <opt> value
     *
     * @param int - \RO\Cmd\EArtifactOptType $value
     * @return \RO\Cmd\ArtifactOptGuildCmd
     */
    public function setOpt( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <guid> has a value
     *
     * @return boolean
     */
    public function hasGuid(){
      return $this->_has(4);
    }
    
    /**
     * Clear <guid> value
     *
     * @return \RO\Cmd\ArtifactOptGuildCmd
     */
    public function clearGuid(){
      return $this->_clear(4);
    }
    
    /**
     * Get <guid> value
     *
     * @param int $idx
     * @return string
     */
    public function getGuid($idx = NULL){
      return $this->_get(4, $idx);
    }
    
    /**
     * Set <guid> value
     *
     * @param string $value
     * @return \RO\Cmd\ArtifactOptGuildCmd
     */
    public function setGuid( $value, $idx = NULL){
      return $this->_set(4, $value, $idx);
    }
    
    /**
     * Get all elements of <guid>
     *
     * @return string[]
     */
    public function getGuidList(){
     return $this->_get(4);
    }
    
    /**
     * Add a new element to <guid>
     *
     * @param string $value
     * @return \RO\Cmd\ArtifactOptGuildCmd
     */
    public function addGuid( $value){
     return $this->_add(4, $value);
    }
    
    /**
     * Check if <charid> has a value
     *
     * @return boolean
     */
    public function hasCharid(){
      return $this->_has(5);
    }
    
    /**
     * Clear <charid> value
     *
     * @return \RO\Cmd\ArtifactOptGuildCmd
     */
    public function clearCharid(){
      return $this->_clear(5);
    }
    
    /**
     * Get <charid> value
     *
     * @return int
     */
    public function getCharid(){
      return $this->_get(5);
    }
    
    /**
     * Set <charid> value
     *
     * @param int $value
     * @return \RO\Cmd\ArtifactOptGuildCmd
     */
    public function setCharid( $value){
      return $this->_set(5, $value);
    }
  }
}


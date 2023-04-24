<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: GuildSCmd.proto

namespace RO\Cmd {

  class QueryShowPhotoGuildSCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::GUILD_PROTOCMD;
    
    /**  @var int - \RO\Cmd\GuildSParam */
    public $param = \RO\Cmd\GuildSParam::GUILDSPARAM_QUERY_SHOWPHOTOLIST;
    
    /**  @var int - \RO\Cmd\EPhotoAction */
    public $action = \RO\Cmd\EPhotoAction::EPHOTOACTION_MIN;
    
    /**  @var int */
    public $guildid = 0;
    
    /**  @var \RO\Cmd\PhotoLoad[]  */
    public $loads = array();
    
    /**  @var string[]  */
    public $exists = array();
    
    /**  @var int[]  */
    public $members = array();
    
    /**  @var \RO\Cmd\PhotoFrame[]  */
    public $results = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.QueryShowPhotoGuildSCmd');

      // OPTIONAL ENUM cmd = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "cmd";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\Command';
      $f->default   = \RO\Cmd\Command::GUILD_PROTOCMD;
      $descriptor->addField($f);

      // OPTIONAL ENUM param = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "param";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\GuildSParam';
      $f->default   = \RO\Cmd\GuildSParam::GUILDSPARAM_QUERY_SHOWPHOTOLIST;
      $descriptor->addField($f);

      // OPTIONAL ENUM action = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "action";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EPhotoAction';
      $f->default   = \RO\Cmd\EPhotoAction::EPHOTOACTION_MIN;
      $descriptor->addField($f);

      // OPTIONAL UINT64 guildid = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "guildid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // REPEATED MESSAGE loads = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "loads";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\PhotoLoad';
      $descriptor->addField($f);

      // REPEATED STRING exists = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "exists";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      // REPEATED UINT64 members = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "members";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $descriptor->addField($f);

      // REPEATED MESSAGE results = 8
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 8;
      $f->name      = "results";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\PhotoFrame';
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
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
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
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
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
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function clearParam(){
      return $this->_clear(2);
    }
    
    /**
     * Get <param> value
     *
     * @return int - \RO\Cmd\GuildSParam
     */
    public function getParam(){
      return $this->_get(2);
    }
    
    /**
     * Set <param> value
     *
     * @param int - \RO\Cmd\GuildSParam $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function setParam( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <action> has a value
     *
     * @return boolean
     */
    public function hasAction(){
      return $this->_has(3);
    }
    
    /**
     * Clear <action> value
     *
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function clearAction(){
      return $this->_clear(3);
    }
    
    /**
     * Get <action> value
     *
     * @return int - \RO\Cmd\EPhotoAction
     */
    public function getAction(){
      return $this->_get(3);
    }
    
    /**
     * Set <action> value
     *
     * @param int - \RO\Cmd\EPhotoAction $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function setAction( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <guildid> has a value
     *
     * @return boolean
     */
    public function hasGuildid(){
      return $this->_has(4);
    }
    
    /**
     * Clear <guildid> value
     *
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function clearGuildid(){
      return $this->_clear(4);
    }
    
    /**
     * Get <guildid> value
     *
     * @return int
     */
    public function getGuildid(){
      return $this->_get(4);
    }
    
    /**
     * Set <guildid> value
     *
     * @param int $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function setGuildid( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <loads> has a value
     *
     * @return boolean
     */
    public function hasLoads(){
      return $this->_has(5);
    }
    
    /**
     * Clear <loads> value
     *
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function clearLoads(){
      return $this->_clear(5);
    }
    
    /**
     * Get <loads> value
     *
     * @param int $idx
     * @return \RO\Cmd\PhotoLoad
     */
    public function getLoads($idx = NULL){
      return $this->_get(5, $idx);
    }
    
    /**
     * Set <loads> value
     *
     * @param \RO\Cmd\PhotoLoad $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function setLoads(\RO\Cmd\PhotoLoad $value, $idx = NULL){
      return $this->_set(5, $value, $idx);
    }
    
    /**
     * Get all elements of <loads>
     *
     * @return \RO\Cmd\PhotoLoad[]
     */
    public function getLoadsList(){
     return $this->_get(5);
    }
    
    /**
     * Add a new element to <loads>
     *
     * @param \RO\Cmd\PhotoLoad $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function addLoads(\RO\Cmd\PhotoLoad $value){
     return $this->_add(5, $value);
    }
    
    /**
     * Check if <exists> has a value
     *
     * @return boolean
     */
    public function hasExists(){
      return $this->_has(6);
    }
    
    /**
     * Clear <exists> value
     *
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function clearExists(){
      return $this->_clear(6);
    }
    
    /**
     * Get <exists> value
     *
     * @param int $idx
     * @return string
     */
    public function getExists($idx = NULL){
      return $this->_get(6, $idx);
    }
    
    /**
     * Set <exists> value
     *
     * @param string $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function setExists( $value, $idx = NULL){
      return $this->_set(6, $value, $idx);
    }
    
    /**
     * Get all elements of <exists>
     *
     * @return string[]
     */
    public function getExistsList(){
     return $this->_get(6);
    }
    
    /**
     * Add a new element to <exists>
     *
     * @param string $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function addExists( $value){
     return $this->_add(6, $value);
    }
    
    /**
     * Check if <members> has a value
     *
     * @return boolean
     */
    public function hasMembers(){
      return $this->_has(7);
    }
    
    /**
     * Clear <members> value
     *
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function clearMembers(){
      return $this->_clear(7);
    }
    
    /**
     * Get <members> value
     *
     * @param int $idx
     * @return int
     */
    public function getMembers($idx = NULL){
      return $this->_get(7, $idx);
    }
    
    /**
     * Set <members> value
     *
     * @param int $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function setMembers( $value, $idx = NULL){
      return $this->_set(7, $value, $idx);
    }
    
    /**
     * Get all elements of <members>
     *
     * @return int[]
     */
    public function getMembersList(){
     return $this->_get(7);
    }
    
    /**
     * Add a new element to <members>
     *
     * @param int $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function addMembers( $value){
     return $this->_add(7, $value);
    }
    
    /**
     * Check if <results> has a value
     *
     * @return boolean
     */
    public function hasResults(){
      return $this->_has(8);
    }
    
    /**
     * Clear <results> value
     *
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function clearResults(){
      return $this->_clear(8);
    }
    
    /**
     * Get <results> value
     *
     * @param int $idx
     * @return \RO\Cmd\PhotoFrame
     */
    public function getResults($idx = NULL){
      return $this->_get(8, $idx);
    }
    
    /**
     * Set <results> value
     *
     * @param \RO\Cmd\PhotoFrame $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function setResults(\RO\Cmd\PhotoFrame $value, $idx = NULL){
      return $this->_set(8, $value, $idx);
    }
    
    /**
     * Get all elements of <results>
     *
     * @return \RO\Cmd\PhotoFrame[]
     */
    public function getResultsList(){
     return $this->_get(8);
    }
    
    /**
     * Add a new element to <results>
     *
     * @param \RO\Cmd\PhotoFrame $value
     * @return \RO\Cmd\QueryShowPhotoGuildSCmd
     */
    public function addResults(\RO\Cmd\PhotoFrame $value){
     return $this->_add(8, $value);
    }
  }
}


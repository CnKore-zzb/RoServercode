<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: GuildSCmd.proto

namespace RO\Cmd {

  class GuildCityActionGuildSCmd extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\Command */
    public $cmd = \RO\Cmd\Command::GUILD_PROTOCMD;
    
    /**  @var int - \RO\Cmd\GuildSParam */
    public $param = \RO\Cmd\GuildSParam::GUILDSPARAM_GUILD_CITY_ACTION;
    
    /**  @var int - \RO\Cmd\EGuildCityAction */
    public $action = \RO\Cmd\EGuildCityAction::EGUILDCITYACTION_MIN;
    
    /**  @var int - \RO\Cmd\EGuildCityStatus */
    public $status = \RO\Cmd\EGuildCityStatus::EGUILDCITYSTATUS_MIN;
    
    /**  @var int - \RO\Cmd\EGuildCityResult */
    public $result = \RO\Cmd\EGuildCityResult::EGUILDCITYRESULT_MIN;
    
    /**  @var int */
    public $zoneid = 0;
    
    /**  @var string */
    public $scenename = null;
    
    /**  @var \RO\Cmd\GuildCityInfo[]  */
    public $infos = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.GuildCityActionGuildSCmd');

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
      $f->default   = \RO\Cmd\GuildSParam::GUILDSPARAM_GUILD_CITY_ACTION;
      $descriptor->addField($f);

      // OPTIONAL ENUM action = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "action";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EGuildCityAction';
      $f->default   = \RO\Cmd\EGuildCityAction::EGUILDCITYACTION_MIN;
      $descriptor->addField($f);

      // OPTIONAL ENUM status = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "status";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EGuildCityStatus';
      $f->default   = \RO\Cmd\EGuildCityStatus::EGUILDCITYSTATUS_MIN;
      $descriptor->addField($f);

      // OPTIONAL ENUM result = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "result";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EGuildCityResult';
      $f->default   = \RO\Cmd\EGuildCityResult::EGUILDCITYRESULT_MIN;
      $descriptor->addField($f);

      // OPTIONAL UINT32 zoneid = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "zoneid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL STRING scenename = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "scenename";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // REPEATED MESSAGE infos = 8
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 8;
      $f->name      = "infos";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\GuildCityInfo';
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
     * @return \RO\Cmd\GuildCityActionGuildSCmd
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
     * @return \RO\Cmd\GuildCityActionGuildSCmd
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
     * @return \RO\Cmd\GuildCityActionGuildSCmd
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
     * @return \RO\Cmd\GuildCityActionGuildSCmd
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
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function clearAction(){
      return $this->_clear(3);
    }
    
    /**
     * Get <action> value
     *
     * @return int - \RO\Cmd\EGuildCityAction
     */
    public function getAction(){
      return $this->_get(3);
    }
    
    /**
     * Set <action> value
     *
     * @param int - \RO\Cmd\EGuildCityAction $value
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function setAction( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <status> has a value
     *
     * @return boolean
     */
    public function hasStatus(){
      return $this->_has(4);
    }
    
    /**
     * Clear <status> value
     *
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function clearStatus(){
      return $this->_clear(4);
    }
    
    /**
     * Get <status> value
     *
     * @return int - \RO\Cmd\EGuildCityStatus
     */
    public function getStatus(){
      return $this->_get(4);
    }
    
    /**
     * Set <status> value
     *
     * @param int - \RO\Cmd\EGuildCityStatus $value
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function setStatus( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <result> has a value
     *
     * @return boolean
     */
    public function hasResult(){
      return $this->_has(5);
    }
    
    /**
     * Clear <result> value
     *
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function clearResult(){
      return $this->_clear(5);
    }
    
    /**
     * Get <result> value
     *
     * @return int - \RO\Cmd\EGuildCityResult
     */
    public function getResult(){
      return $this->_get(5);
    }
    
    /**
     * Set <result> value
     *
     * @param int - \RO\Cmd\EGuildCityResult $value
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function setResult( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <zoneid> has a value
     *
     * @return boolean
     */
    public function hasZoneid(){
      return $this->_has(6);
    }
    
    /**
     * Clear <zoneid> value
     *
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function clearZoneid(){
      return $this->_clear(6);
    }
    
    /**
     * Get <zoneid> value
     *
     * @return int
     */
    public function getZoneid(){
      return $this->_get(6);
    }
    
    /**
     * Set <zoneid> value
     *
     * @param int $value
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function setZoneid( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <scenename> has a value
     *
     * @return boolean
     */
    public function hasScenename(){
      return $this->_has(7);
    }
    
    /**
     * Clear <scenename> value
     *
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function clearScenename(){
      return $this->_clear(7);
    }
    
    /**
     * Get <scenename> value
     *
     * @return string
     */
    public function getScenename(){
      return $this->_get(7);
    }
    
    /**
     * Set <scenename> value
     *
     * @param string $value
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function setScenename( $value){
      return $this->_set(7, $value);
    }
    
    /**
     * Check if <infos> has a value
     *
     * @return boolean
     */
    public function hasInfos(){
      return $this->_has(8);
    }
    
    /**
     * Clear <infos> value
     *
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function clearInfos(){
      return $this->_clear(8);
    }
    
    /**
     * Get <infos> value
     *
     * @param int $idx
     * @return \RO\Cmd\GuildCityInfo
     */
    public function getInfos($idx = NULL){
      return $this->_get(8, $idx);
    }
    
    /**
     * Set <infos> value
     *
     * @param \RO\Cmd\GuildCityInfo $value
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function setInfos(\RO\Cmd\GuildCityInfo $value, $idx = NULL){
      return $this->_set(8, $value, $idx);
    }
    
    /**
     * Get all elements of <infos>
     *
     * @return \RO\Cmd\GuildCityInfo[]
     */
    public function getInfosList(){
     return $this->_get(8);
    }
    
    /**
     * Add a new element to <infos>
     *
     * @param \RO\Cmd\GuildCityInfo $value
     * @return \RO\Cmd\GuildCityActionGuildSCmd
     */
    public function addInfos(\RO\Cmd\GuildCityInfo $value){
     return $this->_add(8, $value);
    }
  }
}

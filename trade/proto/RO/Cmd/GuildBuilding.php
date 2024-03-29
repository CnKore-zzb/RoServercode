<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: GuildCmd.proto

namespace RO\Cmd {

  class GuildBuilding extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\EGuildBuilding */
    public $type = \RO\Cmd\EGuildBuilding::EGUILDBUILDING_MIN;
    
    /**  @var int */
    public $level = 0;
    
    /**  @var \RO\Cmd\GuildBuildMaterial[]  */
    public $materials = array();
    
    /**  @var boolean */
    public $isbuilding = false;
    
    /**  @var int */
    public $nextwelfaretime = 0;
    
    /**  @var int */
    public $nextbuildtime = 0;
    
    /**  @var int */
    public $progress = 0;
    
    /**  @var \RO\Cmd\GuildBuildMaterial[]  */
    public $restmaterials = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.GuildBuilding');

      // OPTIONAL ENUM type = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "type";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EGuildBuilding';
      $f->default   = \RO\Cmd\EGuildBuilding::EGUILDBUILDING_MIN;
      $descriptor->addField($f);

      // OPTIONAL UINT32 level = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "level";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // REPEATED MESSAGE materials = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "materials";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\GuildBuildMaterial';
      $descriptor->addField($f);

      // OPTIONAL BOOL isbuilding = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "isbuilding";
      $f->type      = \DrSlump\Protobuf::TYPE_BOOL;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = false;
      $descriptor->addField($f);

      // OPTIONAL UINT32 nextwelfaretime = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "nextwelfaretime";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 nextbuildtime = 8
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 8;
      $f->name      = "nextbuildtime";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 progress = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "progress";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // REPEATED MESSAGE restmaterials = 7
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 7;
      $f->name      = "restmaterials";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\GuildBuildMaterial';
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <type> has a value
     *
     * @return boolean
     */
    public function hasType(){
      return $this->_has(1);
    }
    
    /**
     * Clear <type> value
     *
     * @return \RO\Cmd\GuildBuilding
     */
    public function clearType(){
      return $this->_clear(1);
    }
    
    /**
     * Get <type> value
     *
     * @return int - \RO\Cmd\EGuildBuilding
     */
    public function getType(){
      return $this->_get(1);
    }
    
    /**
     * Set <type> value
     *
     * @param int - \RO\Cmd\EGuildBuilding $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function setType( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <level> has a value
     *
     * @return boolean
     */
    public function hasLevel(){
      return $this->_has(2);
    }
    
    /**
     * Clear <level> value
     *
     * @return \RO\Cmd\GuildBuilding
     */
    public function clearLevel(){
      return $this->_clear(2);
    }
    
    /**
     * Get <level> value
     *
     * @return int
     */
    public function getLevel(){
      return $this->_get(2);
    }
    
    /**
     * Set <level> value
     *
     * @param int $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function setLevel( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <materials> has a value
     *
     * @return boolean
     */
    public function hasMaterials(){
      return $this->_has(3);
    }
    
    /**
     * Clear <materials> value
     *
     * @return \RO\Cmd\GuildBuilding
     */
    public function clearMaterials(){
      return $this->_clear(3);
    }
    
    /**
     * Get <materials> value
     *
     * @param int $idx
     * @return \RO\Cmd\GuildBuildMaterial
     */
    public function getMaterials($idx = NULL){
      return $this->_get(3, $idx);
    }
    
    /**
     * Set <materials> value
     *
     * @param \RO\Cmd\GuildBuildMaterial $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function setMaterials(\RO\Cmd\GuildBuildMaterial $value, $idx = NULL){
      return $this->_set(3, $value, $idx);
    }
    
    /**
     * Get all elements of <materials>
     *
     * @return \RO\Cmd\GuildBuildMaterial[]
     */
    public function getMaterialsList(){
     return $this->_get(3);
    }
    
    /**
     * Add a new element to <materials>
     *
     * @param \RO\Cmd\GuildBuildMaterial $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function addMaterials(\RO\Cmd\GuildBuildMaterial $value){
     return $this->_add(3, $value);
    }
    
    /**
     * Check if <isbuilding> has a value
     *
     * @return boolean
     */
    public function hasIsbuilding(){
      return $this->_has(4);
    }
    
    /**
     * Clear <isbuilding> value
     *
     * @return \RO\Cmd\GuildBuilding
     */
    public function clearIsbuilding(){
      return $this->_clear(4);
    }
    
    /**
     * Get <isbuilding> value
     *
     * @return boolean
     */
    public function getIsbuilding(){
      return $this->_get(4);
    }
    
    /**
     * Set <isbuilding> value
     *
     * @param boolean $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function setIsbuilding( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <nextwelfaretime> has a value
     *
     * @return boolean
     */
    public function hasNextwelfaretime(){
      return $this->_has(5);
    }
    
    /**
     * Clear <nextwelfaretime> value
     *
     * @return \RO\Cmd\GuildBuilding
     */
    public function clearNextwelfaretime(){
      return $this->_clear(5);
    }
    
    /**
     * Get <nextwelfaretime> value
     *
     * @return int
     */
    public function getNextwelfaretime(){
      return $this->_get(5);
    }
    
    /**
     * Set <nextwelfaretime> value
     *
     * @param int $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function setNextwelfaretime( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <nextbuildtime> has a value
     *
     * @return boolean
     */
    public function hasNextbuildtime(){
      return $this->_has(8);
    }
    
    /**
     * Clear <nextbuildtime> value
     *
     * @return \RO\Cmd\GuildBuilding
     */
    public function clearNextbuildtime(){
      return $this->_clear(8);
    }
    
    /**
     * Get <nextbuildtime> value
     *
     * @return int
     */
    public function getNextbuildtime(){
      return $this->_get(8);
    }
    
    /**
     * Set <nextbuildtime> value
     *
     * @param int $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function setNextbuildtime( $value){
      return $this->_set(8, $value);
    }
    
    /**
     * Check if <progress> has a value
     *
     * @return boolean
     */
    public function hasProgress(){
      return $this->_has(6);
    }
    
    /**
     * Clear <progress> value
     *
     * @return \RO\Cmd\GuildBuilding
     */
    public function clearProgress(){
      return $this->_clear(6);
    }
    
    /**
     * Get <progress> value
     *
     * @return int
     */
    public function getProgress(){
      return $this->_get(6);
    }
    
    /**
     * Set <progress> value
     *
     * @param int $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function setProgress( $value){
      return $this->_set(6, $value);
    }
    
    /**
     * Check if <restmaterials> has a value
     *
     * @return boolean
     */
    public function hasRestmaterials(){
      return $this->_has(7);
    }
    
    /**
     * Clear <restmaterials> value
     *
     * @return \RO\Cmd\GuildBuilding
     */
    public function clearRestmaterials(){
      return $this->_clear(7);
    }
    
    /**
     * Get <restmaterials> value
     *
     * @param int $idx
     * @return \RO\Cmd\GuildBuildMaterial
     */
    public function getRestmaterials($idx = NULL){
      return $this->_get(7, $idx);
    }
    
    /**
     * Set <restmaterials> value
     *
     * @param \RO\Cmd\GuildBuildMaterial $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function setRestmaterials(\RO\Cmd\GuildBuildMaterial $value, $idx = NULL){
      return $this->_set(7, $value, $idx);
    }
    
    /**
     * Get all elements of <restmaterials>
     *
     * @return \RO\Cmd\GuildBuildMaterial[]
     */
    public function getRestmaterialsList(){
     return $this->_get(7);
    }
    
    /**
     * Add a new element to <restmaterials>
     *
     * @param \RO\Cmd\GuildBuildMaterial $value
     * @return \RO\Cmd\GuildBuilding
     */
    public function addRestmaterials(\RO\Cmd\GuildBuildMaterial $value){
     return $this->_add(7, $value);
    }
  }
}


<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordCmd.proto

namespace RO\Cmd {

  class BlobOption extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\EQueryType */
    public $type = \RO\Cmd\EQueryType::EQUERYTYPE_MIN;
    
    /**  @var int */
    public $normalskill_option = 1;
    
    /**  @var int */
    public $fashionhide = 0;
    
    /**  @var int */
    public $bitopt = 18446744073709551615;
    
    /**  @var \RO\Cmd\SkillOption[]  */
    public $skillopts = array();
    
    /**  @var int - \RO\Cmd\EQueryType */
    public $wedding_type = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.BlobOption');

      // OPTIONAL ENUM type = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "type";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EQueryType';
      $f->default   = \RO\Cmd\EQueryType::EQUERYTYPE_MIN;
      $descriptor->addField($f);

      // OPTIONAL UINT32 normalskill_option = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "normalskill_option";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 1;
      $descriptor->addField($f);

      // OPTIONAL UINT32 fashionhide = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "fashionhide";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT64 bitopt = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "bitopt";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 18446744073709551615;
      $descriptor->addField($f);

      // REPEATED MESSAGE skillopts = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "skillopts";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\SkillOption';
      $descriptor->addField($f);

      // OPTIONAL ENUM wedding_type = 6
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 6;
      $f->name      = "wedding_type";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EQueryType';
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
     * @return \RO\Cmd\BlobOption
     */
    public function clearType(){
      return $this->_clear(1);
    }
    
    /**
     * Get <type> value
     *
     * @return int - \RO\Cmd\EQueryType
     */
    public function getType(){
      return $this->_get(1);
    }
    
    /**
     * Set <type> value
     *
     * @param int - \RO\Cmd\EQueryType $value
     * @return \RO\Cmd\BlobOption
     */
    public function setType( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <normalskill_option> has a value
     *
     * @return boolean
     */
    public function hasNormalskillOption(){
      return $this->_has(2);
    }
    
    /**
     * Clear <normalskill_option> value
     *
     * @return \RO\Cmd\BlobOption
     */
    public function clearNormalskillOption(){
      return $this->_clear(2);
    }
    
    /**
     * Get <normalskill_option> value
     *
     * @return int
     */
    public function getNormalskillOption(){
      return $this->_get(2);
    }
    
    /**
     * Set <normalskill_option> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobOption
     */
    public function setNormalskillOption( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <fashionhide> has a value
     *
     * @return boolean
     */
    public function hasFashionhide(){
      return $this->_has(3);
    }
    
    /**
     * Clear <fashionhide> value
     *
     * @return \RO\Cmd\BlobOption
     */
    public function clearFashionhide(){
      return $this->_clear(3);
    }
    
    /**
     * Get <fashionhide> value
     *
     * @return int
     */
    public function getFashionhide(){
      return $this->_get(3);
    }
    
    /**
     * Set <fashionhide> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobOption
     */
    public function setFashionhide( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <bitopt> has a value
     *
     * @return boolean
     */
    public function hasBitopt(){
      return $this->_has(4);
    }
    
    /**
     * Clear <bitopt> value
     *
     * @return \RO\Cmd\BlobOption
     */
    public function clearBitopt(){
      return $this->_clear(4);
    }
    
    /**
     * Get <bitopt> value
     *
     * @return int
     */
    public function getBitopt(){
      return $this->_get(4);
    }
    
    /**
     * Set <bitopt> value
     *
     * @param int $value
     * @return \RO\Cmd\BlobOption
     */
    public function setBitopt( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <skillopts> has a value
     *
     * @return boolean
     */
    public function hasSkillopts(){
      return $this->_has(5);
    }
    
    /**
     * Clear <skillopts> value
     *
     * @return \RO\Cmd\BlobOption
     */
    public function clearSkillopts(){
      return $this->_clear(5);
    }
    
    /**
     * Get <skillopts> value
     *
     * @param int $idx
     * @return \RO\Cmd\SkillOption
     */
    public function getSkillopts($idx = NULL){
      return $this->_get(5, $idx);
    }
    
    /**
     * Set <skillopts> value
     *
     * @param \RO\Cmd\SkillOption $value
     * @return \RO\Cmd\BlobOption
     */
    public function setSkillopts(\RO\Cmd\SkillOption $value, $idx = NULL){
      return $this->_set(5, $value, $idx);
    }
    
    /**
     * Get all elements of <skillopts>
     *
     * @return \RO\Cmd\SkillOption[]
     */
    public function getSkilloptsList(){
     return $this->_get(5);
    }
    
    /**
     * Add a new element to <skillopts>
     *
     * @param \RO\Cmd\SkillOption $value
     * @return \RO\Cmd\BlobOption
     */
    public function addSkillopts(\RO\Cmd\SkillOption $value){
     return $this->_add(5, $value);
    }
    
    /**
     * Check if <wedding_type> has a value
     *
     * @return boolean
     */
    public function hasWeddingType(){
      return $this->_has(6);
    }
    
    /**
     * Clear <wedding_type> value
     *
     * @return \RO\Cmd\BlobOption
     */
    public function clearWeddingType(){
      return $this->_clear(6);
    }
    
    /**
     * Get <wedding_type> value
     *
     * @return int - \RO\Cmd\EQueryType
     */
    public function getWeddingType(){
      return $this->_get(6);
    }
    
    /**
     * Set <wedding_type> value
     *
     * @param int - \RO\Cmd\EQueryType $value
     * @return \RO\Cmd\BlobOption
     */
    public function setWeddingType( $value){
      return $this->_set(6, $value);
    }
  }
}

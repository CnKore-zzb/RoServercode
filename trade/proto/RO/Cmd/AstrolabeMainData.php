<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordCmd.proto

namespace RO\Cmd {

  class AstrolabeMainData extends \DrSlump\Protobuf\Message {

    /**  @var int - \RO\Cmd\EAstrolabeType */
    public $type = null;
    
    /**  @var \RO\Cmd\AstrolabeData[]  */
    public $astrolabes = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.AstrolabeMainData');

      // OPTIONAL ENUM type = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "type";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EAstrolabeType';
      $descriptor->addField($f);

      // REPEATED MESSAGE astrolabes = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "astrolabes";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\AstrolabeData';
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
     * @return \RO\Cmd\AstrolabeMainData
     */
    public function clearType(){
      return $this->_clear(1);
    }
    
    /**
     * Get <type> value
     *
     * @return int - \RO\Cmd\EAstrolabeType
     */
    public function getType(){
      return $this->_get(1);
    }
    
    /**
     * Set <type> value
     *
     * @param int - \RO\Cmd\EAstrolabeType $value
     * @return \RO\Cmd\AstrolabeMainData
     */
    public function setType( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <astrolabes> has a value
     *
     * @return boolean
     */
    public function hasAstrolabes(){
      return $this->_has(2);
    }
    
    /**
     * Clear <astrolabes> value
     *
     * @return \RO\Cmd\AstrolabeMainData
     */
    public function clearAstrolabes(){
      return $this->_clear(2);
    }
    
    /**
     * Get <astrolabes> value
     *
     * @param int $idx
     * @return \RO\Cmd\AstrolabeData
     */
    public function getAstrolabes($idx = NULL){
      return $this->_get(2, $idx);
    }
    
    /**
     * Set <astrolabes> value
     *
     * @param \RO\Cmd\AstrolabeData $value
     * @return \RO\Cmd\AstrolabeMainData
     */
    public function setAstrolabes(\RO\Cmd\AstrolabeData $value, $idx = NULL){
      return $this->_set(2, $value, $idx);
    }
    
    /**
     * Get all elements of <astrolabes>
     *
     * @return \RO\Cmd\AstrolabeData[]
     */
    public function getAstrolabesList(){
     return $this->_get(2);
    }
    
    /**
     * Add a new element to <astrolabes>
     *
     * @param \RO\Cmd\AstrolabeData $value
     * @return \RO\Cmd\AstrolabeMainData
     */
    public function addAstrolabes(\RO\Cmd\AstrolabeData $value){
     return $this->_add(2, $value);
    }
  }
}


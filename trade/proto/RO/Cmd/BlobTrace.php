<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: RecordCmd.proto

namespace RO\Cmd {

  class BlobTrace extends \DrSlump\Protobuf\Message {

    /**  @var \RO\Cmd\TraceItem[]  */
    public $items = array();
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.BlobTrace');

      // REPEATED MESSAGE items = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "items";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_REPEATED;
      $f->reference = '\RO\Cmd\TraceItem';
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <items> has a value
     *
     * @return boolean
     */
    public function hasItems(){
      return $this->_has(1);
    }
    
    /**
     * Clear <items> value
     *
     * @return \RO\Cmd\BlobTrace
     */
    public function clearItems(){
      return $this->_clear(1);
    }
    
    /**
     * Get <items> value
     *
     * @param int $idx
     * @return \RO\Cmd\TraceItem
     */
    public function getItems($idx = NULL){
      return $this->_get(1, $idx);
    }
    
    /**
     * Set <items> value
     *
     * @param \RO\Cmd\TraceItem $value
     * @return \RO\Cmd\BlobTrace
     */
    public function setItems(\RO\Cmd\TraceItem $value, $idx = NULL){
      return $this->_set(1, $value, $idx);
    }
    
    /**
     * Get all elements of <items>
     *
     * @return \RO\Cmd\TraceItem[]
     */
    public function getItemsList(){
     return $this->_get(1);
    }
    
    /**
     * Add a new element to <items>
     *
     * @param \RO\Cmd\TraceItem $value
     * @return \RO\Cmd\BlobTrace
     */
    public function addItems(\RO\Cmd\TraceItem $value){
     return $this->_add(1, $value);
    }
  }
}

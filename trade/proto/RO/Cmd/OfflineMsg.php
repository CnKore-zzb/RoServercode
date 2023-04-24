<?php
// DO NOT EDIT! Generated by Protobuf-PHP protoc plugin @package_version@
// Source: ChatCmd.proto

namespace RO\Cmd {

  class OfflineMsg extends \DrSlump\Protobuf\Message {

    /**  @var int */
    public $targetid = 0;
    
    /**  @var int */
    public $senderid = 0;
    
    /**  @var int */
    public $time = 0;
    
    /**  @var int - \RO\Cmd\EOfflineMsg */
    public $type = \RO\Cmd\EOfflineMsg::EOFFLINEMSG_MIN;
    
    /**  @var string */
    public $sendername = null;
    
    /**  @var \RO\Cmd\ChatRetCmd */
    public $chat = null;
    
    /**  @var int */
    public $itemid = 0;
    
    /**  @var int */
    public $price = 0;
    
    /**  @var int */
    public $count = 0;
    
    /**  @var int */
    public $givemoney = 0;
    
    /**  @var int - \RO\Cmd\EMoneyType */
    public $moneytype = \RO\Cmd\EMoneyType::EMONEYTYPE_MIN;
    
    /**  @var string */
    public $sysstr = null;
    
    /**  @var string */
    public $gmcmd = null;
    
    /**  @var int */
    public $id = 0;
    
    /**  @var string */
    public $msg = null;
    
    /**  @var \RO\Cmd\ItemData */
    public $itemdata = null;
    
    /**  @var \RO\Cmd\SysMsg */
    public $syscmd = null;
    
    /**  @var \RO\Cmd\TutorReward */
    public $tutorreward = null;
    
    /**  @var \RO\Cmd\OffMsgUserAddItem */
    public $useradditem = null;
    
    /**  @var \RO\Cmd\WeddingEventMsgCCmd */
    public $weddingmsg = null;
    

    /** @var \Closure[] */
    protected static $__extensions = array();

    public static function descriptor()
    {
      $descriptor = new \DrSlump\Protobuf\Descriptor(__CLASS__, 'Cmd.OfflineMsg');

      // OPTIONAL UINT64 targetid = 1
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 1;
      $f->name      = "targetid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT64 senderid = 2
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 2;
      $f->name      = "senderid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 time = 3
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 3;
      $f->name      = "time";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL ENUM type = 4
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 4;
      $f->name      = "type";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EOfflineMsg';
      $f->default   = \RO\Cmd\EOfflineMsg::EOFFLINEMSG_MIN;
      $descriptor->addField($f);

      // OPTIONAL STRING sendername = 5
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 5;
      $f->name      = "sendername";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL MESSAGE chat = 19
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 19;
      $f->name      = "chat";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ChatRetCmd';
      $descriptor->addField($f);

      // OPTIONAL UINT32 itemid = 11
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 11;
      $f->name      = "itemid";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 price = 12
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 12;
      $f->name      = "price";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 count = 13
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 13;
      $f->name      = "count";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL UINT32 givemoney = 14
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 14;
      $f->name      = "givemoney";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT32;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL ENUM moneytype = 15
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 15;
      $f->name      = "moneytype";
      $f->type      = \DrSlump\Protobuf::TYPE_ENUM;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\EMoneyType';
      $f->default   = \RO\Cmd\EMoneyType::EMONEYTYPE_MIN;
      $descriptor->addField($f);

      // OPTIONAL STRING sysstr = 20
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 20;
      $f->name      = "sysstr";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL STRING gmcmd = 16
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 16;
      $f->name      = "gmcmd";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL UINT64 id = 17
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 17;
      $f->name      = "id";
      $f->type      = \DrSlump\Protobuf::TYPE_UINT64;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->default   = 0;
      $descriptor->addField($f);

      // OPTIONAL STRING msg = 18
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 18;
      $f->name      = "msg";
      $f->type      = \DrSlump\Protobuf::TYPE_STRING;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $descriptor->addField($f);

      // OPTIONAL MESSAGE itemdata = 22
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 22;
      $f->name      = "itemdata";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\ItemData';
      $descriptor->addField($f);

      // OPTIONAL MESSAGE syscmd = 21
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 21;
      $f->name      = "syscmd";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\SysMsg';
      $descriptor->addField($f);

      // OPTIONAL MESSAGE tutorreward = 23
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 23;
      $f->name      = "tutorreward";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\TutorReward';
      $descriptor->addField($f);

      // OPTIONAL MESSAGE useradditem = 24
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 24;
      $f->name      = "useradditem";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\OffMsgUserAddItem';
      $descriptor->addField($f);

      // OPTIONAL MESSAGE weddingmsg = 25
      $f = new \DrSlump\Protobuf\Field();
      $f->number    = 25;
      $f->name      = "weddingmsg";
      $f->type      = \DrSlump\Protobuf::TYPE_MESSAGE;
      $f->rule      = \DrSlump\Protobuf::RULE_OPTIONAL;
      $f->reference = '\RO\Cmd\WeddingEventMsgCCmd';
      $descriptor->addField($f);

      foreach (self::$__extensions as $cb) {
        $descriptor->addField($cb(), true);
      }

      return $descriptor;
    }

    /**
     * Check if <targetid> has a value
     *
     * @return boolean
     */
    public function hasTargetid(){
      return $this->_has(1);
    }
    
    /**
     * Clear <targetid> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearTargetid(){
      return $this->_clear(1);
    }
    
    /**
     * Get <targetid> value
     *
     * @return int
     */
    public function getTargetid(){
      return $this->_get(1);
    }
    
    /**
     * Set <targetid> value
     *
     * @param int $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setTargetid( $value){
      return $this->_set(1, $value);
    }
    
    /**
     * Check if <senderid> has a value
     *
     * @return boolean
     */
    public function hasSenderid(){
      return $this->_has(2);
    }
    
    /**
     * Clear <senderid> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearSenderid(){
      return $this->_clear(2);
    }
    
    /**
     * Get <senderid> value
     *
     * @return int
     */
    public function getSenderid(){
      return $this->_get(2);
    }
    
    /**
     * Set <senderid> value
     *
     * @param int $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setSenderid( $value){
      return $this->_set(2, $value);
    }
    
    /**
     * Check if <time> has a value
     *
     * @return boolean
     */
    public function hasTime(){
      return $this->_has(3);
    }
    
    /**
     * Clear <time> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearTime(){
      return $this->_clear(3);
    }
    
    /**
     * Get <time> value
     *
     * @return int
     */
    public function getTime(){
      return $this->_get(3);
    }
    
    /**
     * Set <time> value
     *
     * @param int $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setTime( $value){
      return $this->_set(3, $value);
    }
    
    /**
     * Check if <type> has a value
     *
     * @return boolean
     */
    public function hasType(){
      return $this->_has(4);
    }
    
    /**
     * Clear <type> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearType(){
      return $this->_clear(4);
    }
    
    /**
     * Get <type> value
     *
     * @return int - \RO\Cmd\EOfflineMsg
     */
    public function getType(){
      return $this->_get(4);
    }
    
    /**
     * Set <type> value
     *
     * @param int - \RO\Cmd\EOfflineMsg $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setType( $value){
      return $this->_set(4, $value);
    }
    
    /**
     * Check if <sendername> has a value
     *
     * @return boolean
     */
    public function hasSendername(){
      return $this->_has(5);
    }
    
    /**
     * Clear <sendername> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearSendername(){
      return $this->_clear(5);
    }
    
    /**
     * Get <sendername> value
     *
     * @return string
     */
    public function getSendername(){
      return $this->_get(5);
    }
    
    /**
     * Set <sendername> value
     *
     * @param string $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setSendername( $value){
      return $this->_set(5, $value);
    }
    
    /**
     * Check if <chat> has a value
     *
     * @return boolean
     */
    public function hasChat(){
      return $this->_has(19);
    }
    
    /**
     * Clear <chat> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearChat(){
      return $this->_clear(19);
    }
    
    /**
     * Get <chat> value
     *
     * @return \RO\Cmd\ChatRetCmd
     */
    public function getChat(){
      return $this->_get(19);
    }
    
    /**
     * Set <chat> value
     *
     * @param \RO\Cmd\ChatRetCmd $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setChat(\RO\Cmd\ChatRetCmd $value){
      return $this->_set(19, $value);
    }
    
    /**
     * Check if <itemid> has a value
     *
     * @return boolean
     */
    public function hasItemid(){
      return $this->_has(11);
    }
    
    /**
     * Clear <itemid> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearItemid(){
      return $this->_clear(11);
    }
    
    /**
     * Get <itemid> value
     *
     * @return int
     */
    public function getItemid(){
      return $this->_get(11);
    }
    
    /**
     * Set <itemid> value
     *
     * @param int $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setItemid( $value){
      return $this->_set(11, $value);
    }
    
    /**
     * Check if <price> has a value
     *
     * @return boolean
     */
    public function hasPrice(){
      return $this->_has(12);
    }
    
    /**
     * Clear <price> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearPrice(){
      return $this->_clear(12);
    }
    
    /**
     * Get <price> value
     *
     * @return int
     */
    public function getPrice(){
      return $this->_get(12);
    }
    
    /**
     * Set <price> value
     *
     * @param int $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setPrice( $value){
      return $this->_set(12, $value);
    }
    
    /**
     * Check if <count> has a value
     *
     * @return boolean
     */
    public function hasCount(){
      return $this->_has(13);
    }
    
    /**
     * Clear <count> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearCount(){
      return $this->_clear(13);
    }
    
    /**
     * Get <count> value
     *
     * @return int
     */
    public function getCount(){
      return $this->_get(13);
    }
    
    /**
     * Set <count> value
     *
     * @param int $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setCount( $value){
      return $this->_set(13, $value);
    }
    
    /**
     * Check if <givemoney> has a value
     *
     * @return boolean
     */
    public function hasGivemoney(){
      return $this->_has(14);
    }
    
    /**
     * Clear <givemoney> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearGivemoney(){
      return $this->_clear(14);
    }
    
    /**
     * Get <givemoney> value
     *
     * @return int
     */
    public function getGivemoney(){
      return $this->_get(14);
    }
    
    /**
     * Set <givemoney> value
     *
     * @param int $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setGivemoney( $value){
      return $this->_set(14, $value);
    }
    
    /**
     * Check if <moneytype> has a value
     *
     * @return boolean
     */
    public function hasMoneytype(){
      return $this->_has(15);
    }
    
    /**
     * Clear <moneytype> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearMoneytype(){
      return $this->_clear(15);
    }
    
    /**
     * Get <moneytype> value
     *
     * @return int - \RO\Cmd\EMoneyType
     */
    public function getMoneytype(){
      return $this->_get(15);
    }
    
    /**
     * Set <moneytype> value
     *
     * @param int - \RO\Cmd\EMoneyType $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setMoneytype( $value){
      return $this->_set(15, $value);
    }
    
    /**
     * Check if <sysstr> has a value
     *
     * @return boolean
     */
    public function hasSysstr(){
      return $this->_has(20);
    }
    
    /**
     * Clear <sysstr> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearSysstr(){
      return $this->_clear(20);
    }
    
    /**
     * Get <sysstr> value
     *
     * @return string
     */
    public function getSysstr(){
      return $this->_get(20);
    }
    
    /**
     * Set <sysstr> value
     *
     * @param string $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setSysstr( $value){
      return $this->_set(20, $value);
    }
    
    /**
     * Check if <gmcmd> has a value
     *
     * @return boolean
     */
    public function hasGmcmd(){
      return $this->_has(16);
    }
    
    /**
     * Clear <gmcmd> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearGmcmd(){
      return $this->_clear(16);
    }
    
    /**
     * Get <gmcmd> value
     *
     * @return string
     */
    public function getGmcmd(){
      return $this->_get(16);
    }
    
    /**
     * Set <gmcmd> value
     *
     * @param string $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setGmcmd( $value){
      return $this->_set(16, $value);
    }
    
    /**
     * Check if <id> has a value
     *
     * @return boolean
     */
    public function hasId(){
      return $this->_has(17);
    }
    
    /**
     * Clear <id> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearId(){
      return $this->_clear(17);
    }
    
    /**
     * Get <id> value
     *
     * @return int
     */
    public function getId(){
      return $this->_get(17);
    }
    
    /**
     * Set <id> value
     *
     * @param int $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setId( $value){
      return $this->_set(17, $value);
    }
    
    /**
     * Check if <msg> has a value
     *
     * @return boolean
     */
    public function hasMsg(){
      return $this->_has(18);
    }
    
    /**
     * Clear <msg> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearMsg(){
      return $this->_clear(18);
    }
    
    /**
     * Get <msg> value
     *
     * @return string
     */
    public function getMsg(){
      return $this->_get(18);
    }
    
    /**
     * Set <msg> value
     *
     * @param string $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setMsg( $value){
      return $this->_set(18, $value);
    }
    
    /**
     * Check if <itemdata> has a value
     *
     * @return boolean
     */
    public function hasItemdata(){
      return $this->_has(22);
    }
    
    /**
     * Clear <itemdata> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearItemdata(){
      return $this->_clear(22);
    }
    
    /**
     * Get <itemdata> value
     *
     * @return \RO\Cmd\ItemData
     */
    public function getItemdata(){
      return $this->_get(22);
    }
    
    /**
     * Set <itemdata> value
     *
     * @param \RO\Cmd\ItemData $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setItemdata(\RO\Cmd\ItemData $value){
      return $this->_set(22, $value);
    }
    
    /**
     * Check if <syscmd> has a value
     *
     * @return boolean
     */
    public function hasSyscmd(){
      return $this->_has(21);
    }
    
    /**
     * Clear <syscmd> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearSyscmd(){
      return $this->_clear(21);
    }
    
    /**
     * Get <syscmd> value
     *
     * @return \RO\Cmd\SysMsg
     */
    public function getSyscmd(){
      return $this->_get(21);
    }
    
    /**
     * Set <syscmd> value
     *
     * @param \RO\Cmd\SysMsg $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setSyscmd(\RO\Cmd\SysMsg $value){
      return $this->_set(21, $value);
    }
    
    /**
     * Check if <tutorreward> has a value
     *
     * @return boolean
     */
    public function hasTutorreward(){
      return $this->_has(23);
    }
    
    /**
     * Clear <tutorreward> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearTutorreward(){
      return $this->_clear(23);
    }
    
    /**
     * Get <tutorreward> value
     *
     * @return \RO\Cmd\TutorReward
     */
    public function getTutorreward(){
      return $this->_get(23);
    }
    
    /**
     * Set <tutorreward> value
     *
     * @param \RO\Cmd\TutorReward $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setTutorreward(\RO\Cmd\TutorReward $value){
      return $this->_set(23, $value);
    }
    
    /**
     * Check if <useradditem> has a value
     *
     * @return boolean
     */
    public function hasUseradditem(){
      return $this->_has(24);
    }
    
    /**
     * Clear <useradditem> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearUseradditem(){
      return $this->_clear(24);
    }
    
    /**
     * Get <useradditem> value
     *
     * @return \RO\Cmd\OffMsgUserAddItem
     */
    public function getUseradditem(){
      return $this->_get(24);
    }
    
    /**
     * Set <useradditem> value
     *
     * @param \RO\Cmd\OffMsgUserAddItem $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setUseradditem(\RO\Cmd\OffMsgUserAddItem $value){
      return $this->_set(24, $value);
    }
    
    /**
     * Check if <weddingmsg> has a value
     *
     * @return boolean
     */
    public function hasWeddingmsg(){
      return $this->_has(25);
    }
    
    /**
     * Clear <weddingmsg> value
     *
     * @return \RO\Cmd\OfflineMsg
     */
    public function clearWeddingmsg(){
      return $this->_clear(25);
    }
    
    /**
     * Get <weddingmsg> value
     *
     * @return \RO\Cmd\WeddingEventMsgCCmd
     */
    public function getWeddingmsg(){
      return $this->_get(25);
    }
    
    /**
     * Set <weddingmsg> value
     *
     * @param \RO\Cmd\WeddingEventMsgCCmd $value
     * @return \RO\Cmd\OfflineMsg
     */
    public function setWeddingmsg(\RO\Cmd\WeddingEventMsgCCmd $value){
      return $this->_set(25, $value);
    }
  }
}


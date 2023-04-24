// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: xCmd.proto

#ifndef PROTOBUF_xCmd_2eproto__INCLUDED
#define PROTOBUF_xCmd_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace Cmd {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_xCmd_2eproto();
void protobuf_AssignDesc_xCmd_2eproto();
void protobuf_ShutdownFile_xCmd_2eproto();

class Nonce;

enum Command {
  LOGIN_USER_PROTOCMD = 1,
  ERROR_USER_PROTOCMD = 2,
  SCENE_USER_PROTOCMD = 5,
  SCENE_USER_ITEM_PROTOCMD = 6,
  SCENE_USER_SKILL_PROTOCMD = 7,
  SCENE_USER_QUEST_PROTOCMD = 8,
  SCENE_USER2_PROTOCMD = 9,
  SCENE_USER_PET_PROTOCMD = 10,
  FUBEN_PROTOCMD = 11,
  SCENE_USER_MAP_PROTOCMD = 12,
  SCENE_USER_MOUNT_PROTOCMD = 13,
  SCENE_BOSS_PROTOCMD = 15,
  SCENE_USER_CARRIER_PROTOCMD = 16,
  SCENE_USER_ACHIEVE_PROTOCMD = 17,
  SCENE_USER_TIP_PROTOCMD = 18,
  SCENE_USER_CHATROOM_PROTOCMD = 19,
  INFINITE_TOWER_PROTOCMD = 20,
  SCENE_USER_SEAL_PROTOCMD = 21,
  SCENE_USER_INTER_PROTOCMD = 22,
  SCENE_USER_MANUAL_PROTOCMD = 23,
  SCENE_USER_CHAT_PROTOCMD = 24,
  USER_EVENT_PROTOCMD = 25,
  SCENE_USER_TRADE_PROTOCMD = 26,
  SCENE_USER_AUGURY_PROTOCMD = 27,
  SCENE_USER_ASTROLABE_PROTOCMD = 28,
  SCENE_USER_FOOD_PROTOCMD = 29,
  SCENE_USER_PHOTO_PROTOCMD = 30,
  SCENE_USER_TUTOR_PROTOCMD = 31,
  SCENE_USER_BEING_PROTOCMD = 32,
  SESSION_USER_GUILD_PROTOCMD = 50,
  SESSION_USER_TEAM_PROTOCMD = 51,
  SESSION_USER_SHOP_PROTOCMD = 52,
  SESSION_USER_WEATHER_PROTOCMD = 53,
  SESSION_USER_MAIL_PROTOCMD = 55,
  SESSION_USER_SOCIALITY_PROTOCMD = 56,
  RECORD_USER_TRADE_PROTOCMD = 57,
  DOJO_PROTOCMD = 58,
  CHAT_PROTOCMD = 59,
  ACTIVITY_PROTOCMD = 60,
  MATCHC_PROTOCMD = 61,
  SESSION_USER_AUTHORIZE_PROTOCMD = 62,
  AUCTIONC_PROTOCMD = 63,
  ACTIVITY_EVENT_PROTOCMD = 64,
  WEDDINGC_PROTOCMD = 65,
  PVE_CARD_PROTOCMD = 66,
  TEAM_RAID_PROTOCMD = 67,
  SESSION_OVERSEAS_TW_PROTOCMD = 80,
  CLIENT_CMD = 99,
  MAX_USER_CMD = 100,
  RECORD_DATA_PROTOCMD = 200,
  TRADE_PROTOCMD = 201,
  SESSION_PROTOCMD = 202,
  GMTOOLS_PROTOCMD = 203,
  LOG_PROTOCMD = 204,
  GATE_SUPER_PROTOCMD = 205,
  REGION_PROTOCMD = 206,
  STAT_PROTOCMD = 207,
  SOCIAL_PROTOCMD = 208,
  TEAM_PROTOCMD = 209,
  GUILD_PROTOCMD = 210,
  GZONE_PROTOCMD = 211,
  MATCHS_PROTOCMD = 212,
  AUCTIONS_PROTOCMD = 213,
  WEDDINGS_PROTOCMD = 214,
  BOSSS_PROTOCMD = 216,
  REG_CMD = 253,
  GATEWAY_CMD = 250,
  SYSTEM_PROTOCMD = 255
};
bool Command_IsValid(int value);
const Command Command_MIN = LOGIN_USER_PROTOCMD;
const Command Command_MAX = SYSTEM_PROTOCMD;
const int Command_ARRAYSIZE = Command_MAX + 1;

const ::google::protobuf::EnumDescriptor* Command_descriptor();
inline const ::std::string& Command_Name(Command value) {
  return ::google::protobuf::internal::NameOfEnum(
    Command_descriptor(), value);
}
inline bool Command_Parse(
    const ::std::string& name, Command* value) {
  return ::google::protobuf::internal::ParseNamedEnum<Command>(
    Command_descriptor(), name, value);
}
// ===================================================================

class Nonce : public ::google::protobuf::Message {
 public:
  Nonce();
  virtual ~Nonce();

  Nonce(const Nonce& from);

  inline Nonce& operator=(const Nonce& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Nonce& default_instance();

  void Swap(Nonce* other);

  // implements Message ----------------------------------------------

  Nonce* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Nonce& from);
  void MergeFrom(const Nonce& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional uint32 timestamp = 1;
  inline bool has_timestamp() const;
  inline void clear_timestamp();
  static const int kTimestampFieldNumber = 1;
  inline ::google::protobuf::uint32 timestamp() const;
  inline void set_timestamp(::google::protobuf::uint32 value);

  // optional uint32 index = 2;
  inline bool has_index() const;
  inline void clear_index();
  static const int kIndexFieldNumber = 2;
  inline ::google::protobuf::uint32 index() const;
  inline void set_index(::google::protobuf::uint32 value);

  // optional string sign = 3;
  inline bool has_sign() const;
  inline void clear_sign();
  static const int kSignFieldNumber = 3;
  inline const ::std::string& sign() const;
  inline void set_sign(const ::std::string& value);
  inline void set_sign(const char* value);
  inline void set_sign(const char* value, size_t size);
  inline ::std::string* mutable_sign();
  inline ::std::string* release_sign();
  inline void set_allocated_sign(::std::string* sign);

  // @@protoc_insertion_point(class_scope:Cmd.Nonce)
 private:
  inline void set_has_timestamp();
  inline void clear_has_timestamp();
  inline void set_has_index();
  inline void clear_has_index();
  inline void set_has_sign();
  inline void clear_has_sign();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::uint32 timestamp_;
  ::google::protobuf::uint32 index_;
  ::std::string* sign_;
  friend void  protobuf_AddDesc_xCmd_2eproto();
  friend void protobuf_AssignDesc_xCmd_2eproto();
  friend void protobuf_ShutdownFile_xCmd_2eproto();

  void InitAsDefaultInstance();
  static Nonce* default_instance_;
};
// ===================================================================


// ===================================================================

// Nonce

// optional uint32 timestamp = 1;
inline bool Nonce::has_timestamp() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void Nonce::set_has_timestamp() {
  _has_bits_[0] |= 0x00000001u;
}
inline void Nonce::clear_has_timestamp() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void Nonce::clear_timestamp() {
  timestamp_ = 0u;
  clear_has_timestamp();
}
inline ::google::protobuf::uint32 Nonce::timestamp() const {
  // @@protoc_insertion_point(field_get:Cmd.Nonce.timestamp)
  return timestamp_;
}
inline void Nonce::set_timestamp(::google::protobuf::uint32 value) {
  set_has_timestamp();
  timestamp_ = value;
  // @@protoc_insertion_point(field_set:Cmd.Nonce.timestamp)
}

// optional uint32 index = 2;
inline bool Nonce::has_index() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void Nonce::set_has_index() {
  _has_bits_[0] |= 0x00000002u;
}
inline void Nonce::clear_has_index() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void Nonce::clear_index() {
  index_ = 0u;
  clear_has_index();
}
inline ::google::protobuf::uint32 Nonce::index() const {
  // @@protoc_insertion_point(field_get:Cmd.Nonce.index)
  return index_;
}
inline void Nonce::set_index(::google::protobuf::uint32 value) {
  set_has_index();
  index_ = value;
  // @@protoc_insertion_point(field_set:Cmd.Nonce.index)
}

// optional string sign = 3;
inline bool Nonce::has_sign() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void Nonce::set_has_sign() {
  _has_bits_[0] |= 0x00000004u;
}
inline void Nonce::clear_has_sign() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void Nonce::clear_sign() {
  if (sign_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    sign_->clear();
  }
  clear_has_sign();
}
inline const ::std::string& Nonce::sign() const {
  // @@protoc_insertion_point(field_get:Cmd.Nonce.sign)
  return *sign_;
}
inline void Nonce::set_sign(const ::std::string& value) {
  set_has_sign();
  if (sign_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    sign_ = new ::std::string;
  }
  sign_->assign(value);
  // @@protoc_insertion_point(field_set:Cmd.Nonce.sign)
}
inline void Nonce::set_sign(const char* value) {
  set_has_sign();
  if (sign_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    sign_ = new ::std::string;
  }
  sign_->assign(value);
  // @@protoc_insertion_point(field_set_char:Cmd.Nonce.sign)
}
inline void Nonce::set_sign(const char* value, size_t size) {
  set_has_sign();
  if (sign_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    sign_ = new ::std::string;
  }
  sign_->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:Cmd.Nonce.sign)
}
inline ::std::string* Nonce::mutable_sign() {
  set_has_sign();
  if (sign_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    sign_ = new ::std::string;
  }
  // @@protoc_insertion_point(field_mutable:Cmd.Nonce.sign)
  return sign_;
}
inline ::std::string* Nonce::release_sign() {
  clear_has_sign();
  if (sign_ == &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    return NULL;
  } else {
    ::std::string* temp = sign_;
    sign_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    return temp;
  }
}
inline void Nonce::set_allocated_sign(::std::string* sign) {
  if (sign_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete sign_;
  }
  if (sign) {
    set_has_sign();
    sign_ = sign;
  } else {
    clear_has_sign();
    sign_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  }
  // @@protoc_insertion_point(field_set_allocated:Cmd.Nonce.sign)
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace Cmd

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::Cmd::Command> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::Cmd::Command>() {
  return ::Cmd::Command_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_xCmd_2eproto__INCLUDED
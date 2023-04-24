#include <utility>
#include "xlib/xEntryManager.h"

// xEntryMultiName
xEntryMultiName::xEntryMultiName() {
}

bool xEntryMultiName::push(xEntry *e) {
  ets_.insert(std::make_pair(e->name, e));
  return true;
}

void xEntryMultiName::erase(xEntry *e) {
  ets_.erase(e->name);
}

void xEntryMultiName::find(const char* name, xEntry** e) {
  if (!name) return;
  ConstIter it = ets_.find(name);
  if (it != ets_.end()) {
    *e = it->second;
  }
}

xEntry* xEntryMultiName::getEntryByName(const char* name) {
  if (!name) return NULL;
  ConstIter it = ets_.find(name);
  if (it != ets_.end())
    return it->second;
  return NULL;
}

// xEntryID
xEntryID::xEntryID() {
}

bool xEntryID::push(xEntry *e) {
  ConstIter it = ets_.find(e->id);
  if (it == ets_.end()) {
    ets_.insert(std::pair<UINT, xEntry *>(e->id, e));
    return true;
  }
  return false;
}

void xEntryID::erase(xEntry *e) {
  Iter it = ets_.find(e->id);
  if (it != ets_.end()) {
    ets_.erase(it);
  }
}

xEntry* xEntryID::getEntryByID(UINT id) {
  ConstIter it = ets_.find(id);
  if (it != ets_.end())
    return it->second;
  return NULL;
}

void xEntryID::find(UINT id, xEntry** e) {
  ConstIter it = ets_.find(id);
  if (it != ets_.end()) {
    *e = (it->second);
  }
}

// xEntryName
xEntryName::xEntryName() {
}

bool xEntryName::push(xEntry *e) {
  Iter it = ets_.find(e->name);
  if (it == ets_.end()) {
    ets_.insert(std::make_pair(e->name, e));
    return true;
  }
  return false;
}

void xEntryName::erase(xEntry *e) {
  Iter it = ets_.find(e->name);
  if (it != ets_.end()) {
    ets_.erase(it);
  }
}

void xEntryName::find(const char* name, xEntry** e) {
  if (!name) return;
  ConstIter it = ets_.find(name);
  if (it != ets_.end()) {
    *e = it->second;
  }
}

xEntry* xEntryName::getEntryByName(const char* name) {
  if (!name) return NULL;
  ConstIter it = ets_.find(name);
  if (it != ets_.end())
    return it->second;
  return NULL;
}

// xEntryTempID
xEntryTempID::xEntryTempID() {
}

bool xEntryTempID::push(xEntry *e) {
  ConstIter it = ets_.find(e->tempid);
  if (it == ets_.end()) {
    ets_.insert(std::pair<UINT, xEntry *>(e->tempid, e));
    return true;
  }
  return false;
}

void xEntryTempID::erase(xEntry *e) {
  Iter it = ets_.find(e->tempid);
  if (it != ets_.end()) {
    ets_.erase(it);
  }
}

xEntry* xEntryTempID::getEntryByTempID(UINT id) {
  ConstIter it = ets_.find(id);
  if (it != ets_.end())
    return it->second;
  return NULL;
}

void xEntryTempID::find(UINT id, xEntry** e) {
  ConstIter it = ets_.find(id);
  if (it != ets_.end()) {
    *e = it->second;
  }
}

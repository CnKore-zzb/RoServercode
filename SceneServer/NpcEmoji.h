#pragma once

#include "xDefine.h"
#include "xNoncopyable.h"
#include "TableStruct.h"

class SceneNpc;

class NpcEmoji : private xNoncopyable
{
  public:
    NpcEmoji(SceneNpc *npc);
    ~NpcEmoji();
    
  public:
    void init();
    void play(DWORD emojiID);
    void check(const char *t);

  private:
    SceneNpc *m_pNpc = nullptr;
    const MonsterEmojiBase *m_base = nullptr;
    std::map<std::string, DWORD> m_cdlist;
};

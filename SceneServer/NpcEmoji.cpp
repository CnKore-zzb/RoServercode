#include "NpcEmoji.h"
#include "SceneNpc.h"
#include "TableManager.h"

NpcEmoji::NpcEmoji(SceneNpc *npc):m_pNpc(npc)
{
  init();
}

NpcEmoji::~NpcEmoji()
{
}

void NpcEmoji::init()
{
  m_base = TableManager::getMe().getMonsterEmojiCFG(m_pNpc->getNpcID());
}

void NpcEmoji::play(DWORD emojiID)
{
  m_pNpc->playEmoji(emojiID);
}

void NpcEmoji::check(const char *t)
{
  if (!t || !m_base) return;

  MonsterEmojiItem item;
  if (m_base->getItem(t, item))
  {
    if (item.cd)
    {
      auto it = m_cdlist.find(t);
      if (it!=m_cdlist.end())
      {
        if (it->second > now())
          return;
      }
    }
    if (selectByPercent(item.per))
    {
      play(item.emoji);
      if (item.cd)
      {
        m_cdlist[t] = now() + item.cd;
      }
    }
  }
}

#include <math.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include "zlib/zlib.h"
#include "xTime.h"
#include "GMCommandRuler.h"
#include "xSceneEntryDynamic.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "Scene.h"
#include "SceneUserManager.h"
#include "SceneServer.h"
#include "SkillItem.h"
#include "MsgManager.h"
#include "SceneManager.h"
#include "GatewayCmd.h"
//#include "SessionSceneCmd.h"
#include "SessionCmd.pb.h"
#include "SuperCmd.h"
#include <stdexcept>
#include "SystemCmd.pb.h"
#include "GearManager.h"
#include "BossCmd.pb.h"
//#include "SceneLuaManager.h"
#include "SessionWeather.pb.h"
#include "SceneTower.h"
#include "SkillManager.h"
#include "CarrierCmd.pb.h"
#include "SceneItemManager.h"
#include "RecordTrade.pb.h"
#include "LoginUserCmd.pb.h"
#include "LuaManager.h"
#include "SessionSociality.pb.h"
#include "RedisManager.h"
#include "PlatLogManager.h"
#include "DScene.h"
#include "ActivityManager.h"
#include "MailManager.h"
#include "ChatManager_SC.h"
#include "CommonConfig.h"
#include "MatchCCmd.pb.h"
#include "MatchSCmd.pb.h"
#include "GuildRaidConfig.h"
#include "BaseConfig.h"
#include "SceneActManager.h"
#include "GuildCityManager.h"
#include "SocialCmd.pb.h"
#include "SceneWeddingMgr.h"
#include "PetWork.h"
#include "ActivityEventManager.h"
#include "PveCardEffect.h"
#include "Menu.h"
#include "UserRecords.h"
#include "SceneShop.h"
#include "Transfer.h"
#include "GlobalManager.h"
#include "BossMgr.h"
extern "C"
{
#include "xlib/md5/md5.h"
}

//const char LOG_NAME[] = "GMCommandRuler";

GmCommand GMCommands[]=
{
  // 组长以上权限
  {"loadlua", GMCommandRuler::loadlua, HUMAN_SUPER_GM|HUMAN_GM_CAPTAIN|HUMAN_GM, "加载配置"},
  {"captain", GMCommandRuler::captain, HUMAN_SUPER_GM|HUMAN_GM_CAPTAIN, "组长权限杂项"},
  {"cmdfilter", GMCommandRuler::cmdfilter, HUMAN_SUPER_GM|HUMAN_GM_CAPTAIN, "消息过滤"},
  {"patch", GMCommandRuler::patch, HUMAN_SUPER_GM|HUMAN_GM_CAPTAIN|HUMAN_GM, "补丁操作"},

  // GM command
  {"additem", GMCommandRuler::additem, HUMAN_NORMAL, "添加物品"},
  {"subitem", GMCommandRuler::subitem, HUMAN_NORMAL, "删除物品" },
  {"addmoney", GMCommandRuler::addmoney, HUMAN_NORMAL, "增加金钱"},
  {"submoney", GMCommandRuler::submoney, HUMAN_NORMAL, "减少金钱" },
  {"money", GMCommandRuler::money, HUMAN_NORMAL, "金钱操作" },
  {"removecard", GMCommandRuler::removecard, HUMAN_NORMAL, "删除卡片" },
  {"addattrpoint", GMCommandRuler::addattrpoint, HUMAN_NORMAL, "增加属性点"},
  {"addbaseexp", GMCommandRuler::addbaseexp, HUMAN_NORMAL, "增加人物经验"},
  {"addjobexp", GMCommandRuler::addjobexp, HUMAN_NORMAL, "增加职业经验"},
  {"exchangejob", GMCommandRuler::exchangejob, HUMAN_NORMAL, "切换职业"},
  {"reward", GMCommandRuler::reward, HUMAN_NORMAL, "奖励"},
  {"setattr", GMCommandRuler::setattr, HUMAN_NORMAL, "设置属性"},
  {"raid", GMCommandRuler::dmap, HUMAN_NORMAL, "动态地图"},
  {"gocity", GMCommandRuler::gocity, HUMAN_NORMAL, "回城"},
  {"follower", GMCommandRuler::follower, HUMAN_NORMAL, "跟随npc"},
  {"rideon", GMCommandRuler::rideon, HUMAN_NORMAL, "上坐骑"},
  {"rideoff", GMCommandRuler::rideoff, HUMAN_NORMAL, "下坐骑"},
  //{"stage", GMCommandRuler::stage, HUMAN_NORMAL, "关卡"},
  {"addbuff", GMCommandRuler::buff, HUMAN_NORMAL, "添加buff"},
  {"delbuff", GMCommandRuler::delbuff, HUMAN_NORMAL, "删除buff"},
  //{"changeskill", GMCommandRuler::changeskill, HUMAN_NORMAL, "修改技能参数"},
  {"heal", GMCommandRuler::heal, HUMAN_NORMAL, "使用药品"},
  {"god", GMCommandRuler::god, HUMAN_NORMAL, "无敌"},
  {"killer", GMCommandRuler::killer, HUMAN_NORMAL, "必杀"},
  {"hideme", GMCommandRuler::hideme, HUMAN_NORMAL, "隐身"},
  {"normal", GMCommandRuler::normal, HUMAN_NORMAL, "恢复正常 无敌 必杀 隐身"},
  {"levelup", GMCommandRuler::levelup, HUMAN_NORMAL, "base升级"},
  {"joblevelup", GMCommandRuler::joblevelup, HUMAN_NORMAL, "job升级"},
  {"basetolevel", GMCommandRuler::basetolevel, HUMAN_NORMAL, "base升到指定等级"},
  {"jobtolevel", GMCommandRuler::jobtolevel, HUMAN_NORMAL, "job升到指定等级"},
  {"killnine", GMCommandRuler::killnine, HUMAN_NORMAL, "杀9屏"},
  {"effect", GMCommandRuler::effect, HUMAN_NORMAL, "特效"},
  {"quest", GMCommandRuler::quest, HUMAN_NORMAL, "完成任务"},
  {"summon", GMCommandRuler::summon, HUMAN_NORMAL, "召唤npc"},
  {"setcolor", GMCommandRuler::setColor, HUMAN_NORMAL, "改变衣服颜色"},
  {"systime", GMCommandRuler::setSysTime, HUMAN_NORMAL, "系统时间"},
  {"menu", GMCommandRuler::menu, HUMAN_NORMAL, "功能打开"},
  {"restart", GMCommandRuler::restart, HUMAN_NORMAL, "重启服务器"},
  {"carrier", GMCommandRuler::carrier, HUMAN_NORMAL, "载具"},
  {"addmount", GMCommandRuler::addmount, HUMAN_NORMAL, "载具"},
  {"gethair", GMCommandRuler::getHair, HUMAN_NORMAL, "激活理发店发型"},
  {"geteye", GMCommandRuler::getEye, HUMAN_NORMAL, "激活美瞳"},
  {"testbuff", GMCommandRuler::testBuff, HUMAN_NORMAL, "buff测试"},
  {"gear", GMCommandRuler::gear, HUMAN_NORMAL, "场景装置"},
  {"showboss", GMCommandRuler::showboss, HUMAN_NORMAL, "显示boss坐标"},
  {"cleanraidboss", GMCommandRuler::cleancorpse, HUMAN_NORMAL, "净化boss尸体"},
  {"openui", GMCommandRuler::openui, HUMAN_NORMAL, "打开UI界面"},
  {"exit_go", GMCommandRuler::exit_go, HUMAN_NORMAL, "跳转"},
  {"exit_visible", GMCommandRuler::exit_visible, HUMAN_NORMAL, "设置跳转"},
  {"addpurify", GMCommandRuler::addpurify, HUMAN_NORMAL, "添加净化值"},
  {"finishquest", GMCommandRuler::finishQuest, HUMAN_NORMAL, "完成任务"},
  {"finishGroup", GMCommandRuler::finishGroup, HUMAN_NORMAL, "完成小任务"},
  {"addskill", GMCommandRuler::addskill, HUMAN_NORMAL, "获得技能"},
  {"addpartner", GMCommandRuler::addpartner, HUMAN_NORMAL, "添加猎鹰"},
  {"removepartner", GMCommandRuler::removepartner, HUMAN_NORMAL, "移除猎鹰"},
  {"npcfunction", GMCommandRuler::npcfunction, HUMAN_NORMAL, "调用NPC功能"},
  {"modelshow", GMCommandRuler::modelshow, HUMAN_NORMAL, "模型显示"},
  //{"followleader", GMCommandRuler::followleader, HUMAN_NORMAL, "跟随队长"},
  {"sound_effect", GMCommandRuler::sound_effect, HUMAN_NORMAL, "播放音效"},
  {"clearpack", GMCommandRuler::clearpack, HUMAN_NORMAL, "清空包裹"},
  {"sound_bgm", GMCommandRuler::sound_bgm, HUMAN_NORMAL, "改变BGM"},
  {"gametime", GMCommandRuler::gametime, HUMAN_NORMAL, "改变玩家的游戏时间"},
  {"resetgametime", GMCommandRuler::resetgametime, HUMAN_NORMAL, "重置玩家的游戏时间"},
  {"playaction", GMCommandRuler::playaction, HUMAN_NORMAL, "播放动作表情"},
  {"check_gametime", GMCommandRuler::checkgametime, HUMAN_NORMAL, "检查玩家的游戏时间"},
  {"check_weather", GMCommandRuler::checkweather, HUMAN_NORMAL, "检查玩家当前天气"},
  {"set_sky", GMCommandRuler::setsky, HUMAN_NORMAL, "设置玩家当前天空"},
  {"set_weather", GMCommandRuler::setweather, HUMAN_NORMAL, "设置玩家当前天气"},
  { "setenv", GMCommandRuler::setenv, HUMAN_NORMAL, "设置玩家当前天空 天气" },
  {"randpos", GMCommandRuler::randPos, HUMAN_NORMAL, "随机移动"},
  {"savemap", GMCommandRuler::savemap, HUMAN_NORMAL, "保存回城点"},
  {"shakescreen",GMCommandRuler::shakescreen, HUMAN_NORMAL, "震屏" },
  {"useskill",GMCommandRuler::useskill, HUMAN_NORMAL, "使用技能" },
  {"delstatus",GMCommandRuler::delstatus, HUMAN_NORMAL, "祛除buff状态" },
  {"entertower",GMCommandRuler::entertower, HUMAN_NORMAL, "进无限塔" },
  {"gopvp",GMCommandRuler::gopvp, HUMAN_NORMAL, "进入pvp" },
  {"rewards",GMCommandRuler::rewards, HUMAN_NORMAL, "同时多个reward" },
  {"fakedead",GMCommandRuler::fakedead, HUMAN_NORMAL, "进入装死状态" },
  {"addinterlocution",GMCommandRuler::addinterlocution, HUMAN_NORMAL, "添加问题" },
  {"resetattrpoint",GMCommandRuler::resetattrpoint, HUMAN_NORMAL, "重置属性点" },
  {"setattrpoint",GMCommandRuler::setattrpoint, HUMAN_NORMAL, "设置属性点" },
  {"resetskillpoint",GMCommandRuler::resetskillpoint, HUMAN_NORMAL, "重置技能点" },
  {"setskillpoint",GMCommandRuler::setskillpoint, HUMAN_NORMAL, "修改技能点" },
  {"npcscale",GMCommandRuler::npcscale, HUMAN_NORMAL, "怪物体型设置" },
  {"changecarrier",GMCommandRuler::changecarrier, HUMAN_NORMAL, "换载具" },
  {"scenery",GMCommandRuler::scenery, HUMAN_NORMAL, "景点拍照" },
  {"npctalk",GMCommandRuler::npctalk, HUMAN_NORMAL, "npc 讲话" },
  {"usehair",GMCommandRuler::usehair, HUMAN_NORMAL, "usehair" },
  {"npcdie",GMCommandRuler::npcdie, HUMAN_NORMAL, "npcdie" },
  {"openseal",GMCommandRuler::openseal, HUMAN_NORMAL, "openseal" },
  {"activemap",GMCommandRuler::activemap, HUMAN_NORMAL, "激活地图" },
  {"setsearchrange",GMCommandRuler::setSearchRange, HUMAN_NORMAL, "设置npc的搜索范围" },
  {"passtower",GMCommandRuler::passtower, HUMAN_NORMAL, "通关无限塔"},
  {"resettower",GMCommandRuler::resettower, HUMAN_NORMAL, "重置无限塔"},
  {"barrage",GMCommandRuler::barrage, HUMAN_NORMAL, "弹幕"},
  {"show_npc",GMCommandRuler::shownpc, HUMAN_NORMAL, "对指定npc可见"},
  {"hide_npc",GMCommandRuler::hidenpc, HUMAN_NORMAL, "对指定npc不可见"},
  {"addmotion",GMCommandRuler::addmotion, HUMAN_NORMAL, "获得动作"},
  {"handinhand",GMCommandRuler::handinhand, HUMAN_NORMAL, "牵手"},
  {"image",GMCommandRuler::image, HUMAN_NORMAL, "地图镜像"},
  {"misc",GMCommandRuler::misc, HUMAN_NORMAL, "杂项"},
  {"setdaily", GMCommandRuler::setdaily, HUMAN_NORMAL, "设置抗击魔潮倍率"},
  {"initdaily", GMCommandRuler::initdaily, HUMAN_NORMAL, "刷新抗击魔潮"},
  {"refreshquest", GMCommandRuler::refreshquest, HUMAN_NORMAL, "刷新任务"},
  {"dropitem", GMCommandRuler::dropItem, HUMAN_NORMAL, "道具掉落"},
  {"manual", GMCommandRuler::manual, HUMAN_NORMAL, "冒险手册"},
  {"portrait", GMCommandRuler::portrait, HUMAN_NORMAL, "头像"},
  {"delchar", GMCommandRuler::delchar, HUMAN_NORMAL, "删除角色"},

  { "trade_brief_search", GMCommandRuler::tradeBriefSearch, HUMAN_NORMAL, "某类物品的挂单总表" },
  { "trade_detail_search", GMCommandRuler::tradeDetailSearch, HUMAN_NORMAL, "某个物品的详细挂单" },
  { "trade_my_pending", GMCommandRuler::tradeMyPending, HUMAN_NORMAL, "我的挂单" },
  { "trade_my_log", GMCommandRuler::tradeMyLog, HUMAN_NORMAL, "我的交易记录" },

  { "trade_price", GMCommandRuler::tradePrice, HUMAN_NORMAL, "请求服务器价格" },
  { "trade_sell", GMCommandRuler::tradeSell, HUMAN_NORMAL, "交易出售物品" },
  { "trade_buy", GMCommandRuler::tradeBuy, HUMAN_NORMAL, "交易购买物品" },
  { "trade_cancel", GMCommandRuler::tradeCancel, HUMAN_NORMAL, "交易购买物品" },

  { "refreshtower", GMCommandRuler::refreshtower, HUMAN_NORMAL, "刷新无限塔" },
  { "compensateuser", GMCommandRuler::compensateuser, HUMAN_NORMAL, "玩家补偿" },
  { "speffect", GMCommandRuler::speffect, HUMAN_NORMAL, "特殊特效" },
  { "changearrow", GMCommandRuler::changearrow, HUMAN_NORMAL, "切换箭矢" },

  { "guildlevelup", GMCommandRuler::guildlevelup, HUMAN_NORMAL, "增加公会等级" },
  { "movetrack", GMCommandRuler::movetrack, HUMAN_NORMAL, "开启移动追踪" },
  { "setzone", GMCommandRuler::setzone, HUMAN_NORMAL, "切线" },
  { "itemimage",GMCommandRuler::itemimage, HUMAN_NORMAL, "二人世界" },
  { "itemmusic",GMCommandRuler::itemmusic, HUMAN_NORMAL, "音乐午后" },
  { "addhandnpc", GMCommandRuler::addhandnpc, HUMAN_NORMAL, "添加牵手npc" },
  { "toy_smile", GMCommandRuler::toy_smile, HUMAN_NORMAL, "痒痒棒" },
  { "use_gift_code", GMCommandRuler::use_gift_code, HUMAN_NORMAL, "兑换礼包码" },
  { "activity", GMCommandRuler::activity, HUMAN_NORMAL, "开启活动" },

  { "jumpzone", GMCommandRuler::jumpzone, HUMAN_NORMAL, "切线" },
  { "activity_reward", GMCommandRuler::activity_reward, HUMAN_NORMAL, "单身狗" },
  { "sendmsg", GMCommandRuler::sendmsg, HUMAN_NORMAL, "发送system msg" },
  { "dropreward", GMCommandRuler::dropReward, HUMAN_NORMAL, "道具掉落"},
  { "playdialog", GMCommandRuler::playdialog, HUMAN_NORMAL, "播放一段dialog"},
  { "unlockmanual", GMCommandRuler::unlockmanual, HUMAN_NORMAL, "解锁冒险手册"},
  { "usedepositcard", GMCommandRuler::usedepositcard, HUMAN_NORMAL, "使用充值卡片" },
  { "pickup", GMCommandRuler::pickup, HUMAN_NORMAL, "捡取" },
  { "npcmove", GMCommandRuler::npcmove, HUMAN_NORMAL, "使指定npc移动" },
  { "testtool", GMCommandRuler::testtool, HUMAN_NORMAL, "测试工具, 方便测试" },
  { "equip", GMCommandRuler::equip, HUMAN_NORMAL, "装备属性设置" },
  { "reset_shopskill", GMCommandRuler::resetShopSkill, HUMAN_NORMAL, "重置冒险技能" },
  { "randzeny", GMCommandRuler::randzeny, HUMAN_NORMAL, "随机获得zeny" },
  { "clearbattletime", GMCommandRuler::clearBattletime, HUMAN_NORMAL, "清空沉迷时间" },
  { "setcredit", GMCommandRuler::setCredit, HUMAN_NORMAL, "设置信用度" },
  { "playcharge", GMCommandRuler::playcharge, HUMAN_NORMAL, "播放充值动画" },
  { "chat", GMCommandRuler::chat, HUMAN_NORMAL, "聊天" },
  { "addweaponpet", GMCommandRuler::addweaponpet, HUMAN_NORMAL, "战斗猫" },
  { "loveletter", GMCommandRuler::loveletter, HUMAN_NORMAL, "情书" },
  { "refinetest", GMCommandRuler::refinetest, HUMAN_NORMAL, "精炼测试" },
  { "lotterytest", GMCommandRuler::lotterytest, HUMAN_NORMAL, "扭蛋测试" },
  { "unlockweaponpet", GMCommandRuler::unlockweaponpet, HUMAN_NORMAL, "战斗猫解锁" },
  { "seenpc", GMCommandRuler::seenpc, HUMAN_NORMAL, "设置自己对指定npc可见或不可见" },
  { "questitem", GMCommandRuler::questitem, HUMAN_NORMAL, "接取任务并给与道具" },
  { "reducehp", GMCommandRuler::reducehp, HUMAN_NORMAL, "扣血" },
  { "decsealcount", GMCommandRuler::decsealcount, HUMAN_NORMAL, "扣除裂隙修复次数" },
  { "changequest", GMCommandRuler::changequest, HUMAN_NORMAL, "改变任务step" },
  { "addquota", GMCommandRuler::addquota, HUMAN_NORMAL, "增加赠送额度" },
  { "subquota", GMCommandRuler::subquota, HUMAN_NORMAL, "减少赠送额度" },
  { "clearquota", GMCommandRuler::clearquota, HUMAN_NORMAL, "清除积分额度数据" },    
  { "joinpvp", GMCommandRuler::joinpvp, HUMAN_NORMAL, "加入pvp战斗" },
  { "kickpvpuser", GMCommandRuler::kickpvpuser, HUMAN_NORMAL, "将指定玩家踢出pvp" },
  { "resetpvp", GMCommandRuler::resetpvp, HUMAN_NORMAL, "重置pvp" },
  { "switchpvp", GMCommandRuler::switchpvp, HUMAN_NORMAL, "开关pvp" },
  { "firework", GMCommandRuler::firework, HUMAN_NORMAL, "花火" },
  { "optguildraid", GMCommandRuler::optguildraid, HUMAN_NORMAL, "操作公会副本大门" },  
  { "setcardslot", GMCommandRuler::setcardslot, HUMAN_NORMAL, "设置装备卡槽数量" },   
  { "deletepwd", GMCommandRuler::deletePassword, HUMAN_NORMAL, "删除安全密码" },
  { "oneclickrefine", GMCommandRuler::oneclickrefine, HUMAN_NORMAL, "一键精炼" },
  { "questnpc", GMCommandRuler::questnpc, HUMAN_NORMAL, "任务npc" },
  { "astrolabe", GMCommandRuler::astrolabe, HUMAN_NORMAL, "星盘" },
  { "showmini", GMCommandRuler::showmini, HUMAN_NORMAL, "跳转到mini的坐标"},
  { "resetaugury", GMCommandRuler::resetaugury, HUMAN_NORMAL, "重置占卜次数" },
  { "setvar", GMCommandRuler::setvar, HUMAN_NORMAL, "设置变量"},
  { "charge", GMCommandRuler::charge, HUMAN_NORMAL, "充值模拟"},
  { "photo", GMCommandRuler::photo, HUMAN_NORMAL, "个人相册" },
  { "delspeffect", GMCommandRuler::delspeffect, HUMAN_NORMAL, "删除特效" },
  { "achieve", GMCommandRuler::achieve, HUMAN_NORMAL, "个人成就" },
  { "catchpet", GMCommandRuler::catchpet, HUMAN_NORMAL, "捕捉宠物"},
  { "precatchpet", GMCommandRuler::precatchpet, HUMAN_NORMAL, "捕捉宠物,任务使用, 不使用道具"},
  { "pet", GMCommandRuler::pet, HUMAN_NORMAL, "宠物"},
  { "cookerlvup", GMCommandRuler::cookerlvup, HUMAN_NORMAL, "厨师等级升级" },
  { "tasterlvup", GMCommandRuler::tasterlvup, HUMAN_NORMAL, "美食家等级升级" },
  { "addcookerexp", GMCommandRuler::addcookerexp, HUMAN_NORMAL, "增加厨师经验" },
  { "addtasterexp", GMCommandRuler::addtasterexp, HUMAN_NORMAL, "增加美食家经验" },
  { "addfooddataexp", GMCommandRuler::addfooddataexp, HUMAN_NORMAL, "增加料理经验" },
  { "sceneeffect", GMCommandRuler::sceneeffect, HUMAN_NORMAL, "场景管理特效" },
  { "clearsatiety", GMCommandRuler::clearsatiety, HUMAN_NORMAL, "清空饱腹度" },
  { "advancepro", GMCommandRuler::advancepro, HUMAN_NORMAL, "一键转职" },
  { "unlockrecipe", GMCommandRuler::unlockrecipe, HUMAN_NORMAL, "解锁食谱" },       
  { "tutor", GMCommandRuler::tutor, HUMAN_NORMAL, "导师" },
  { "tutorskill", GMCommandRuler::tutorskill, HUMAN_NORMAL, "导师技能" },
  { "finishboardquest", GMCommandRuler::finishboardquest, HUMAN_NORMAL, "完成看板任务" },
  { "settowermaxlayer", GMCommandRuler::settowermaxlayer, HUMAN_NORMAL, "无限塔设置最高层" },       
  { "being", GMCommandRuler::being, HUMAN_NORMAL, "生命体" },
  { "lottery", GMCommandRuler::lottery, HUMAN_NORMAL, "扭蛋机" },       
  { "codeitem", GMCommandRuler::codeitem, HUMAN_NORMAL, "道具礼包码使用" },
  { "codeused", GMCommandRuler::codeused, HUMAN_NORMAL, "道具礼包码兑换成功" },
  { "beingbody", GMCommandRuler::beingbody, HUMAN_NORMAL, "生命体换肤" },
  { "addguildpray", GMCommandRuler::addguildpray, HUMAN_NORMAL, "工会祝福等级" },
  { "breakequip", GMCommandRuler::breakequip, HUMAN_NORMAL, "破坏装备" },
  { "guild", GMCommandRuler::guild, HUMAN_NORMAL, "公会" },
  { "callteamer", GMCommandRuler::callteamer, HUMAN_NORMAL, "召唤队友" },
  { "guildchallenge", GMCommandRuler::guildchallenge, HUMAN_NORMAL, "公会挑战" },
  { "addjoyvalue", GMCommandRuler::addjoyvalue, HUMAN_NORMAL, "增加欢乐值" },
  { "addbattletime", GMCommandRuler::addbattletime, HUMAN_NORMAL, "增加战斗时长" },
  { "genderreward", GMCommandRuler::genderreward, HUMAN_NORMAL, "根据性别解锁menu" },
  { "manualattributes", GMCommandRuler::addmanualattributes, HUMAN_NORMAL, "添加同一类型所有冒险手册属性" },
  { "manualleveldown", GMCommandRuler::manualleveldown, HUMAN_NORMAL, "降低冒险等级" },
  { "deltitle", GMCommandRuler::deltitle, HUMAN_NORMAL, "删除称号" },
  { "addmaxjoblv", GMCommandRuler::addmaxjoblv, HUMAN_NORMAL, "增加最大job等级" },
  { "setlotterycnt", GMCommandRuler::setlotterycnt, HUMAN_NORMAL, "设置每日扭蛋的次数" },
  { "clearlotterylimit", GMCommandRuler::clearlotterylimit, HUMAN_NORMAL, "清空扭蛋限制" },    
  { "marry", GMCommandRuler::marry, HUMAN_NORMAL, "结婚" },
  { "divorce", GMCommandRuler::divorce, HUMAN_NORMAL, "离婚" },
  { "marriageproposal", GMCommandRuler::marriageproposal, HUMAN_NORMAL, "求婚" },
  { "divorcemail", GMCommandRuler::divorcemail, HUMAN_NORMAL, "强制离婚邮件通知对方" },    
  { "forcedivorce", GMCommandRuler::forcedivorce, HUMAN_NORMAL, "强制离婚" },    
  { "rmcodeitem", GMCommandRuler::rmcodeitem, HUMAN_NORMAL, "删除兑换码道具" },
  { "tower", GMCommandRuler::tower, HUMAN_NORMAL, "无限塔" },
  { "laboratory", GMCommandRuler::laboratory, HUMAN_NORMAL, "研究所" },
  { "carddecompose", GMCommandRuler::carddecompose, HUMAN_NORMAL, "卡片分解" },
  { "tutorgrowreward", GMCommandRuler::tutorgrowreward, HUMAN_NORMAL, "领取学生成长奖励" },
  { "pvecardeffect", GMCommandRuler::pvecardeffect, HUMAN_NORMAL, "pve卡牌效果" },    
  { "user", GMCommandRuler::user, HUMAN_NORMAL, "玩家" },
  { "servant", GMCommandRuler::servant, HUMAN_NORMAL, "仆人" },
  { "branchremove", GMCommandRuler::branchremove, HUMAN_NORMAL, "职业分支移除" },
  { "branchrestore", GMCommandRuler::branchrestore, HUMAN_NORMAL, "还原至第一次备份分支，慎用" },
  { "branchcmd", GMCommandRuler::branchcmd, HUMAN_NORMAL, "分支协议，测试用" },
  { "slotcmd", GMCommandRuler::slotcmd, HUMAN_NORMAL, "存档协议，测试用" },
  { "resetmainchar", GMCommandRuler::resetmainchar, HUMAN_NORMAL, "重置主号，测试用" },
  { "modifycharge", GMCommandRuler::modifycharge, HUMAN_NORMAL, "修改充值相关数据" },
  { "inviteteammates", GMCommandRuler::inviteteammates, HUMAN_NORMAL, "邀请队友来到自己身边" },
  { "unlockpetwear", GMCommandRuler::unlockpetwear, HUMAN_NORMAL, "解锁宠物换装" },
  { "lockquota", GMCommandRuler::lockquota, HUMAN_NORMAL, "冻结额度" },
  { "unlockquota", GMCommandRuler::unlockquota, HUMAN_NORMAL, "解冻额度" },
  { "rewardsafety", GMCommandRuler::rewardsafety, HUMAN_NORMAL, "reward保底信息操作" },
  { "setshopgotcount", GMCommandRuler::setshopgotcount, HUMAN_NORMAL, "设置商品购买次数" },
  { "activatetransfer", GMCommandRuler::activatetransfer, HUMAN_NORMAL, "激活传送阵"},
  { "boss", GMCommandRuler::boss, HUMAN_NORMAL, "boss相关" },
  { "addresist", GMCommandRuler::addresist, HUMAN_NORMAL, "增加抗击魔潮次数" },
};

GMCommandRuler::GMCommandRuler()
{
}

GMCommandRuler::~GMCommandRuler()
{
}

#define GET_USER(entry) \
  if (!entry || entry->getEntryType()!=SCENE_ENTRY_USER) return false; \
SceneUser *pUser = (SceneUser *)entry;

bool GMCommandRuler::luaGMCmd(xSceneEntryDynamic* entry, const char* command)
{
  std::string str = command;
  return GMCommandRuler::execute(entry, str);
}

bool GMCommandRuler::execute(xSceneEntryDynamic* entry, std::string command)
{
  if (!entry || command.length()<=2)// || command[0]!='/'||command[1]!='/')
    return false;
  //command.erase(0,2);
  std::vector<std::string> sVec;
  stringTok(command, " ", sVec);
  if(sVec.empty())
    return false;
  GmFun pFun = 0;
  if (SCENE_ENTRY_USER==entry->getEntryType())
  {
    if (!checkGMRight((SceneUser *)entry, (char *)sVec[0].c_str(), pFun))
    {
      return false;
    }
  }
  else
  {
    for(DWORD n=0;n<(sizeof(GMCommands)/sizeof(GmCommand));n++)
    {
      if(0==strncmp(GMCommands[n].cmd, (char *)sVec[0].c_str(), MAX_NAMESIZE))
      {
        pFun = GMCommands[n].p;
      }
    }
  }

  //std::stringstream stream;
  //stream << "cmd={type=" << command << "}";

  xLuaData params;
  getParam(sVec, params);
  return (*pFun)(entry, params);
//  exeFunction(user, pFun);  
//  return true;
}

bool GMCommandRuler::execute(std::string command)
{
  if ( command.length() <= 2)
    return false;
  std::vector<std::string> sVec;
  stringTok(command, " ", sVec);
  if (sVec.empty())
    return false;
  GmFun pFun = 0;

  {
    for (DWORD n = 0; n < (sizeof(GMCommands) / sizeof(GmCommand)); n++)
    {
      if (0 == strncmp(GMCommands[n].cmd, (char *)sVec[0].c_str(), MAX_NAMESIZE))
      {
        pFun = GMCommands[n].p;
      }
    }
  }
  if (pFun == 0)
    return false;

  //std::stringstream stream;
  //stream << "cmd={type=" << command << "}";

  xLuaData params;
  getParam(sVec, params);
  return (*pFun)(nullptr, params);
}

bool GMCommandRuler::execute(xSceneEntryDynamic* entry, const xLuaData &data)
{
  if (!entry) return false;
  if (!data.has("type"))
  {
    XERR << "[GM指令-执行]" << entry->id << entry->name << "执行指令" << data.getTableString("type") << "失败,没有指令type" << XEND;
    return false;
  }

  GmFun pFun = 0;
  if (SCENE_ENTRY_USER==entry->getEntryType())
  {
    if (!checkGMRight((SceneUser *)entry, data.getTableString("type"), pFun))
    {
      XERR << "[GM指令-执行]" << entry->id << entry->name << "执行指令" << data.getTableString("type") << "失败,没有权限" << XEND;
      return false;
    }
  }
  else
  {
    for(DWORD n=0;n<(sizeof(GMCommands)/sizeof(GmCommand));n++)
    {
      if(0==strncmp(GMCommands[n].cmd, data.getTableString("type"), MAX_NAMESIZE))
      {
        pFun = GMCommands[n].p;
      }
    }
  }
  if (!pFun)
  {
    XERR << "[GM指令-执行]" << entry->id << entry->name << "执行指令" << data.getTableString("type") << "失败,未找到该指令" << XEND;
    return false;
  }

  return (*pFun)(entry, data);
//  exeFunction(user, pFun);  
//  return true;
}

void GMCommandRuler::getParam(const std::vector<std::string> &sVec, xLuaData &params)
{
  if (sVec.size() <= 1)
    return;

  std::vector<std::string> pVec;
  for (DWORD n = 1; n < sVec.size(); ++n)
  {
    pVec.clear();
    stringTok(sVec[n],"=", pVec);
    if (pVec.size() == 2)
    {
      // value=20 ; value=string
      std::string::size_type i = sVec[n].find_first_of("{", 0);
      if (std::string::npos == i)
      {
        params.setData(pVec[0], pVec[1]);
        continue;
      }

      // value={20,30}
      std::string::size_type j = sVec[n].find_first_of("}", 0);
      if (std::string::npos == j || j < i)
        continue;
      std::string str2 = sVec[n].substr(i + 1, j - i);
      std::vector<std::string> pVec2;
      stringTok(str2, ",", pVec2);
      xLuaData tmpdata;
      for (DWORD k = 0; k < pVec2.size(); ++k)
      {
        std::stringstream ss;
        ss << (k+1);
        tmpdata.setData(ss.str(), atof(pVec2[k].c_str()));
      }
      params.set(pVec[0], tmpdata);
    }
    else if (pVec.size()==1)
    {
      std::stringstream str;
      str.str("");
      str << n;
      params.setData(str.str().c_str(), pVec[0]);
    }

    // value={a=20,b=30}
    else if (pVec.size() > 2)
    {
      std::string::size_type i = sVec[n].find_first_of("{", 0);
      std::string::size_type j = sVec[n].find_first_of("}", 0);
      if (std::string::npos == i || std::string::npos == j || j < i)
        continue;
      std::string str = sVec[n].substr(i + 1, j - i);
      std::vector<std::string> pVec2;
      stringTok(str, ",", pVec2);
      std::vector<std::string> pVec3;
      xLuaData tmpdata;
      for (DWORD k = 0; k < pVec2.size(); ++k)
      {
        pVec3.clear();
        stringTok(pVec2[k], "=", pVec3);
        if (pVec3.size() != 2)
          continue;
        tmpdata.setData(pVec3[0], atof(pVec3[1].c_str()));
      }
      params.set(pVec[0], tmpdata);
    }
  }
}

bool GMCommandRuler::checkGMRight(SceneUser* user,const char* command, GmFun &fun)
{
  if(!user) return false;
  BYTE pri = user->getPrivilege();
  if (pri <= HUMAN_NORMAL) return false;
  for(DWORD n=0;n<(sizeof(GMCommands)/sizeof(GmCommand));n++)
  {
    if(0==strncmp(GMCommands[n].cmd, command, MAX_NAMESIZE))
    {
      fun = GMCommands[n].p;
      if (pri & GMCommands[n].pri)
      {
        return true;
      }
      else
      {
        //ChatMessage::sendInfo(user, Cmd::MESSAGE_INFO_FAIL,"权限不够.");
        return false;
      }
    }
  }
  //ChatMessage::sendInfo(user, Cmd::MESSAGE_INFO_FAIL,"未知指令.");
  return false;
}

bool GMCommandRuler::loadlua(xSceneEntryDynamic* user, const xLuaData &params)
{
  if (params.has("name") == false && params.has("lua") == false && params.has("log") == false)
    return false;

  LoadLuaSessionCmd message;
  if (params.has("name") == true)
    message.set_table(params.getTableString("name"));
  if (params.has("lua") == true)
    message.set_lua(params.getTableString("lua"));
  if (params.has("log") == true)
    message.set_log(params.getTableString("log"));
  message.set_serverid(thisServer->getZoneID());

  //is all zone reload
  bool allZone = true;
  if (params.has("allzone") == true)
    allZone = params.getTableInt("allzone");
  message.set_allzone(allZone);
  PROTOBUF(message, send, len);
  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::cmdfilter(xSceneEntryDynamic* user, const xLuaData &params)
{
  DWORD cmd = params.getTableInt("cmd");
  DWORD param = params.getTableInt("param");
  DWORD type = params.getTableInt("type");

  if (!cmd || !param || !type)
  {
    return false;
  }

  CmdFilterGatewayCmd send;
  send.cmd = cmd;
  send.param = param;
  send.type = type;
  thisServer->sendCmdToServer(&send, sizeof(send), "GateServer");

  XLOG << "[GM],cmdfilter:" << cmd << param << type << XEND;

  return true;
}

bool GMCommandRuler::captain(xSceneEntryDynamic* user, const xLuaData &params)
{
  return true;
}

bool GMCommandRuler::patch(xSceneEntryDynamic* user, const xLuaData &params)
{
  if (!params.has("act")) return false;

  std::string act = params.getTableString("act");
  if (act == "clearcdtime")
  {
    if (params.has("id"))
    {
      SceneUser *pUser = SceneUserManager::getMe().getUserByID(atoll(params.getTableString("id")));
      if (pUser)
      {
        pUser->m_oCDTime.clear();
      }
    }

  }
  return true;
}

// gm command
bool GMCommandRuler::additem(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  DWORD id = 0;
  DWORD count = 0;
  EPackMethod eMethod = EPACKMETHOD_CHECK_NOPILE;
  ESource eSource = ESOURCE_GM;

  if (!params.has("id"))
    return false;
  id = params.getTableInt("id");

  if (params.has("num"))
    count = params.getTableInt("num");
  count = count == 0 ? 1 : count;

  if (params.has("source"))
  {
    DWORD dwSource = params.getTableInt("source");
    if (dwSource <= ESOURCE_MIN || dwSource >= ESOURCE_MAX)
      return false;
    if (!ESource_IsValid(dwSource))
      return false;
    eSource = static_cast<ESource>(dwSource);
  }

  if (params.has("method"))
  {
    DWORD dwMethod = params.getTableInt("method");
    if (dwMethod <= EPACKMETHOD_MIN || dwMethod >= EPACKMETHOD_MAX)
      return false;
    eMethod = static_cast<EPackMethod>(dwMethod);
  }

  ItemData oData;
  oData.mutable_base()->set_id(id);
  oData.mutable_base()->set_count(count);
  oData.mutable_base()->set_source(eSource);

  bool bEgg = false;
  if (params.has("egg_skill1") == true)
  {
    oData.mutable_egg()->add_skillids(params.getTableInt("egg_skill1"));
    bEgg = true;
  }
  if (params.has("egg_skill2") == true)
  {
    oData.mutable_egg()->add_skillids(params.getTableInt("egg_skill2"));
    bEgg = true;
  }
  if (params.has("egg_skill3") == true)
  {
    oData.mutable_egg()->add_skillids(params.getTableInt("egg_skill3"));
    bEgg = true;
  }
  if (params.has("egg_skill4") == true)
  {
    oData.mutable_egg()->add_skillids(params.getTableInt("egg_skill4"));
    bEgg = true;
  }
  if (params.has("egg_baselv") == true)
  {
    oData.mutable_egg()->set_lv(params.getTableInt("egg_baselv"));
    bEgg = true;

    DWORD petid = PetConfig::getMe().getPetIDByItem(id);
    oData.mutable_egg()->set_id(petid);
    const SPetCFG* pCFG = PetConfig::getMe().getPetCFG(oData.egg().id());
    if (pCFG == nullptr)
      return false;
    TSetDWORD setIDs;
    pCFG->getRandomSkill(setIDs);
    for (auto &s : setIDs)
      oData.mutable_egg()->add_skillids(s);
  }
  if (params.has("egg_friendlv") == true)
  {
    oData.mutable_egg()->set_friendlv(params.getTableInt("egg_friendlv"));
    bEgg = true;
  }

  if (bEgg)
  {
    const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(id);
    if (pCFG == nullptr || pCFG->eItemType != EITEMTYPE_EGG)
      return false;
  }

  //enchant
  DWORD enchantType = params.getTableInt("enchant_type");
  if (enchantType)
  {
    const SItemCFG* pItemCfg = ItemConfig::getMe().getItemCFG(id);
    if (pItemCfg == nullptr)
    {
      return false;
    }

    if (pItemCfg->eEquipType == EEQUIPTYPE_MIN)
    {
      XERR << "[GM-增加道具-附魔] 不是装备，不可附魔 itemid" << id << "eEquipType" << pItemCfg->eEquipType << XEND;
      return false;
    }
    if (EEnchantType_IsValid(enchantType) == false)
    {
      XERR << "[GM-增加道具-附魔] 非法的附魔类型 itemid" << id << "eEquipType" << pItemCfg->eEquipType << XEND;
      return false;
    }

    EnchantData enchantData;
    if (!EEnchantType_IsValid(enchantType))
      return false;
    EEnchantType eType = static_cast<EEnchantType>(enchantType);
    const SEnchantCFG* pCfg = ItemConfig::getMe().getEnchantCFG(eType);
    if (pCfg == nullptr)
    {
      XERR << "[GM-增加道具-附魔] 错误的附魔类型 itemid" << id << "eEquipType" << pItemCfg->eEquipType << "enchanttype" << eType << XEND;
      return false;
    }

    enchantData.set_type(eType);

    auto enchantF = [&](string attrStr, string valueStr)
    {
      DWORD dwAttr = params.getTableInt(attrStr);
      float fValue = params.getTableFloat(valueStr);
      if (dwAttr == 0 || fValue == 0)
        return true;

      EnchantAttr* pAttr = enchantData.add_attrs();
      if (pAttr == nullptr)
      {
        XERR << "[GM-增加道具-附魔] 创建attr protobuf失败" << XEND;
        return false;
      }
      const RoleData* pData = RoleDataConfig::getMe().getRoleData(dwAttr);
      if (pData == nullptr)
      {
        XERR << "[GM-增加道具-附魔] 失败 attr : " << dwAttr << "不合法" << XEND;
        return false;
      }
      if (!EAttrType_IsValid(dwAttr))
      {
        XERR << "[GM-增加道具-附魔] 失败 attr : " << dwAttr << "转换失败" << XEND;
        return false;
      }
      pAttr->set_type(static_cast<EAttrType>(dwAttr));
      pAttr->set_value(pData->bPercent ? fValue * FLOAT_TO_DWORD : fValue);
      return true;
    };

    if (enchantF("attr1", "value1") == false)
      return false;

    if (enchantF("attr2", "value2") == false)
      return false;
    if (enchantF("attr3", "value3") == false)
      return false;

    DWORD dwEnchantBuf = params.getTableInt("buffid");
    if (dwEnchantBuf)
    {
      if (!pCfg->gmCheckAndSet(pItemCfg->eItemType, enchantData, dwEnchantBuf))
      {
        XERR << "[GM-增加道具-附魔] 错误的附魔buffid itemid" << id << "eEquipType" << pItemCfg->eEquipType << "enchanttype" << eType << "buffid" << dwEnchantBuf << XEND;
        return false;
      }
    }
    oData.mutable_enchant()->CopyFrom(enchantData);
  }

  EPackType eType = static_cast<EPackType>(params.getTableInt("packtype"));
  if (eType != EPACKTYPE_MIN)
  {
    BasePackage* pPack = pUser->getPackage().getPackage(eType);
    if (pPack == nullptr)
      return false;
    return pPack->addItem(oData, eMethod);
  }

  return pUser->getPackage().addItem(oData, eMethod);
}

bool GMCommandRuler::subitem(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  string guid = params.getTableString("guid");
  ItemInfo oItem;
  EPackType packType = static_cast<EPackType>(params.getTableInt("packtype"));

  oItem.set_guid(params.getTableString("guid"));
  oItem.set_id(params.getTableInt("id"));
  oItem.set_count(params.getTableInt("num"));

  BasePackage* pPack = pUser->getPackage().getPackage(packType);
  if (pPack == nullptr)
  {
    XERR << "[GM指令-删除物品]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "删除 guid :" << oItem.guid() << "失败,未找到" << packType << XEND;
    return false;
  }

  if (oItem.guid().empty() == false)
  {
    ItemBase* pBase = pPack->getItem(oItem.guid());
    if (pBase == nullptr)
    {
      XERR << "[GM指令-删除物品]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "删除 guid :" << guid << "失败,在" << packType << "中未找到该物品" << XEND;
      return false;
    }
    if (pBase->getCount() < oItem.count())
    {
      XERR << "[GM指令-删除物品]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "删除 guid :" << guid << "失败,在" << packType << "中物品数量 :" << pBase->getCount() << "不足 :" << oItem.count() << XEND;
      return false;
    }
    pPack->reduceItem(guid, ESOURCE_GM, oItem.count(), true);
  }
  else
  {
    if (pPack->checkItemCount(oItem.id(), oItem.count()) == false)
    {
      XERR << "[GM指令-删除物品]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "删除" << oItem.ShortDebugString() << "失败,在" << packType << "中物品数量不足" << XEND;
      return false;
    }
    pPack->reduceItem(oItem.id(), ESOURCE_GM, oItem.count());
  }

  return true;
}

bool GMCommandRuler::addmoney(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  EMoneyType eType = EMONEYTYPE_MIN;
  DWORD type = 0;
  QWORD count = 0;

  if (!params.has("type"))
    return false;
  type = params.getTableInt("type");
  if (type <= EMONEYTYPE_MIN || type >= EMONEYTYPE_MAX)
    return false;
  eType = static_cast<EMoneyType>(type);

  if (!params.has("num"))
    return false;
  count = params.getTableQWORD("num");
  count = count == 0 ? 1 : count;

  pUser->addMoney(eType, count, ESOURCE_GM);
  return true;
}

bool GMCommandRuler::submoney(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  EMoneyType eType = EMONEYTYPE_MIN;
  DWORD type = 0;
  QWORD count = 0;

  if (!params.has("type"))
    return false;
  type = params.getTableInt("type");
  if (type <= EMONEYTYPE_MIN || type >= EMONEYTYPE_MAX)
    return false;
  eType = static_cast<EMoneyType>(type);

  if (!params.has("num"))
    return false;
  count = params.getTableQWORD("num");
  count = count == 0 ? 1 : count;

  return pUser->subMoney(eType, count, ESOURCE_GM);
}

bool GMCommandRuler::money(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("max") == true && params.has("debt") == true)
  {
    QWORD qwMax = params.getTableQWORD("max");
    QWORD qwDebt = params.getTableQWORD("debt");
    QWORD qwOriDebt = qwDebt;

    if (pUser->getUserSceneData().getSilver() < qwDebt)
    {
      qwDebt -= pUser->getUserSceneData().getSilver();
      pUser->getUserSceneData().setSilver(0);
      pUser->getUserSceneData().addZenyDebt(qwMax, qwDebt);

      MsgParams mailParams;
      mailParams.addNumber(qwOriDebt);
      mailParams.addNumber(pUser->getUserSceneData().getZenyDebt());
      MailManager::getMe().sendMail(pUser->id, MiscConfig::getMe().getSystemCFG().dwDebtMailID, mailParams);
    }
    else
    {
      pUser->getUserSceneData().setSilver(pUser->getUserSceneData().getSilver() - qwDebt);
    }

    XLOG << "[GM指令-zeny上限]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "设置zeny上限 :" << qwMax << "成功 zeny :" << pUser->getUserSceneData().getSilver() << "debt :" << pUser->getUserSceneData().getZenyDebt() << XEND;
  }

  return true;
}

bool GMCommandRuler::removecard(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  std::string strCardGuid;
  std::string strEquipGuid;

  if (!params.has("cardguid"))
    return false;
  strCardGuid = params.getTableString("cardguid");

  if (!params.has("equipguid"))
    return false;
  strEquipGuid = params.getTableString("equipguid");

  //del card
  return pUser->getPackage().removeEquipcard(strCardGuid, strEquipGuid);
}

bool GMCommandRuler::addattrpoint(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  DWORD point = 0;
  if (!params.has("num"))
    return false;
  point = params.getTableInt("num");

  // add point
  SceneFighter* pFighter = pUser->getFighter();
  if (pFighter == nullptr)
    return false;
  pFighter->setTotalPoint(pFighter->getTotalPoint() + point);
  return true;
}

bool GMCommandRuler::addbaseexp(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  DWORD exp = 0;
  if (!params.has("num"))
    return false;
  exp = params.getTableInt("num");
  exp = exp == 0 ? 1 : exp;

  // add exp
  pUser->addBaseExp(exp, ESOURCE_GM);
  return true;
}

bool GMCommandRuler::addjobexp(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  DWORD exp = 0;
  if (!params.has("num"))
    return false;
  exp = params.getTableInt("num");
  exp = exp == 0 ? 1 : exp;

  // add exp
  pUser->addJobExp(exp, ESOURCE_GM);
  return true;
}

bool GMCommandRuler::exchangejob(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  DWORD profession = 0;
  if (!params.has("id"))
    return false;
  profession = params.getTableInt("id");
  if (profession <= EPROFESSION_MIN || profession >= EPROFESSION_MAX || EProfession_IsValid(profession) == false)
    return false;

  EProfession eProfession = static_cast<EProfession>(profession);
  pUser->exchangeProfession(eProfession);
  return true;
}

bool GMCommandRuler::reward(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  DWORD rewardID = 0;
  bool bShow = false;
  EPackMethod eMethod = EPACKMETHOD_NOCHECK;
  if (!params.has("id"))
  {
    XERR << "[GM指令-奖励]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未包含id" << XEND;
    return false;
  }

  rewardID = params.getTableInt("id");
  bShow = params.getTableInt("show") == 1;
  ESource source = ESOURCE_REWARD;
  if (params.has("source"))
  {
    DWORD s = params.getTableInt("source");
    if (ESource_IsValid(s))
      source = static_cast<ESource>(s);
  }
  DWORD dwDoubleReward = params.getTableInt("doublesource");

  if (params.has("method"))
  {
    DWORD dwMethod = params.getTableInt("method");
    if (dwMethod <= EPACKMETHOD_MIN || dwMethod >= EPACKMETHOD_MAX || dwMethod == EPACKMETHOD_CHECK_NOPILE || dwMethod == EPACKMETHOD_CHECK_WITHPILE)
      return false;
    eMethod = static_cast<EPackMethod>(dwMethod);
  }

  bool bUnlock = false;
  if(params.has("unlockmanual"))
    bUnlock = true;

  if (pUser->getPackage().rollReward(rewardID, eMethod, bShow, true, false, dwDoubleReward, bUnlock, source) == false)
  {
    XERR << "[GM指令-奖励]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未包含id" << XEND;
    return false;
  }  

  if (params.has("buff"))
  {
    DWORD buffId = params.getTableInt("buff");
    pUser->m_oBuff.add(buffId, pUser);
  }
  return true;
}

bool GMCommandRuler::setattr(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  Attribute* pAttr = pUser->getAttribute();
  if (pAttr == nullptr)
    return false;

  // get param
  DWORD attr = 0;
  float value = 0;
  if (!params.has("attrtype") || !params.has("attrvalue"))
    return false;

  attr = params.getTableInt("attrtype");
  value = params.getTableInt("attrvalue");

  // check param
  if (EAttrType_IsValid(attr) == false)
    return false;
  if (attr <= EATTRTYPE_MIN || attr >= EATTRTYPE_MAX)
    return false;
  EAttrType eType = static_cast<EAttrType>(attr);

  pUser->setGMAttr(eType, value);
  pUser->refreshDataAtonce();

  return true;
}

bool GMCommandRuler::dmap(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD dmapid = params.getTableInt("id");
  if (dmapid == 0)
    return false;

  CreateDMapParams stParams;
  stParams.qwCharID = pUser->id;
  stParams.dwRaidID = dmapid;
  stParams.oGuildInfo.set_id(pUser->getGuild().id());

  if (params.has("team") == true)
  {
    const TMapGTeamMember& mapMember = pUser->getTeam().getTeamMemberList();
    for (auto &m : mapMember)
      stParams.vecMembers.push_back(m.first);
  }
  return SceneManager::getMe().createDScene(stParams);
}

bool GMCommandRuler::gocity(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr || pUser->isAlive() == false)
    return false;

  DWORD zoneid = params.has("zoneid") == false ? 0 : params.getTableInt("zoneid");
  DWORD mapid = params.has("mapid") == false ? pUser->getUserSceneData().getSaveMap() : params.getTableInt("mapid");
  DWORD bpindex = params.getTableInt("bp");
  xPos pos;
  if (bpindex != 0)
  {
    const SceneBase* pTargetScene = SceneManager::getMe().getDataByID(mapid);
    if (pTargetScene && pTargetScene->getSceneObject(0))
    {
      const xPos* pPos = pTargetScene->getSceneObject(0)->getBornPoint(bpindex);
      if (pPos != nullptr)
        pos = *pPos;
    }
  }
  
  if (pUser->getScene() && pUser->getScene()->isDPvpScene())
  {
    mapid = pUser->getUserMap().getLastStaticMapID();
    pos = pUser->getUserMap().getLastStaticMapPos();
  }

  if (params.has("x") == true)
    pos.x = params.getTableFloat("x");
  if (params.has("y") == true)
    pos.y = params.getTableFloat("y");
  if (params.has("z") == true)
    pos.z = params.getTableFloat("z");

  if (zoneid == 0)
  {
    pUser->gomap(mapid, GoMapType::GM, pos);
  }
  else
  {
    pUser->getUserZone().gomap(zoneid, mapid, GoMapType::GM, pos);
  }
  return true;
}

bool GMCommandRuler::follower(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;

  // get param
  if (params.has("id"))
  {
    DWORD npcid = params.getTableInt("id");
    if (npcid)
    {
      DWORD num = 1;
      if (params.has("num"))
        num = params.getTableInt("num");

      SFollow sfData;
      if (params.has("spdRatio"))
        sfData.spdRatio = params.getTableFloat("spdRatio");
      if (params.has("behaviours"))
        sfData.behaviours = params.getTableInt("behaviours");
      if (params.has("questid"))
        sfData.questid = params.getTableInt("questid");
      if (params.has("time"))
        sfData.clearTime = params.getTableInt("time") + now();
      sfData.nameID = npcid;
      for (DWORD i = 0; i < num; ++ i)
      {
        entry->m_oFollower.add(sfData);
      }
    }
    else
    {
      entry->m_oFollower.clear();
    }
  }
  else if (params.has("clear"))
  {
    DWORD npcid = params.getTableInt("clear");
    if (npcid)
    {
      std::list<SceneNpc *> list;
      entry->m_oFollower.get(list);
      for (auto &it : list)
      {
        if (npcid == it->getNpcID())
        {
          entry->m_oFollower.del(it->getTempID());
          it->removeAtonce();
        }
      }
    }
  }
  return true;
}

bool GMCommandRuler::rideon(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  DWORD mountid = 0;
  if (!params.has("mountid"))
    return false;

  mountid = params.getTableInt("mountid");

  return pUser->getPackage().ride(ERIDETYPE_ON, mountid);
}

bool GMCommandRuler::rideoff(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  return pUser->getPackage().ride(ERIDETYPE_OFF);
}

/*bool GMCommandRuler::stage(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  std::string act = params.getTableString("act");
  if (act == "start")
  {
    pUser->m_stage.start(params.getTableInt("id"), params.getTableInt("step"), params.getTableInt("type"));
  }
  else if (act == "get")
  {
    pUser->m_stage.getReward(params.getTableInt("id"), params.getTableInt("star"));
  }
  else
  {
  }

  return true;
}*/

bool GMCommandRuler::buff(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;

  if (!params.has("id") && !params.has("ids") && !params.has("multilayer"))
    return false;

  TVecDWORD vecIDs;
  if (params.has("id"))
  {
    vecIDs.push_back(params.getTableInt("id"));
  }
  else if (params.has("ids"))
  {
    xLuaData ids = params.getData("ids");
    auto func = [&](const string& key, xLuaData& data)
    {
      vecIDs.push_back(data.getInt());
    };
    ids.foreach(func);
  }

  QWORD lasttime = 0;
  QWORD endtime = 0;
  if (params.has("lasttime"))
  {
    lasttime = params.getTableInt("lasttime") * ONE_THOUSAND;
    endtime = lasttime + xTime::getCurMSec();
  }

  // 对别人使用
  if (params.getTableInt("GM_Target") == 1)
  {
    for (auto &v : vecIDs)
    {
      QWORD userid = params.getTableQWORD("id1");
      xSceneEntryDynamic* pEntry = xSceneEntryDynamic::getEntryByID(userid);
      if (pEntry)
      {
        pEntry->m_oBuff.add(v, entry, 0, 0, endtime);
      }
    }
    return true;
  }
  if (params.has("layer"))
  {
    for (auto &v : vecIDs)
    {
      DWORD layer = params.getTableInt("layer");
      entry->m_oBuff.addLayers(v, layer);
    }
    return true;
  }
  if (params.has("multilayer"))
  {
    std::map<DWORD, DWORD> mapBuff2Layer;
    xLuaData layerbuff = params.getData("multilayer");
    auto layerfunc = [&](const string& key, xLuaData& data)
    {
      DWORD buffid = data.getTableInt("id");
      DWORD layer = data.getTableInt("layer");
      mapBuff2Layer.emplace(buffid, layer);
    };
    layerbuff.foreach(layerfunc);

    for(auto &s : mapBuff2Layer)
    {
      entry->m_oBuff.addLayers(s.first, s.second);
#ifdef _DEBUG
  XLOG << "[HP储备库], 玩家:" << entry->name << entry->id << "buff id=" << s.first << "addlayer= " << s.second << XEND;
#endif
    }
  }

  auto bufffunc = [&](xSceneEntryDynamic* entry, TVecDWORD vecID)
  {
    for (auto &v : vecID)
    {
      entry->m_oBuff.add(v, entry, 0, 0, endtime);
    }
  };

  DWORD dwObjects = params.getTableInt("objects");
  if (dwObjects == 0)
  {
    bufffunc(entry, vecIDs);
  }
  else if(dwObjects == 1)
  {
    SceneUser *pUser = (SceneUser *)entry;
    if(pUser != nullptr)
    {
      SceneUser* pVisitUser = SceneUserManager::getMe().getUserByID(pUser->getVisitUser());
      if(pVisitUser != nullptr)
        bufffunc(pVisitUser, vecIDs);
    }
  }
  else if(dwObjects == 2)
  {
    bufffunc(entry, vecIDs);

    SceneUser *pUser = (SceneUser *)entry;
    if(pUser != nullptr)
    {
      SceneUser* pVisitUser = SceneUserManager::getMe().getUserByID(pUser->getVisitUser());
      if(pVisitUser != nullptr)
        bufffunc(pVisitUser, vecIDs);
    }
  }
  else if(dwObjects == 3)
  {
    SceneUser *pUser = (SceneUser *)entry;
    if(pUser != nullptr)
    {
      for (auto &m : pUser->getTeam().getTeamMemberList())
      {
        const TeamMemberInfo& rMember = m.second;
        SceneUser* pMember = SceneUserManager::getMe().getUserByID(rMember.charid());
        if(pMember)
          bufffunc(pMember, vecIDs);
        else
        {
          for (auto &v : vecIDs)
          {
            GiveRewardSessionCmd cmd;
            cmd.set_charid(rMember.charid());
            cmd.set_buffid(v);
            PROTOBUF(cmd, send, len);
            thisServer->sendCmdToSession(send, len);
          }
        }
      }
    }
  }
  else
    XERR << "[GM指令-添加buff]" << entry->id << entry->name << "奖励对象dwObjects : " << dwObjects << "失败,不合法" << XEND;
  return true;
}

bool GMCommandRuler::delbuff(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;

  if (!params.has("id"))
    return false;
  DWORD id = params.getTableInt("id");
  return entry->m_oBuff.del(id);
}

/*bool GMCommandRuler::changeskill(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;

  entry->m_oChangeSkill.add(params);

  return true;
}*/

bool GMCommandRuler::heal(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (pUser == nullptr || pUser->getFighter() == nullptr)
    return false;

  enum EValueType
  {
    EVALUETYPE_MIN = 0,
    EVALUETYPE_RAND,
    EVALUETYPE_PERCENT,
    EVALUETYPE_MAX
  };

  if (params.has("hptype") == true)
  {
    if (entry->isCantHeal()) // 使用治愈类道具无效果
      return true;

    DWORD type = params.getTableInt("hptype");
    if (type <= EVALUETYPE_MIN || type >= EVALUETYPE_MAX)
      return false;
  }
  if (params.has("sptype") == true)
  {
    DWORD type = params.getTableInt("sptype");
    if (type <= EVALUETYPE_MIN || type >= EVALUETYPE_MAX)
      return false;
  }

  DWORD formula = params.getTableInt("formula");
  if (params.has("hptype") == true)
  {
    EValueType eType = static_cast<EValueType>(params.getTableInt("hptype"));
    DWORD value = 0;
    if (eType == EVALUETYPE_RAND)
    {
      DWORD min = params.getData("hpvalue").getTableInt("1");
      DWORD max = params.getData("hpvalue").getTableInt("2");
      value = randBetween(min, max);
      value = LuaManager::getMe().call<float>("CalcItemHealValue", value, entry, formula);
    }
    else if (eType == EVALUETYPE_PERCENT)
    {
      DWORD per = params.getTableInt("hpvalue");
      DWORD rate = 1;
      if (pUser->getScene() && pUser->getScene()->isGvg())
        rate = MiscConfig::getMe().getGuildFireCFG().dwHpRate;
      if (pUser->getScene() && pUser->getScene()->isPVPScene())
      {
        if (params.has("pvp_hpvalue"))
          per = params.getTableInt("pvp_hpvalue");
        rate = MiscConfig::getMe().getPvpCommonCFG().dwHealRate;
      }
      rate = rate ? rate : 1;

      value = static_cast<DWORD>(per / 100.0f * pUser->getAttr(EATTRTYPE_MAXHP) / rate);
    }

    pUser->changeHp(value, pUser, false, true);
    if (params.getTableInt("GM_Target") == 1)
      pUser->playDynamicExpression(EAVATAREXPRESSION_SMILE);

    UserActionNtf cmd;
    cmd.set_charid(pUser->id);
    cmd.set_type(EUSERACTIONTYPE_ADDHP);
    cmd.set_value(value);
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToNine(send, len);
  }

  if (params.has("formula2"))
  {
    formula = params.getTableInt("formula2");
  }
  if (params.has("sptype") == true)
  {
    EValueType eType = static_cast<EValueType>(params.getTableInt("sptype"));
    DWORD value = 0;
    if (eType == EVALUETYPE_RAND)
    {
      DWORD min = params.getData("spvalue").getTableInt("1");
      DWORD max = params.getData("spvalue").getTableInt("2");
      value = randBetween(min, max);
      value = LuaManager::getMe().call<float>("CalcItemHealValue", value, entry, formula);
    }
    else if (eType == EVALUETYPE_PERCENT)
    {
      value = static_cast<DWORD>(params.getTableInt("spvalue") / 100.0f * pUser->getAttr(EATTRTYPE_MAXSP));
    }

    pUser->setSp(value + pUser->getSp());

    xLuaData data;
    data.setData("effect", "Common/64SP_up");
    data.setData("effectpos", 6);
    GMCommandRuler::effect(pUser, data);
  }

  if (params.has("buff") == true)
  {
    const xLuaData& buff = params.getData("buff");
    xLuaData buffCopy = buff;
    auto bufff = [&](const string& key, xLuaData& data)
    {
      pUser->m_oBuff.add(data.getInt());
    };
    buffCopy.foreach(bufff);
  }

  if (params.has("status") == true)
  {
    xLuaData stdata = params.getData("status");
    auto delst = [&](const string& key, xLuaData& data)
    {
      pUser->m_oBuff.delStatus(data.getInt());
    };
    stdata.foreach(delst);
  }
  else if (params.getTableInt("statusrandom") != 0)
  {
    DWORD size = params.getTableInt("statusrandom");
    std::set<DWORD> stSet;
    pUser->m_oBuff.getStatus(stSet);
    DWORD i = 0;
    for (auto &s : stSet)
    {
      if (i >= size)
        break;
      pUser->m_oBuff.delStatus(s);
      ++i;
    }
  }


  return true;
}

bool GMCommandRuler::god(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;
  entry->m_blGod = true;

  GET_USER(entry);
  BasePackage* pFoodPack = pUser->getPackage().getPackage(EPACKTYPE_FOOD);
  if (pFoodPack)
    pFoodPack->show();
  return true;
}

bool GMCommandRuler::killer(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;

  entry->m_blKiller = true;

  return true;
}

bool GMCommandRuler::hideme(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;

//  entry->();

  return true;
}
bool GMCommandRuler::normal(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;

  entry->m_blGod = false;
  entry->m_blKiller = false;
  entry->m_oBuff.delBuffByType(EBUFFTYPE_TRANSFORM);

  return true;
}

bool GMCommandRuler::levelup(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;
  if (SCENE_ENTRY_USER!=entry->getEntryType()) return false;

  SceneUser *pUser = (SceneUser *)entry;

  DWORD num = params.getTableInt("num");
  if (!num) num = 1;
  if (num > 100) num = 100;

  QWORD total = 0;
  for (DWORD i=0; i<num; ++i)
  {
    const SUserBaseLvCFG* pCFG = BaseLevelConfig::getMe().getBaseLvCFG(pUser->getUserSceneData().getRolelv() + i + 1);
    if (pCFG)
      total += pCFG->needExp;
  }
  pUser->addBaseExp(total, ESOURCE_GM);

  return true;
}

bool GMCommandRuler::joblevelup(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;

  if (SCENE_ENTRY_USER!=entry->getEntryType()) return false;

  SceneUser *pUser = (SceneUser *)entry;

  DWORD num = params.getTableInt("num");
  if (!num) num = 1;

  QWORD total = 0;
  for (DWORD i=0; i<num; ++i)
  {
    const SUserJobLvCFG* pCFG = JobLevelConfig::getMe().getJobLvCFG(pUser->getFighter()->getJobLv() + i + 1);
    if (pCFG)
      total += pCFG->needExp;
  }
  pUser->addJobExp(total, ESOURCE_GM);

  return true;
}

bool GMCommandRuler::basetolevel(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;
  if (SCENE_ENTRY_USER!=entry->getEntryType()) return false;

  SceneUser *pUser = (SceneUser *)entry;

  DWORD num = params.getTableInt("num");
  if (!num) num = 1;

  QWORD total = 0;
  DWORD deltLv = num > pUser->getUserSceneData().getRolelv() ? num - pUser->getUserSceneData().getRolelv() : 0;
  for (DWORD i=0; i<deltLv; ++i)
  {
    const SUserBaseLvCFG* pCFG = BaseLevelConfig::getMe().getBaseLvCFG(pUser->getUserSceneData().getRolelv() + i + 1);
    if (pCFG)
      total += pCFG->needExp;
  }
  pUser->addBaseExp(total, ESOURCE_GM);

  return true;
}

bool GMCommandRuler::jobtolevel(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry) return false;

  if (SCENE_ENTRY_USER!=entry->getEntryType()) return false;

  SceneUser *pUser = (SceneUser *)entry;

  DWORD num = params.getTableInt("num");
  if (!num) num = 1;

  QWORD total = 0;
  DWORD deltLv = num > pUser->getFighter()->getJobLv() ? num - pUser->getFighter()->getJobLv() : 0;
  for (DWORD i=0; i<deltLv; ++i)
  {
    const SUserJobLvCFG* pCFG = JobLevelConfig::getMe().getJobLvCFG(pUser->getFighter()->getJobLv() + i + 1);
    if (pCFG)
      total += pCFG->needExp;
  }
  pUser->addJobExp(total, ESOURCE_GM);

  return true;
}

bool GMCommandRuler::killnine(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry || !entry->getScene()) return false;

  DWORD range = params.getTableInt("range");
  DWORD id = params.getTableInt("id");

  if (!range) range = 10;

  xSceneEntrySet set;
  entry->getScene()->getEntryList(entry->getPos(), range, set);

  for (auto it=set.begin(); it!=set.end(); ++it)
  {
    xSceneEntryDynamic *p = (xSceneEntryDynamic *)(*it);
    if (!entry->isMyEnemy(p)) continue;
    if (id)
    {
      SceneNpc* pNpc = dynamic_cast<SceneNpc*>(p);
      if (pNpc != nullptr)
      {
        if (pNpc->getNpcID() != id)
          continue;
      }
    }
    if (entry->canAttack(p))
    {
      p->attackMe(p->getAttr(EATTRTYPE_HP), entry);
      if (p->getAttr(EATTRTYPE_HP) != 0)
      {
        p->setAttr(EATTRTYPE_HP, 0);
        p->setStatus(ECREATURESTATUS_REMOVE);
      }
    }
  }

  return true;
}

bool GMCommandRuler::effect(xSceneEntryDynamic* entry, const xLuaData& params)
{
  /*
  特效播放类型
  1.1
  type = "effect" effect = "Skill/Heal"
  播放在entry脚下位置, 特效跟随entry移动（cmd.epbind = true, cmd.posbind = false)
  1.2
  type = "effect" effect = "Skill/Heal" effecpos = xx
  播放在entry ep挂点位置, 特效跟随entry移动（cmd.epbind = true, cmd.posbind = false)

  2.1
  type = "effect" effect = "Skill/Heal" posbind = 1 (effectpos = xx 或 不填)
  播放方式同1, 特效不随entry移动, 而是绑定在位置(cmd.epbind = true/false, cmd.posbind = true)
  2.2
  type = "effect" effect = "Skill/Heal" pos = {xx,yy,zz}
  播放方式在某个位置, 特效不随entry移动, 而是绑定在位置(cmd.epbind = false, cmd.posbind = true)

  ...
  sync = xx, 同步类型;
  npcid = xx, 播放对象换为9屏内离entry最近的npc(id=xx)
  index = xx, 用于索引
  times = xx, 特效类型, =0 表示循环特效, 默认=1 播放一次
  opt = del,(必填index = xx), 删除index=xx的循环特效(charid + index 确定场景上唯一一组特效)
  save = 1 表示保存特效(仅限循环特效)
  */
  if (!entry || !entry->getScene()) return false;

  Cmd::EffectUserCmd cmd;

  stringstream gmsave;
  gmsave.str("");

  DWORD saven = params.getTableInt("save");
  bool saveflag = (saven != 0);
  gmsave << "effect " << " ";
  //gmsave << "save=" << saven << " ";

  if (params.has("etype"))
  {
    DWORD dwType = params.getTableInt("etype");
    if (EEffectType_IsValid(dwType) && dwType > EEFFECTTYPE_NORMAL)
    {
      cmd.set_effecttype(static_cast<EEffectType>(dwType));
      PROTOBUF(cmd, send, len);
      entry->sendCmdToMe(send, len);
      return true;
    }
    gmsave << "etype=" << dwType << " ";
  }

  cmd.set_opt(EEFFECTOPT_PLAY);
  if (params.has("opt"))
  {
    string sopt = params.getTableString("opt");
    if ("play" == sopt)
      cmd.set_opt(EEFFECTOPT_PLAY);
    else if ("stop" == sopt)
      cmd.set_opt(EEFFECTOPT_STOP);
    else if ("delete" == sopt)
      cmd.set_opt(EEFFECTOPT_DELETE);
  }

  cmd.set_effect(params.getTableString("effect"));
  gmsave << "effect=" << cmd.effect() << " ";

  DWORD npcid = params.getTableInt("npcid");

  DWORD sync = params.getTableInt("sync");
  gmsave << "sync=" << sync << " ";

  bool posbind = params.getTableInt("posbind") != 0;
  gmsave << "posbind=" << params.getTableInt("posbind") << " ";

  cmd.set_epbind(true);
  if (params.has("pos"))
  {
    Cmd::ScenePos *spos = cmd.mutable_pos();
    spos->set_x(params.getData("pos").getTableFloat("1") * 1000);
    spos->set_y(params.getData("pos").getTableFloat("2") * 1000);
    spos->set_z(params.getData("pos").getTableFloat("3") * 1000);

    cmd.set_epbind(false);
    posbind = true;
    gmsave << "pos={" << (float)spos->x()/1000.0f << "," << (float)spos->y()/1000.0f << "," << spos->z()/1000.0f << "} ";
  }
  else if (posbind == true && npcid == 0)
  {
    // 绑定在当前位置
    cmd.set_epbind(false);
    gmsave << "pos={" << entry->getPos().x << "," << entry->getPos().y << "," << entry->getPos().z << "} ";
    Cmd::ScenePos *spos = cmd.mutable_pos();
    spos->set_x(entry->getPos().x * 1000);
    spos->set_y(entry->getPos().y * 1000);
    spos->set_z(entry->getPos().z * 1000);
  }

  if (params.has("effectpos"))
  {
    cmd.set_effectpos(params.getTableInt("effectpos"));
    gmsave << "effectpos=" << cmd.effectpos() << " ";
    cmd.set_epbind(true);
  }

  if (params.has("msec"))
    cmd.set_msec(params.getTableInt("msec"));
  else
    cmd.set_msec(0);
  if (params.has("times"))
    cmd.set_times(params.getTableInt("times"));
  else
    cmd.set_times(1);

  if (params.has("index"))
  {
    cmd.set_index(params.getTableInt("index"));
    gmsave << "index=" << cmd.index() << " ";
  }
  if (params.has("delay"))
  {
    cmd.set_delay(params.getTableInt("delay"));
  }
  if (params.has("skillid"))
  {
    cmd.set_skillid(params.getTableInt("skillid"));
  }

  cmd.set_posbind(posbind);
  cmd.set_charid(entry->id);

  SceneUser* pUser = dynamic_cast<SceneUser*> (entry);
  if (saveflag && pUser != nullptr && cmd.opt() == EEFFECTOPT_PLAY)
  {
    pUser->getUserSceneData().addGMCommand(cmd.index(), gmsave.str());
  }
  else if (pUser != nullptr && cmd.opt() == EEFFECTOPT_DELETE)
  {
    pUser->getUserSceneData().delGMCommand(cmd.index());
  }

  // quest npc
  SceneNpc* npc = dynamic_cast<SceneNpc*> (entry);
  if (npc != nullptr && npc->define.m_oVar.m_qwQuestOwnerID != 0)
  {
    SceneUser* pMaster = SceneUserManager::getMe().getUserByID(npc->define.m_oVar.m_qwQuestOwnerID);
    if (pMaster == nullptr)
      return false;
    PROTOBUF(cmd, send, len);
    pMaster->sendCmdToMe(send, len);
    return true;
  }

  // 离玩家最近的npc播放表情
  xSceneEntryDynamic* pPlayer = entry;

  if (npcid != 0)
  {
    if (pUser == nullptr)
      return false;
    xSceneEntrySet tarSet;
    entry->getScene()->getEntryListInNine(SCENE_ENTRY_NPC, entry->getPos(), tarSet);
    SceneNpc* pNpc = nullptr;
    float minDis = 100.0f;
    for (auto s = tarSet.begin(); s != tarSet.end(); ++s)
    {
      SceneNpc* pt = SceneNpcManager::getMe().getNpcByTempID((*s)->id);
      if (pt == nullptr || pt->getScene() == nullptr || pt->getNpcID() != npcid)
        continue;
      float dis = getDistance(entry->getPos(), pt->getPos());
      if (dis < minDis)
      {
        pNpc = pt;
        minDis = dis;
      }
    }
    if (pNpc == nullptr || pNpc->getScene() == nullptr)
      return false;
    cmd.set_charid(pNpc->id);
    pPlayer = pNpc;

    if (params.getTableInt("save_on_npc") == 1 && cmd.opt() != EEFFECTOPT_DELETE)
    {
      pNpc->setEffect(cmd.effect(), cmd.effectpos());
    }
    else
    {
      pNpc->clearEffect();
    }
  }

  PROTOBUF(cmd, send, len);
  if (pPlayer == nullptr || pPlayer->getScene() == nullptr)
    return false;

  if (sync == 0) // 同步所有玩家
    pPlayer->sendCmdToNine(send, len);
  else if (sync == 1 && pUser) // 仅自己可见
    pUser->sendCmdToMe(send, len);
  else if (sync == 2 && pUser) // 队友可见
  {
    if (pUser->getTeamID() != 0)
    {
      const TMapGTeamMember& mapMember = pUser->getTeam().getTeamMemberList();
      for (auto &m : mapMember)
      {
        QWORD tuserid = m.second.charid();
        SceneUser* ptUser = SceneUserManager::getMe().getUserByID(tuserid);
        if (ptUser == nullptr || ptUser->check2PosInNine(pUser) == false)
          continue;
        ptUser->sendCmdToMe(send, len);
      }
    }
    else
    {
      pUser->sendCmdToMe(send, len);
    }
  }
  return true;
}

bool GMCommandRuler::playaction(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry || !entry->getScene())
  {
    XERR << "[GM指令-表情播放] 失败,对象为空" << XEND;
    return true;
  }

  // 离玩家最近的npc播放表情
  DWORD type = params.getTableInt("style");
  if (type != 1 && type != 2 && type != 3 && type != 4)
  {
    XERR << "[GM指令-表情播放]" << entry->id << entry->name << "播放表情 type :" << type << "失败,不合法" << XEND;
    return true;
  }

  UserActionNtf cmd;
  if (type == 1)
    cmd.set_type(EUSERACTIONTYPE_EXPRESSION);
  else if (type == 2)
    cmd.set_type(EUSERACTIONTYPE_MOTION);
  else if (type == 3)
    cmd.set_type(EUSERACTIONTYPE_NORMALMOTION);
  else if (type == 4)
    cmd.set_type(EUSERACTIONTYPE_GEAR_ACTION);

  DWORD id = params.getTableInt("id");
  if (id == 0)
  {
    XERR << "[GM指令-表情播放]" << entry->id << entry->name << "播放表情 type :" << type << "id :" << id << "失败,id不合法" << XEND;
    return true;
  }
  cmd.set_value(id);
  cmd.set_charid(entry->id);

  if (cmd.type() == EUSERACTIONTYPE_MOTION)
    entry->setAction(id);

  if(params.has("savestatus"))
  {
    if(cmd.type() == EUSERACTIONTYPE_GEAR_ACTION)
    {
      SceneNpc* pNpc = nullptr;
      if(entry->getEntryType() == SCENE_ENTRY_NPC)
        pNpc = (SceneNpc *)entry;
      if(pNpc != nullptr)
        pNpc->setGearStatus(id);
    }
    else if (cmd.type() == EUSERACTIONTYPE_MOTION || cmd.type() == EUSERACTIONTYPE_NORMALMOTION)
    {
      if(entry->getEntryType() == SCENE_ENTRY_NPC)
      {
        ((SceneNpc*)entry)->setAction(id);
      }
    }
  }

  DWORD npcid = params.getTableInt("npcid");
  // 最近的指定id npc播放
  if (npcid != 0)
  {
    GET_USER(entry);
    if (pUser == nullptr)
      return true;
    xSceneEntrySet tarSet;
    entry->getScene()->getEntryListInNine(SCENE_ENTRY_NPC, entry->getPos(), tarSet);
    SceneNpc* pNpc = nullptr;
    float minDis = 100.0f;
    for (auto s = tarSet.begin(); s != tarSet.end(); ++s)
    {
      SceneNpc* pt = SceneNpcManager::getMe().getNpcByTempID((*s)->id);
      if (pt == nullptr || pt->getScene() == nullptr || pt->getNpcID() != npcid)
        continue;
      float dis = getDistance(entry->getPos(), pt->getPos());
      if (dis < minDis)
      {
        pNpc = pt;
        minDis = dis;
      }
    }
    if (pNpc == nullptr || pNpc->getScene() == nullptr)
    {
      XERR << "[GM指令-表情播放]" << entry->id << entry->name << "播放表情 type :" << type << "id :" << id << "失败,未找到对应npc" << XEND;
      return true;
    }
    cmd.set_charid(pNpc->id);
    PROTOBUF(cmd, send, len);

    DWORD sync = params.getTableInt("sync");
    if (sync == 0) // 同步所有玩家
      pNpc->getScene()->sendCmdToNine(pNpc->getPos(), send, len);
    else if (sync == 1) // 仅自己可见
      pUser->sendCmdToMe(send, len);
    else if (sync == 2) // 队友可见
    {
      const GTeam& rTeam = pUser->getTeam();
      if (rTeam.getTeamID() != 0)
      {
        for (auto &m : rTeam.getTeamMemberList())
        {
          QWORD tuserid = m.second.charid();
          SceneUser* ptUser = SceneUserManager::getMe().getUserByID(tuserid);
          if (ptUser == nullptr)
            continue;
          ptUser->sendCmdToMe(send, len);
        }
      }
    }

    if (params.getTableInt("save_on_npc") == 1)
    {
      pNpc->setAction(id);
    }
    else
    {
      pNpc->setAction(0);
    }
    return true;
  }

  PROTOBUF(cmd, send, len);
  entry->getScene()->sendCmdToNine(entry->getPos(), send, len);
  return true;
}

bool GMCommandRuler::quest(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (params.has("id") == false)
    return false;
  if (params.has("type1") == false)
    return false;

  DWORD questid = params.getTableInt("id");
  DWORD type = params.getTableInt("type1");
  bool bSubmitInclue = params.getTableInt("submit") == 1;
  Quest& oQuest = pUser->getQuest();

  if (type == 1)
    oQuest.runStep(questid);
  else if (type == 2)
    oQuest.acceptQuest(questid, true);
  else if (type == 3)
    oQuest.abandonGroup(questid, bSubmitInclue);
  else if (type == 4)
  {
    const TMapQuestCFGEx mapCFG = QuestManager::getMe().getList();
    for (auto &v : mapCFG)
      oQuest.acceptQuest(v.first, true);
  }
  else if (type == 5)
    return oQuest.addForbidQuest(questid);
  else if (type == 6)
    oQuest.removeForbidQuest(questid);
  else if (type == 7)
    return oQuest.addChoiceQuest(questid);
  else if (type == 8)
    return oQuest.removeChoiceQuest(questid);

/*#ifdef _DEBUG
  oQuest.sendGuildQuestList();
#endif*/
  return true;
}

bool GMCommandRuler::summon(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (!entry || !entry->getScene()) return false;

  xPos dest;
  bool bSuccess = false;
  DWORD dwCount = 0;
  while (++dwCount < 30)
  {
    if (entry->getScene()->getRandPos(entry->getPos(), 1.0f, dest) == false)
    {
      XERR << "[GMCommandRuler::summon id =" << entry->id << "name =" << entry->name << "mapid =" << entry->getScene()->getMapID() << "randpos failed]" << XEND;
      dest = entry->getPos();
      bSuccess = false;
      break;
    }

    if (getDistance(entry->getPos(), dest) > 1.5f)
    {
      dest = entry->getPos();
      continue;
    }

    bSuccess = true;
    break;
  }
  if (!bSuccess)
      XERR << "[GMCommandRuler::summon id =" << entry->id << "name =" << entry->name << "mapid =" << entry->getScene()->getMapID() << "randpos failed after 30 times]" << XEND;

  // get params
  NpcDefine def;
  def.load(params);
  def.setPos(dest);
  DWORD num = 1;
  if (params.has("num"))
    num = params.getTableInt("num");

  vector<tuple<DWORD, DWORD, DWORD>> vecIDs;
  xLuaData idrange = params.getData("idrange");
  auto idrangef = [&](const string& key, xLuaData& data)
  {
    DWORD dwMin = data.getTableInt("1");
    DWORD dwMax = data.getTableInt("2");
    DWORD dwRate = data.getTableInt("3");

    vecIDs.push_back(make_tuple(dwMin, dwMax, dwRate));
    //XDBG("min = %u, max = %u, rate = %u", dwMin, dwMax, dwRate);
  };
  idrange.foreach(idrangef);

  TSetDWORD setExcludeID;
  xLuaData excludeid = params.getData("excludeid");
  auto excludeidf = [&](const string& key, xLuaData& data)
  {
    setExcludeID.insert(data.getInt());
  };
  excludeid.foreach(excludeidf);

  // rand id if not empty
  if (vecIDs.empty() == false)
  {
    DWORD dwLastRate = 0;
    for (auto &v : vecIDs)
    {
      get<2>(v) += dwLastRate;
      dwLastRate = get<2>(v);
    }

    //for (auto v : vecIDs)
    //  XDBG("min1 = %u, max1 = %u, rate1 = %u", get<0>(v), get<1>(v), get<2>(v));

    auto randID = [](DWORD dwMin, DWORD dwMax, const TSetDWORD& setIDs) -> DWORD
    {
      DWORD dwCount = 0;
      while (dwCount++ < 30)
      {
        DWORD dwID = randBetween(dwMin, dwMax);
        if (setIDs.find(dwID) != setIDs.end())
          continue;
        return dwID;
      }
      return 0;
    };

    DWORD dwRand = randBetween(0, dwLastRate);
    DWORD dwID = 0;
    for (auto v : vecIDs)
    {
      if (dwRand < get<2>(v))
      {
        dwID = randID(get<0>(v), get<1>(v), setExcludeID);
        if (dwID != 0)
        {
          def.setID(dwID);
          //XDBG("id = %u rand = %u total = %u", def.getID(), dwRand, dwLastRate);
          break;
        }
      }
    }
  }
  //XDBG("------------------------------------------------");
  /*if (params.has("idmin") == true && params.has("idmax") == true)
  {
    DWORD min = params.getTableInt("idmin");
    DWORD max = params.getTableInt("idmax");
    DWORD npcid = randBetween(min, max);
    def.setID(npcid);
  }*/

  //SceneNpcManager::getMe().createTreasureNpc(entry->getScene(), def, ETREETYPE_GOLD);
  bool bPriAttackOwner = (params.getTableInt("pri_attack_owner") == 1 && def.getSearch() != 0 && entry->getEntryType() == SCENE_ENTRY_USER);

  for (DWORD n = 0; n < num; ++n)
  {
    SceneNpc* npc = SceneNpcManager::getMe().createNpc(def, entry->getScene());
    if (npc == nullptr)
    {
      XERR << "[GMCommandRuler::summon id =" << entry->id << "name =" << entry->name << "create npc =" << def.getID() << "error]" << XEND;
      return false;
    }
    if (bPriAttackOwner)
      npc->m_ai.setPriAttackUser(entry->id);
  }

  return true;
}

bool GMCommandRuler::setColor(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  string type;
  DWORD value = 0;
  if (!params.has("target"))
    return false;
  type = params.getTableString("target");

  if (!params.has("color"))
    return false;
  value = params.getTableInt("color");

  if (type == "body")
  {
    EPackType pkgType = EPACKTYPE_FASHIONEQUIP;
    if (params.has("pkgtype"))
      pkgType = static_cast<EPackType>(params.getTableInt("pkgtype"));
    return pUser->getPackage().colorBody(pkgType, value);
  }
  if (type == "hair")
  {
    pUser->getHairInfo().useColorFree(value);
    return true;
  }

  return false;
}

bool GMCommandRuler::setSysTime(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("day"))
  {
    std::stringstream stream;
    stream << params.getTableString("day");
    if (params.has("time"))
    {
      stream << " " << params.getTableString("time");
    }
      if (xTime::setTime(stream.str()))
    {
      XLOG << "[GM]" << pUser->name << "设置系统时间" << stream.str() << XEND;
      Cmd::ServerTimeSystemCmd message;
      message.set_adjust(xTime::adjust);
      PROTOBUF(message, send, len);
      thisServer->sendCmdToSuper(send, len);
    }
  }

  return true;
}

bool GMCommandRuler::menu(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("id") == false)
    return false;

  auto open = [](SceneUser* pUser, const SMenuCFG& rCFG, bool bAnim) -> bool
  {
    if (pUser == nullptr)
      return false;

    if (pUser->getMenu().isOpen(rCFG.id) == true)
      return false;

    pUser->getMenu().m_setValidMenus.insert(rCFG.id);
    pUser->getMenu().processMenuEvent(rCFG.id);

    NewMenu cmd;
    cmd.set_animplay(bAnim);
    cmd.add_list(rCFG.id);
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
    return true;
  };

  DWORD id = params.getTableInt("id");
  if (id == 0)
  {
    const TMapMenuCFG& mapCFG = MenuConfig::getMe().getMenuList();
    for (auto m = mapCFG.begin(); m != mapCFG.end(); ++m)
      open(pUser, m->second, false);
  }
  else
  {
    const SMenuCFG* pCFG = MenuConfig::getMe().getMenuCFG(id);
    if (pCFG == nullptr)
      return false;

    if (open(pUser, *pCFG, true) == false)
      return false;
  }

  pUser->getEvent().onMenuOpen(0);
  return true;
}

/*bool GMCommandRuler::testbuff(xSceneEntryDynamic* entry, const xLuaData& params)
{
  xSceneEntrySet uSet;
  entry->getScene()->getEntryListInBlock(SCENE_ENTRY_NPC, entry->getPos(), 5, uSet);
  for(auto m = uSet.begin(); m != uSet.end(); ++m)
  {
    SceneNpc *pNpc = dynamic_cast<SceneNpc*>((*m));
      if (pNpc)
      {
        pNpc->m_oBuff.add(entry, params);
        break;
      }
  }
  return true;
}*/
bool GMCommandRuler::restart(xSceneEntryDynamic* entry, const xLuaData& params)
{
#ifndef _DEBUG
  return true;
#endif

  pid_t pid = fork();
  if(pid < 0)
  {
    XERR << "[重启服务器],fork error" << XEND;
    return true;
  }
  else if (pid == 0)
  {
    char path[256] = {0};
    if (getcwd(path,256))
    {
      std::string exestr;
      exestr = exestr + path;
      exestr = exestr + "/";
      exestr = exestr + "restartSH";

      execl("/bin/sh", "/bin/sh", "-c", exestr.c_str(), nullptr);
    }
  }

  return true;
}

bool GMCommandRuler::carrier(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("id") == false)
    return false;
  DWORD dwNeedAct = 1;
  if (params.has("act"))
    dwNeedAct = params.getTableInt("act");

  if (dwNeedAct > 1)
    dwNeedAct = 1;

  DWORD invite = 1;
  if (params.has("membertype"))
  {
    if (params.getTableInt("membertype") == 1)
      invite = 0;
  }

  pUser->m_oCarrier.create(params.getTableInt("id"), params.getTableInt("line"), dwNeedAct, invite);

  if (params.has("assemble"))
  {
    pUser->m_oCarrier.changeAssemble(params.getTableInt("assemble"));
  }

  return true;
}

bool GMCommandRuler::addmount(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("id") == false)
    return false;

  DWORD id = params.getTableInt("id");
  ItemInfo oItem;
  oItem.set_id(id);
  oItem.set_count(1);
  oItem.set_source(ESOURCE_GM);
  return pUser->getPackage().addItem(oItem, EPACKMETHOD_AVAILABLE);
}

bool GMCommandRuler::getHair(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("id") == false)
    return false;
  DWORD id = params.getTableInt("id");
  pUser->getHairInfo().addNewHair(id);
  return true;
}

bool GMCommandRuler::getEye(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("id") == false)
    return false;
  DWORD id = params.getTableInt("id");
  pUser->getEye().addNewEye(id);
  return true;
}

bool GMCommandRuler::testBuff(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (params.has("id") == false)
    return false;
  DWORD id = params.getTableInt("id");
  DWORD skillid = params.getTableInt("id2");
  pUser->m_oBuff.add(id, entry, skillid);
  return true;
}

bool GMCommandRuler::gear(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (params.has("id") == false)
    return false;

  if (!pUser->getScene()) return false;

  if (params.has("map"))
  {
    if ((int)pUser->getScene()->getMapID()!=params.getTableInt("map"))
      return false;
  }

  pUser->getScene()->m_oGear.set(params.getTableInt("id"), params.getTableInt("state"), pUser);
  return true;
}

bool GMCommandRuler::showboss(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (!pUser->getScene()) return false;

  xSceneEntrySet set;
  pUser->getScene()->getAllEntryList(SCENE_ENTRY_NPC, set);
  for (auto &it : set)
  {
    SceneNpc *npc = (SceneNpc *)it;
    if (npc->isBoss())
    {
      Cmd::BossPosUserCmd message;
      ScenePos *p = message.mutable_pos();
      p->set_x(npc->getPos().getX());
      p->set_y(npc->getPos().getY());
      p->set_z(npc->getPos().getZ());
      PROTOBUF(message, send, len);
      pUser->sendCmdToMe(send, len);
      pUser->goTo(npc->getPos());
    }
  }
  return true;
}

bool GMCommandRuler::cleancorpse(xSceneEntryDynamic* entry, const xLuaData& params)
{
  /*GET_USER(entry);

  if (!pUser->getScene()) return false;

  xSceneEntrySet set;
  pUser->getScene()->getAllEntryList(SCENE_ENTRY_NPC, set);

  for (auto &it : set)
  {
    PurifyNpc *npc = dynamic_cast<PurifyNpc*> (it);
    if (npc && npc->needPurify())
    {
      pUser->purifyRaidBoss(npc->id);
    }
  }*/
  return false;
}


bool GMCommandRuler::openui(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  OpenUI cmd;
  cmd.set_id(params.getTableInt("id"));
  cmd.set_ui(params.getTableInt("param"));
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);

  return true;
}

bool GMCommandRuler::exit_go(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (!pUser->getScene()) return false;

  const SceneObject *pObject = pUser->getScene()->getSceneObject();
  if (!pObject) return false;
  const ExitPoint* pPoint = pObject->getExitPoint(params.getTableInt("id"));
  if (pPoint == nullptr)
    return false;

  pUser->m_oMove.doExit(*pPoint);
  return true;
}

bool GMCommandRuler::exit_visible(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (!pUser->getScene()) return false;
  if (!pUser->getScene()->isDScene()) return false;

  const SceneObject *pObject = pUser->getScene()->getSceneObject();
  if (!pObject) return false;
  const ExitPoint* pPoint = pObject->getExitPoint(params.getTableInt("id"));
  if (pPoint == nullptr)
    return false;
  Cmd::ExitPointState message;
  message.set_exitid(pPoint->m_dwExitID);
  if (pPoint->m_dwPrivate)
  {
    pUser->m_oGear.setExitState(pUser->getScene()->getMapID(), pPoint->m_dwExitID, params.getTableInt("visible"));
    message.set_visible(pPoint->isVisible(pUser));
    PROTOBUF(message, send, len);
    pUser->sendCmdToMe(send, len);
  }
  else
  {
    pUser->getScene()->m_oExit.set(pPoint->m_dwExitID, params.getTableInt("visible"));
    message.set_visible(params.getTableInt("visible"));
    PROTOBUF(message, send, len);
    MsgManager::sendMapCmd(pUser->getScene()->id, send, len);
  }

  return true;
}

bool GMCommandRuler::addpurify(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  DWORD value = params.getTableInt("value");
  pUser->addPurify(value, false);
  return true;
}

bool GMCommandRuler::finishQuest(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (params.has("id") == false)
    return false;

  DWORD id = params.getTableInt("id");
  pUser->getQuest().finishBigQuest(id, params.getTableInt("refresh") == 1);
  return true;
}

bool GMCommandRuler::finishGroup(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (params.has("id") == false)
    return false;
  if (params.has("groupid") == false)
    return false;

  DWORD id = params.getTableInt("id") * 10000 + params.getTableInt("groupid");
  if (pUser->getQuest().finishQuest(id) == false)
    return false;

  pUser->getQuest().acceptNewQuest();
  return true;
}

bool GMCommandRuler::addskill(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (params.has("id") == false)
    return false;
  bool bManual = params.getTableInt("manual") == 1;

  DWORD sourceid = params.getTableInt("source");

  // 用于前端区分唯一id
  if (sourceid == 0)
    sourceid = SKILL_SOURCEID;
  DWORD id = params.getTableInt("id");
  return pUser->addSkill(id, sourceid, bManual ? ESOURCE_NORMAL : ESOURCE_EQUIP);
}

bool GMCommandRuler::addpartner(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (params.has("id") == false)
    return false;

  DWORD id = params.getTableInt("id");
  return pUser->getPet().setPartnerID(id);
}

bool GMCommandRuler::removepartner(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  return pUser->getPet().setPartnerID(0);
}

bool GMCommandRuler::npcfunction(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (params.has("npctype") == false)
    return false;
/*
  if (params.has("param") == false)
    return false;
*/
  //send npcfunction cmd to client
  Cmd::CallNpcFuncCmd cmd;
  cmd.set_type(params.getTableInt("npctype"));
  cmd.set_funparam(params.getTableString("param"));
  SceneUser* pUser = dynamic_cast<SceneUser*>(entry);
  if (nullptr != pUser)
  {
    PROTOBUF(cmd, send, len);
    pUser->sendCmdToMe(send, len);
  }
  return true;
}

bool GMCommandRuler::modelshow(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (params.has("type2") == false)
    return false;
  if (params.has("param") == false)
    return false;

  ModelShow cmd;
  cmd.set_type(params.getTableInt("type2"));
  cmd.set_data(params.getTableString("param"));
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  return true;
}

/*bool GMCommandRuler::followleader(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;
  SceneUser* pMember = SceneUserManager::getMe().getUserByAccID(params.getTableInt("accid"));
  if (nullptr != pMember && pUser->getTeamInfo().teamid() != 0)
  {
    if (pUser->getTeam()->getMember(pMember->id) != nullptr && pUser->getUserSceneData().getOnlineMapID() == pMember->getUserSceneData().getOnlineMapID())
      pUser->gomap(pMember->getUserSceneData().getOnlineMapID(), GoMapType::GMFollow, pMember->getPos());
  }
  return true;
}*/

bool GMCommandRuler::sound_effect(xSceneEntryDynamic * entry, const xLuaData &params)
{
  if (nullptr==entry || nullptr==entry->getScene())
    return false;
  Cmd::SoundEffectCmd cmd;
  Cmd::ScenePos * spos = cmd.mutable_pos();
  if (nullptr == spos)
    return false;

  if (params.has("pos"))
  {
    std::string str = params.getTableString("pos");
    if (string::npos == str.find('{'))
    {
      spos->set_x(params.getData("pos").getTableInt("1"));
      spos->set_y(params.getData("pos").getTableInt("2"));
      spos->set_z(params.getData("pos").getTableInt("3"));
    }
    else
    {
      stringstream ss;
      ss<<str;
      char ch;
      ss>>ch;
      int x = 0, y = 0,z = 0;
      ss>>x,ss>>ch,ss>>y,ss>>ch,ss>>z;
      spos->set_x(x);
      spos->set_y(y);
      spos->set_z(z);
    }
  }
  else
  {
    spos->set_x(entry->getPos().getX());
    spos->set_y(entry->getPos().getY());
    spos->set_z(entry->getPos().getZ());
  }
  cmd.set_se(params.getTableString("se"));
/*
  if (params.has("opt"))
  {
    string sopt = params.getTableString("opt");
    if ("play" == sopt)
      cmd.set_opt(EEFFECTOPT_PLAY);
    else if ("stop" == sopt)
      cmd.set_opt(EEFFECTOPT_STOP);
    else if ("delete" == sopt)
      cmd.set_opt(EEFFECTOPT_DELETE);
  }
*/
  PROTOBUF(cmd, send, len);

  bool bSync = params.getTableInt("sync") == 1;
  if (bSync)
    entry->sendCmdToNine(send, len);
  else
    entry->sendCmdToMe(send, len);

  return true;
}

bool GMCommandRuler::clearpack(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  DWORD type = params.getTableInt("type");
  if (type <= EPACKTYPE_MIN || type >= EPACKTYPE_MAX)
    return false;
  EPackType eType = static_cast<EPackType>(type);

  BasePackage* pPack = pUser->getPackage().getPackage(eType);
  if (pPack == nullptr)
    return false;
  pPack->clear();
  return true;
}

bool GMCommandRuler::sound_bgm(xSceneEntryDynamic * entry, const xLuaData &params)
{
  if (nullptr == entry)
    return false;

  Cmd::ChangeBgmCmd cmd;

  Cmd::EBgmType type = EBGM_TYPE_QUEST;
  if (params.has("bgm"))
  {
    cmd.set_bgm(params.getTableString("bgm"));
  }
  cmd.set_times(1); // 默认播放一次
  if (params.has("times"))
  {
    cmd.set_times(params.getTableInt("times"));
  }
  if (params.has("play"))
  {
    cmd.set_play(params.getTableInt("play"));
  }
  if (params.has("type1"))
  {
    type = static_cast<EBgmType>(params.getTableInt("type1"));
  }

  cmd.set_type(type);
  PROTOBUF(cmd, send, len);
  SceneUser *pUser = dynamic_cast<SceneUser*>(entry);
  if (nullptr != pUser)
    pUser->sendCmdToMe(send, len);

  return true;
}

bool GMCommandRuler::gametime(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  if (params.has("set") == false)
    return false;

  pUser->setOwnTime(params.getTableInt("set"), true);
  return true;
}

bool GMCommandRuler::resetgametime(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  pUser->setOwnTime(0);
  return true;
}

bool GMCommandRuler::checkgametime(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  DWORD dwFrom = params.getTableInt("from");
  DWORD dwTo = params.getTableInt("to");

  DWORD dwGameTimeFrom = dwFrom * 3600;
  DWORD dwGameTimeTo = dwTo * 3600;
  DWORD dwUserGameTime = pUser->getOwnTime();

  return dwGameTimeFrom <= dwUserGameTime && dwUserGameTime < dwGameTimeTo;
}

bool GMCommandRuler::checkweather(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD dwWeather = params.getTableInt("weather");
  DWORD dwUserWeather = pUser->getOwnWeather();
  if (dwUserWeather == 0)
  {
    Scene* pScene = pUser->getScene();
    if (pScene != nullptr)
      dwUserWeather = pScene->getWeather();
  }

  return dwWeather == dwUserWeather;
}

bool GMCommandRuler::setsky(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD dwSky = params.getTableInt("id");
  DWORD all = params.getTableInt("all");
  Scene* pScene = nullptr;
  pScene = pUser->getScene();
  if (!all)
  {
    if (dwSky == 0)
    {      
      if (pScene != nullptr && pScene->base != nullptr)
        dwSky = pScene->getSky();
    }
    if (dwSky == 0)
      return false;
    pUser->setOwnSky(dwSky, true);
  }
  else
  {
    return GMCommandRuler::scene_sky(pScene, params);
  }

  return true;
}

bool GMCommandRuler::setweather(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD dwWeather = params.getTableInt("id");
  DWORD all = params.getTableInt("all");
  if (!all)
  {
    if (dwWeather == 0)
      return false;

    pUser->setOwnWeather(dwWeather, true);
  }
  else
  {
    Scene* pScene = pUser->getScene();
    return GMCommandRuler::scene_weather(pScene, params);
  }
  return true;
}

bool GMCommandRuler::setenv(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  Scene* pScene = pUser->getScene();
  return GMCommandRuler::setenv(pScene, params);
}

bool GMCommandRuler::randPos(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (pUser == nullptr || pUser->getScene() == nullptr)
    return false;

  std::list<SceneUser*> userlist;
  userlist.push_back(pUser);
  if (params.getTableInt("include_follower"))
  {
    std::set<SceneUser*> userset = pUser->getTeamSceneUser();
    userset.erase(pUser);
    for (auto &s : userset)
    {
      if (s->getUserSceneData().getFollowerID() == pUser->id)
        userlist.push_back(s);
    }
  }

  xPos oPos;
  for (auto &user : userlist)
  {
    if(user->getDressUp().getDressUpStatus() != 0)
      continue;

    const SEffectPath& configPath = MiscConfig::getMe().getEffectPath();
    xLuaData data;
    data.setData("type", "effect");
    data.setData("effect", configPath.strLeaveSceneEffect);
    data.setData("posbind", 1);
    GMCommandRuler::getMe().execute(user, data);
    xLuaData sound;
    sound.setData("type", "sound_effect");
    sound.setData("sync", 1);
    sound.setData("se", configPath.strLeaveSceneSound);
    GMCommandRuler::getMe().execute(user, sound);

    if (oPos.empty())
    {
      if (params.has("x") == true || params.has("y") == true || params.has("z") == true)
      {
        oPos.x = params.getTableFloat("x");
        oPos.y = params.getTableFloat("y");
        oPos.z = params.getTableFloat("z");
        user->getScene()->getValidPos(oPos);
      }
      else
      {
        user->getScene()->getRandPos(oPos);
      }
      user->goTo(oPos);
    }
    else
    {
      if (user->isAlive())
      {
        xPos mypos;
        user->getScene()->getRandPos(oPos, 4, mypos);
        user->goTo(mypos);
        MsgManager::sendMsg(user->id, 356, MsgParams(pUser->name));
        XLOG << "[GM-苍蝇翅膀], 玩家:" << user->name << user->id << "跟随队友:" << pUser->name << pUser->id << "一起瞬移" << XEND;
      }
    }

    if (params.has("buff"))
    {
      DWORD buff = params.getTableInt("buff");
      user->m_oBuff.add(buff);
    }
    GMCommandRuler::getMe().execute(user, data);
    GMCommandRuler::getMe().execute(user, sound);
  }

  return true;
}

bool GMCommandRuler::savemap(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  pUser->getUserSceneData().setSaveMap(pUser->getUserSceneData().getOnlineMapID());
  return true;
}

bool GMCommandRuler::shakescreen(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (nullptr == pUser)
    return false;

  Cmd::ShakeScreen cmd;
  if (params.has("amplitude"))
    cmd.set_maxamplitude(params.getTableInt("amplitude"));
  if (params.has("msec"))
    cmd.set_msec(params.getTableInt("msec"));
  if (params.has("shaketype"))
    cmd.set_shaketype(params.getTableInt("shaketype"));

  PROTOBUF(cmd, send, len);
  DWORD dwSync = params.getTableInt("sync");
  if (0 == dwSync)
    pUser->sendCmdToNine(send, len);
  else if ( 1 == dwSync)
    pUser->sendCmdToMe(send, len);
  else if ( 2 == dwSync)
  {
    const GTeam& rTeam = pUser->getTeam();
    if (rTeam.getTeamID() != 0)
    {
      for (auto &m : rTeam.getTeamMemberList())
      {
        QWORD tuserid = m.second.charid();
        SceneUser* ptUser = SceneUserManager::getMe().getUserByID(tuserid);
        if (ptUser == nullptr || pUser->check2PosInNine(ptUser) == false)
          continue;
        ptUser->sendCmdToMe(send, len);
      }
    }
  }

  return true;
}

bool GMCommandRuler::entertower(xSceneEntryDynamic * entry, const xLuaData &params)
{
  //GET_USER(entry);

  //DWORD dwLayer = params.getTableInt("layer");

  //SceneTower::getMe().onUserEnter(pUser, dwLayer);

  return true;
}

bool GMCommandRuler::useskill(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (params.has("id") == false)
    return false;

  const BaseSkill* pSkillCFG = SkillManager::getMe().getSkillCFG(params.getTableInt("id"));
  if (pSkillCFG == nullptr)
    return false;

  xSceneEntryDynamic* target = nullptr;
  if (params.getTableInt("GM_Target") == 1)
  {
    target = xSceneEntryDynamic::getEntryByID(params.getTableQWORD("id1"));
  }
  target = target ? target : pUser;
  if (pUser->m_oSkillProcessor.useBuffSkill(pUser, target, pSkillCFG->getSkillID(), true) == false)
  {
    XLOG << "[GM-useskill], 玩家gm指令释放技能失败" << pUser->name << pUser->id << "技能:" << pSkillCFG->getSkillID() << XEND;
    return false;
  }

  return true;
}

bool GMCommandRuler::delstatus(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (params.has("status") == false)
    return false;

  DWORD status = params.getTableInt("status");
  entry->m_oBuff.delStatus(status);
  return true;
}

bool GMCommandRuler::gopvp(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  DWORD dwPvpMap = pUser->getUserSceneData().getOnlineMapID() + 7000;
  pUser->gomap(dwPvpMap, GoMapType::GoPVP, xPos());
  return true;
}

bool GMCommandRuler::rewards(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  EPackMethod eMethod = EPACKMETHOD_NOCHECK;
  bool bShow = false;
  if (!params.has("id"))
    return false;
  TVecDWORD vecRewardID;
  xLuaData data = params.getData("id");
  auto fData = [&vecRewardID](const string &key, xLuaData &rData)
  {
    DWORD dwID = rData.getInt();
    if (dwID == 0)
      return;
    vecRewardID.push_back(dwID);
  };
  data.foreach(fData);
  if (vecRewardID.size() < 1)
    return false;
  if (params.has("method"))
  {
    DWORD dwMethod = params.getTableInt("method");
    if (dwMethod <= EPACKMETHOD_MIN || dwMethod >= EPACKMETHOD_MAX)
      return false;
    eMethod = static_cast<EPackMethod>(dwMethod);
  }

  bShow = params.getTableInt("show") == 0;
  TVecItemInfo vecItemInfo;
  for (auto it = vecRewardID.begin(); it != vecRewardID.end(); ++it)
  {
    if (pUser->getPackage().rollReward(*it, eMethod, bShow) == false)
      XERR << "[GMCommandRuler::rewards] id =" << pUser->id << "name =" << pUser->name << "roll reward =" << *it << "error" << XEND;
  }
  return true;
}

bool GMCommandRuler::fakedead(xSceneEntryDynamic * entry, const xLuaData &params)
{
  if (entry == nullptr)
    return false;
  entry->setStatus(ECREATURESTATUS_FAKEDEAD);
  entry->refreshDataAtonce();
  return true;
}

bool GMCommandRuler::addinterlocution(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD dwID = params.getTableInt("id");
  const SInterlocution* pCFG = TableManager::getMe().getInterCFG(dwID);
  if (pCFG == nullptr)
    return false;

  pUser->getInterlocution().addInterlocution(dwID, ESOURCE_MIN);
  return true;
}

bool GMCommandRuler::resetattrpoint(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  return pUser->resetAttrPoint();
}

bool GMCommandRuler::setattrpoint(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  // get param
  DWORD point = 0;
  if (!params.has("num"))
    return false;
  point = params.getTableInt("num");

  // set point
  SceneFighter* pFighter = pUser->getFighter();
  if (pFighter == nullptr)
    return false;
  pFighter->setTotalPoint(point);
  return true;
}

bool GMCommandRuler::resetskillpoint(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  SceneFighter* pFighter = pUser->getFighter();
  if (pFighter != nullptr)
    pFighter->getSkill().resetSkill();
  return true;
}

bool GMCommandRuler::setskillpoint(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  if (!params.has("num"))
    return false;
  DWORD num = params.getTableInt("num");  

  SceneFighter* pFighter = pUser->getFighter();
  if (pFighter != nullptr)
    pFighter->getSkill().setSkillPoint(num, ESOURCE_GM);
  return true;
}

bool GMCommandRuler::npcscale(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD dwNpcID = params.getTableInt("id");
  float fScale = params.getTableFloat("scale");
  if (dwNpcID == 0 || fScale == 0.0f)
    return false;

  Scene* pScene = pUser->getScene();
  if (pScene == nullptr)
    return false;

  xSceneEntrySet set;
  pScene->getEntryListInBlock(SCENE_ENTRY_NPC, pUser->getPos(), 5, set);
  for (auto s = set.begin(); s != set.end(); ++s)
  {
    SceneNpc* pNpc = dynamic_cast<SceneNpc*>(*s);
    if (pNpc == nullptr || pNpc->define.getID() != dwNpcID)
      continue;

    pNpc->setScale(fScale);
    break;
  }

  return true;
}

bool GMCommandRuler::changecarrier(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  //QWORD qwMasterID = params.getTableInt("masterid");
  DWORD dwAssembleID = params.getTableInt("id");
  pUser->m_oCarrier.changeAssemble(dwAssembleID);

  return true;
}

bool GMCommandRuler::scenery(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  const xLuaData &data = params.getData("ids");

  for (auto &it : data.m_table)
  {
    pUser->getScenery().add(it.second.getInt());
  }

  DWORD dwSceneryID = params.getTableInt("id");
  if (dwSceneryID != 0)
  {
    pUser->getScenery().add(dwSceneryID);
  }

  return true;
}

bool GMCommandRuler::npctalk(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  DWORD npcid = params.getTableInt("npcid");
  DWORD uniqueID = params.getTableInt("uniqueID");
  DWORD talkid = params.getTableInt("talkid");
  DWORD sync = params.getTableInt("sync");
  bool bVisit = params.getTableInt("visit");
  if (!talkid)
    return false;
  xSceneEntrySet npcSet;
  pUser->getScene()->getEntryListInNine(SCENE_ENTRY_NPC, pUser->getPos(), npcSet);
  TalkInfo cmd;
  if(bVisit)
  {
    SceneNpc* pNpc = pUser->getVisitNpcObj();
    if(pNpc != nullptr)
    {
      cmd.set_guid(pNpc->id);
      cmd.set_talkid(talkid);

      PROTOBUF(cmd, send, len);
      if (sync == 0)
        pNpc->sendCmdToNine(send, len);
      else if (sync == 1)
        pUser->sendCmdToMe(send, len);
    }
  }
  else
  {
    if(!uniqueID && !npcid)
      return false;
    for (auto &s : npcSet)
    {
      SceneNpc* pNpc = dynamic_cast<SceneNpc*> (s);
      if (pNpc == nullptr || 
          (pNpc->getNpcID() != npcid && 
           (pNpc->define.getUniqueID() != uniqueID || !uniqueID)))
        continue;
      cmd.set_guid(pNpc->id);
      cmd.set_talkid(talkid);

      PROTOBUF(cmd, send, len);
      if (sync == 0)
        pNpc->sendCmdToNine(send, len);
      else if (sync == 1)
        pUser->sendCmdToMe(send, len);
    }
  }
  return true;
}

bool GMCommandRuler::usehair(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  if (params.has("id") == false && params.has("color") == false)
    return false;
  DWORD id = params.getTableInt("id");
  pUser->getHairInfo().useHairFree(id);
  DWORD color = params.getTableInt("color");
  pUser->getHairInfo().useColorFree(color);
  return true;
}

bool GMCommandRuler::npcdie(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  pUser->getQuestNpc().killNpc(params.getTableInt("id"), params.getTableInt("uniqueid"));

  return true;
}

bool GMCommandRuler::openseal(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (params.has("id") == false)
    return false;
  bool toDel = params.getTableInt("del") == 1;
  //pUser->getSeal().testOpenSeal();
  DWORD id = params.getTableInt("id");
  if (!toDel)
    pUser->getSeal().openSeal(id);
  else
    pUser->getSeal().closeSeal(id);
  return true;
}

bool GMCommandRuler::activemap(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  if (params.getTableInt("noactivecur") == 0)
  {
    if (pUser->getScene() != nullptr)
      pUser->getFreyja().addFreyja(pUser->getScene()->getMapID());
    else
      XERR << "[GM指令-地图激活]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败, 玩家场景为空" << XEND;
  }

  xLuaData data = params;
  auto func = [&](const string& key, xLuaData& data)
  {
    pUser->getFreyja().addFreyja(data.getInt());
  };
  data.getMutableData("mapid").foreach(func);

  XLOG << "[GM指令-地图激活]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行成功" << XEND;
  return true;
}

bool GMCommandRuler::setSearchRange(xSceneEntryDynamic * entry, const xLuaData &params)
{
  if (!entry) return false;

  Scene* pScene = entry->getScene();
  if (pScene == nullptr)
    return false;

  if (!pScene->isDScene())
    return false;

  DWORD range = params.getTableInt("range");
  if (!range) return false;

  xSceneEntrySet set;
  pScene->getAllEntryList(SCENE_ENTRY_NPC, set);
  Cmd::NpcSearchRangeCmd message;
  for (auto &it : set)
  {
    SceneNpc *npc = (SceneNpc *)it;
    message.set_id(npc->id);
    message.set_range(range);
    npc->m_ai.m_dwSearchRange = range;
    PROTOBUF(message, send, len);
    npc->sendCmdToNine(send, len);
  }

  return true;
}

bool GMCommandRuler::passtower(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  if (pUser->getTeamID() == 0)
    return false;

  DWORD dwLayer = params.getTableInt("layer");
  for (DWORD d = 0; d < dwLayer; ++d)
    pUser->getTower().passLayer(d);
  return true;
}

bool GMCommandRuler::resettower(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  pUser->getVar().setVarValue(EVARTYPE_TOWER, 0);
  pUser->getTower().isRewarded(0);  // 调用isReward中的resetData函数
  pUser->updateTeamTower();
  return true;
}

bool GMCommandRuler::barrage(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  pUser->m_dwBarrageTime = now() + 60;

  return true;
}

bool GMCommandRuler::shownpc(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (params.has("id") == false)
    return false;
  DWORD id = params.getTableInt("id");
  pUser->addShowNpc(id, params.getTableInt("share"));
  return true;
}

bool GMCommandRuler::hidenpc(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (params.has("id") == false)
    return false;
  DWORD id = params.getTableInt("id");
  //bool atonce = params.getTableInt("atonce") == 1;
  pUser->delShowNpc(id);
  return true;
}

bool GMCommandRuler::addmotion(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  if (params.has("id") == false || params.has("style") == false)
    return false;
  DWORD id = params.getTableInt("id");
  DWORD type = params.getTableInt("style");
  if (type == 1)
  {
    if (id == 0)
    {
      auto func = [&](const xEntry* pEntry)
      {
        const SActionAnimBase* pBase = static_cast<const SActionAnimBase*>(pEntry);
        if (pBase == nullptr)
          return;
        pUser->getUserSceneData().addAction(static_cast<DWORD>(pBase->id));
      };
      Table<SActionAnimBase>* pActionAnimList = TableManager::getMe().getActionAnimCFGListNoConst();
      if (pActionAnimList == nullptr)
        return false;
      pActionAnimList->foreachNoConst(func);
    }
    else
    {
      pUser->getUserSceneData().addAction(id);
    }
  }
  else if (type == 2)
  {
    if (id == 0)
    {
      auto func = [&](const xEntry* pEntry)
      {
        const SExpression* pBase = static_cast<const SExpression*>(pEntry);
        if (pBase == nullptr)
          return;
        pUser->getUserSceneData().addExpression(static_cast<DWORD>(pBase->id));
      };
      Table<SExpression>* pExpressionList = TableManager::getMe().getExpressionCFGListNoConst();
      if (pExpressionList == nullptr)
        return false;
      pExpressionList->foreachNoConst(func);
    }
    else
    {
      pUser->getUserSceneData().addExpression(id);
    }
  }
  return true;
}

bool GMCommandRuler::handinhand(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);
  const GTeam& rTeam = pUser->getTeam();
  if (rTeam.getTeamID() != 0)
  {
    if (rTeam.getTeamMemberList().size() != 2)
      return false;

    for (auto &m : rTeam.getTeamMemberList())
    {
      if (pUser->id == m.second.charid())
        continue;

      Cmd::JoinHandsUserCmd message;
      message.set_masterid(pUser->id);
      message.set_time(now() + 300);
      char sign[1024];
      bzero(sign, sizeof(sign));
      std::stringstream ss;
      ss << m.second.charid() << "_" << message.masterid() << "_" << message.time() << "_" << "#$%^&";
      upyun_md5(ss.str().c_str(), ss.str().size(), sign);
      message.set_sign(sign);
      PROTOBUF(message, send, len);
      thisServer->forwardCmdToSceneUser(m.second.charid(), send, len);
      break;
    }
  }
  return true;
}

bool GMCommandRuler::image(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  if (!params.has("raid"))
  {
    return false;
  }

  DWORD range = 10;
  if (params.has("range"))
  {
    range = params.getTableInt("range");
  }
  
  entry->getScene()->m_oImages.add(params.getTableInt("raid"), pUser, pUser->getPos(), range);

  return true;
}

bool GMCommandRuler::misc(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD type = params.getTableInt("type");
  if (type <= EALBUMTYPE_MIN || type >= EALBUMTYPE_MAX || EAlbumType_IsValid(type) == false)
    return false;
  UploadSceneryPhotoUserCmd message;
  message.set_sceneryid(1);
  message.set_type(static_cast<EAlbumType>(type));
  PROTOBUF(message, send, len);
  pUser->doUserCmd((const Cmd::UserCmd*)send, len);

  return true;
}

bool GMCommandRuler::setdaily(xSceneEntryDynamic* entry, const xLuaData &params)
{
  SetGlobalDaily cmd;
  cmd.set_value(params.getTableInt("value"));

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::initdaily(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  return pUser->getQuest().initDaily();
}

bool GMCommandRuler::refreshquest(xSceneEntryDynamic* entry, const xLuaData &params)
{
  RefreshQuest cmd;
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::dropItem(xSceneEntryDynamic* entry, const xLuaData &params)
{
  // get param
  if (entry == nullptr)
    return false;
  DWORD id = 0;
  DWORD count = params.getTableInt("count");
  if (!params.has("id"))
  {
    XERR << "[GM]" << entry->name << "dropItem 语法错误, 找不到id" << XEND;
    return false;
  }
  id = params.getTableInt("id");
  count = count == 0 ? 1 : count;

  if (ItemConfig::getMe().getItemCFG(id) == nullptr)
  {
    XERR << "[GM]" << entry->name << "dropItem 掉落物品失败, 策划表找不到物品" << id << XEND;
    return false;
  }

  // create iteminfo
  ItemInfo stInfo;
  stInfo.set_id(id);
  stInfo.set_count(1);
  stInfo.set_source(ESOURCE_GM);
  Scene* scene = entry->getScene();
  if (!scene)
  {
    XERR << "[GM]" << entry->name << "dropItem scene为空" << XEND;
    return false;
  }
  // create scene item
  Cmd::AddMapItem cmd;
  DWORD extraTime = MiscConfig::getMe().getSceneItemCFG().dwDropInterval;
  float fRange = MiscConfig::getMe().getSceneItemCFG().getRange(count);
  if (params.has("range"))
    fRange = params.getTableFloat("range");

  for (DWORD i = 0; i < count; ++i)
  {
    xPos dest;
    if (scene->getRandPos(entry->getPos(), fRange, dest) == false)
    {
      XERR << "[GM]" << entry->name << "dropItem 获取随机位置失败" << XEND;
      return false;
    }

    if (params.has("GM_ESource"))
    {
      DWORD source = params.getTableInt("GM_ESource");
      ESource esource = static_cast<ESource>(source);
      stInfo.set_source(esource);
    }
    else
    {
      stInfo.set_source(ESOURCE_PICKUP);
    }

    SceneItem* pItem = SceneItemManager::getMe().createSceneItem(scene, stInfo, dest);
    if (pItem == nullptr)
    {
        XERR << "[GM]" << entry->name << "dropItem SceneItem 创建失败" << XEND;
        return false;
    }
    if (entry->getEntryType() == SCENE_ENTRY_USER)
      pItem->addOwner(entry->id);
    pItem->fillMapItemData(cmd.add_items(), extraTime);

    if (cmd.items_size() > 100)
    {
      PROTOBUF(cmd, send, len);
      scene->sendCmdToNine(entry->getPos(), send, len);
      cmd.Clear();
    }
  }
  // inform client
  if (cmd.items_size() <= 100)
  {
    PROTOBUF(cmd, send, len);
    scene->sendCmdToNine(entry->getPos(), send, len);
  }

  return true;
}

bool GMCommandRuler::manual(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD type = params.getTableInt("type");
  if (type == 1)
  {
    DWORD dwPoint = params.getTableInt("point");
    pUser->getManual().addSkillPoint(dwPoint);
  }
  else if (type == 2)
  {
    pUser->getManual().addManualItem(static_cast<EManualType>(params.getTableInt("mtype")), params.getTableInt("id"), static_cast<EManualStatus>(params.getTableInt("status")));
  }
  else if (type == 3)
  {
    pUser->getManual().delManualItem(static_cast<EManualType>(params.getTableInt("mtype")), params.getTableInt("id"));
  }
  else
  {
    const string& method = params.getTableString("method");
    if (method == "active")
    {
      DWORD dwID = params.getTableInt("GM_Use_Source_Item");
      SManualItem* pItem = pUser->getManual().getManualItem(EMANUALTYPE_COLLECTION);
      if (pItem == nullptr)
      {
        XERR << "[GM指令-冒险手册]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "激活" << EMANUALTYPE_COLLECTION <<  dwID << "失败,未找到页签" << XEND;
        return false;
      }
      SManualSubItem* pSubItem = pItem->getSubItem(dwID);
      if (pSubItem != nullptr && pSubItem->eStatus >= EMANUALSTATUS_UNLOCK_CLIENT)
      {
        const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(dwID);
        if (pCFG == nullptr)
        {
          XERR << "[GM指令-冒险手册]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << EMANUALTYPE_COLLECTION << dwID << "已激活,给奖励失败,未在 Table_Item.txt 表中找到" << XEND;
          return false;
        }
        TVecItemInfo vecReward;
        for (auto &s : pCFG->setAdvRewardIDs)
        {
          TVecItemInfo vecItem;
          if (RewardManager::roll(s, pUser, vecItem, ESOURCE_MANUAL) == false)
          {
            XERR << "[GM指令-冒险手册]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
              << EMANUALTYPE_COLLECTION << dwID << "已激活,给奖励失败,随机" << s << "失败" << XEND;
            return false;
          }
          combinItemInfo(vecReward, vecItem);
        }
        if (pUser->getPackage().addItem(vecReward, EPACKMETHOD_CHECK_WITHPILE) == false)
        {
          MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_PACK_FULL);
          return false;
        }
        XLOG << "[GM指令-冒险手册]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << EMANUALTYPE_COLLECTION << dwID << "已激活,直接发送奖励" << vecReward << XEND;
        return true;
      }
      MsgManager::sendMsg(pUser->id, 572);
      XLOG << "[GM指令-冒险手册]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "激活" << EMANUALTYPE_COLLECTION << "成功,由外部逻辑激活" << XEND;
    }
    else if (method == "card")
    {
      SManualItem* pItem = pUser->getManual().getManualItem(EMANUALTYPE_CARD);
      if (pItem == nullptr)
      {
        XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败, 未找到冒险卡片选项" << XEND;
        return false;
      }

      xLuaData tmp = params;

      TSetDWORD setTypes;
      auto typef = [&](const string& key, xLuaData& data)
      {
        setTypes.insert(data.getInt());
      };
      tmp.getMutableData("cardtype").foreach(typef);

      bool bCorrect = true;
      set<EQualityType> setQualitys;
      auto qualityf = [&](const string& key, xLuaData& data)
      {
        DWORD dwQuality = data.getInt();
        if (dwQuality <= EQUALITYTYPE_MIN || dwQuality >= EQUALITYTYPE_MAX || EQualityType_IsValid(dwQuality) == false)
        {
          bCorrect = false;
          XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败, quality :" << dwQuality << "不合法" << XEND;
          return;
        }
        setQualitys.insert(static_cast<EQualityType>(dwQuality));
      };
      tmp.getMutableData("quality").foreach(qualityf);
      if (!bCorrect)
        return false;

      stringstream sstr;
      if (setQualitys.empty() == false)
      {
        sstr << " quality : ";
        for (auto &s : setQualitys)
          sstr << s << " ";
      }

      map<DWORD, SItemCFG> mapCardCFG;
      for (auto &s : setQualitys)
      {
        const SQualityCard* pCFG = ItemConfig::getMe().getCardQualityCFG(s);
        if (pCFG == nullptr)
        {
          XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败, quality :" << s << "未包含任何卡片" << XEND;
          return false;
        }
        for (auto &v : pCFG->vecCardCFG)
          mapCardCFG[v.dwTypeID] = v;
      }

      DWORD dwExtraQuality = params.getTableInt("extraquality");

      TSetDWORD setIDs;
      TSetDWORD setAddIDs;
      for (auto &m : mapCardCFG)
      {
        if (setTypes.empty() == false && setTypes.find(m.second.dwCardType) == setTypes.end())
          continue;
        const SItemCFG* pCFG = ItemConfig::getMe().getItemCFG(m.first);
        if (pCFG == nullptr || ItemConfig::getMe().canEnterManual(pCFG) == false)
          continue;
        const SManualSubItem* pSubItem = pItem->getSubItem(m.first);
        if (pSubItem == nullptr || pSubItem->eStatus < EMANUALSTATUS_UNLOCK_CLIENT)
          setIDs.insert(m.first);
        else if(pCFG->eQualityType == static_cast<EQualityType>(dwExtraQuality))
          setAddIDs.insert(m.first);
      }

      DWORD dwGiveItem = 0;
      dwGiveItem = tmp.getTableInt("itemnum");

      if (setIDs.empty() == true)
      {
        if(dwGiveItem == 0)
        {
          ItemData oData;
          oData.mutable_base()->set_id(ITEM_APOLOGY);
          oData.mutable_base()->set_count(params.getTableInt("apology"));
          if (oData.base().count() <= 0)
          {
            XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败" << sstr.str() << "未包含能激活的卡片,转换为补偿币,补偿币数量为0" << XEND;
            return false;
          }
          if (pUser->getPackage().addItem(oData, EPACKMETHOD_NOCHECK) == false)
          {
            XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败" << sstr.str() << "未包含能激活的卡片,转换为补偿币,添加失败" << XEND;
            return false;
          }
          XLOG << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
            << "执行成功" << sstr.str() << "未包含能激活的卡片,转换为补偿币" << oData.ShortDebugString() << XEND;
        }
        else
        {
          DWORD* pID = randomStlContainer(setAddIDs);
          if (pID == nullptr)
          {
            XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败" << sstr.str() << "随机失败" << XEND;
            return false;
          }
          ItemData oData;
          oData.mutable_base()->set_id(*pID);
          oData.mutable_base()->set_count(dwGiveItem);
          if (pUser->getPackage().addItem(oData, EPACKMETHOD_NOCHECK) == false)
          {
            XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败" << sstr.str() << "添加卡片失败" << "itemid: " << *pID << XEND;
            return false;
          }
        }
      }
      else
      {
        DWORD* pID = randomStlContainer(setIDs);
        if (pID == nullptr)
        {
          XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败" << sstr.str() << "随机失败" << XEND;
          return false;
        }
        if(dwGiveItem != 0)
        {
          ItemData oData;
          oData.mutable_base()->set_id(*pID);
          oData.mutable_base()->set_count(dwGiveItem);
          if (pUser->getPackage().addItem(oData, EPACKMETHOD_NOCHECK) == false)
          {
            XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败" << sstr.str() << "添加卡片失败" << "itemid: " << *pID << XEND;
            return false;
          }
        }
        if (pUser->getManual().addManualItem(EMANUALTYPE_CARD, *pID, EMANUALSTATUS_UNLOCK_CLIENT) == false)
        {
          XERR << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败" << sstr.str() << "激活" << *pID << "失败" << XEND;
          return false;
        }
        XLOG << "[GM指令-manual]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行成功, 激活" << sstr.str() << "id :" << *pID << "卡片" << XEND;
      }
    }
  }

  return true;
}

bool GMCommandRuler::portrait(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD type = params.getTableInt("type");
  if (type == 1)
  {
    auto func = [&](const SPortraitCFG& r)
    {
      if (r.eType != EPORTRAITTYPE_USERPORTRAIT && r.eType != EPORTRAITTYPE_PETPORTRAIT)
        return;
      if (pUser->getPortrait().checkAddItems(r.dwID) == true)
        pUser->getPortrait().addNewItems(r.dwID);
    };
    PortraitConfig::getMe().foreach(func);
  }

  XLOG << "[GM指令-头像]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行指令 type :" << type << XEND;
  return true;
}

bool GMCommandRuler::delchar(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  GMDeleteCharUserCmd cmd;
  cmd.set_accid(pUser->accid);
  cmd.set_zoneid(thisServer->getZoneID());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToGate(send, len);
  return true;
}

bool GMCommandRuler::tradeBriefSearch(xSceneEntryDynamic* entry, const xLuaData &params)
{
  //GET_USER(entry);
  //// get param

  //DWORD itemType = 70;
  //DWORD rankType = RANKTYPE_ITEM_ID_INC;
  //DWORD pageIndex = 0;
  //DWORD pageCount = 3;

  //if (params.has("itemtype"))
  //{
  //  itemType = params.getTableInt("itemtype");
  //}

  //if (params.has("ranktype"))
  //{
  //  rankType = params.getTableInt("ranktype");
  //}

  //if (params.has("pageindex"))
  //{
  //  pageIndex = params.getTableInt("pageindex");
  //}
  //if (params.has("pagecount"))
  //{
  //  pageCount = params.getTableInt("pagecount");
  //}

  //Scene* scene = pUser->getScene();
  //if (!scene)
  //{
  //  XERR("[GM],%s,dropItem scene为空");
  //  return false;
  //}

  //Cmd::BriefPendingListRecordTradeCmd cmd;
  //SearchCond* pSearchCond = cmd.mutable_search_cond();
  //pSearchCond->set_item_type(itemType);
  //pSearchCond->set_rank_type(static_cast<Cmd::RankType>(rankType));
  //pSearchCond->set_page_index(pageIndex);
  //pSearchCond->set_page_count(pageCount);
  //cmd.set_charid(pUser->id);
  //PROTOBUF(cmd, send, len);
  //forwardRecord((const Cmd::UserCmd*)(send), len);
  return true;
}

bool GMCommandRuler::tradeDetailSearch(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  // get param
  DWORD id = 52101;   //12024 蓝色药水 52101

  DWORD rankType = RANKTYPE_ITEM_PRICE_INC;
  DWORD pageIndex = 0;
  DWORD pageCount = 3;

  if (params.has("id"))
  {
    id = params.getTableInt("id");
  }

  if (params.has("ranktype"))
  {
    rankType = params.getTableInt("ranktype");
  }

  if (params.has("pageindex"))
  {
    pageIndex = params.getTableInt("pageindex");
  }
  if (params.has("pagecount"))
  {
    pageCount = params.getTableInt("pagecount");
  }

  Scene* scene = pUser->getScene();
  if (!scene)
  {
    XERR << "[GM]dropItem scene为空" << XEND;
    return false;
  }

  Cmd::DetailPendingListRecordTradeCmd cmd;
  SearchCond* pSearchCond = cmd.mutable_search_cond();
  pSearchCond->set_item_id(id);
  pSearchCond->set_rank_type(static_cast<Cmd::RankType>(rankType));
  pSearchCond->set_page_index(pageIndex);
  pSearchCond->set_page_count(pageCount);
  cmd.set_charid(pUser->id);
  PROTOBUF(cmd, send, len);
  forwardRecord((const Cmd::UserCmd*)(send), len);
  return true;
}

bool GMCommandRuler::tradeMyPending(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  // get param
  DWORD pageIndex = 0;
  DWORD pageCount = 10;

  if (params.has("pageindex"))
  {
    pageIndex = params.getTableInt("pageindex");
  }
  if (params.has("pagecount"))
  {
    pageCount = params.getTableInt("pagecount");
  }

  Scene* scene = pUser->getScene();
  if (!scene)
  {
    XERR << "[GM]scene为空" << XEND;
    return false;
  }

  Cmd::MyPendingListRecordTradeCmd cmd;
  SearchCond* pSearchCond = cmd.mutable_search_cond();
  pSearchCond->set_page_index(pageIndex);
  pSearchCond->set_page_count(pageCount);
  cmd.set_charid(pUser->id);
  PROTOBUF(cmd, send, len);
  forwardRecord((const Cmd::UserCmd*)(send), len);
  return true;
}

bool GMCommandRuler::tradeMyLog(xSceneEntryDynamic* entry, const xLuaData &params)
{
  //GET_USER(entry);
  //// get param

  //DWORD pageIndex = 0;
  //DWORD pageCount = 100;
  //
  //if (params.has("pageindex"))
  //{
  //  pageIndex = params.getTableInt("pageindex");
  //}
  //if (params.has("pagecount"))
  //{
  //  pageCount = params.getTableInt("pagecount");
  //}

  //Scene* scene = pUser->getScene();
  //if (!scene)
  //{
  //  XERR << "[GM]" << pUser->name << "scene为空" << XEND;
  //  return false;
  //}

  //Cmd::MyTradeLogRecordTradeCmd cmd;
  //SearchCond* pSearchCond = cmd.mutable_search_cond();
  //pSearchCond->set_page_index(pageIndex);
  //pSearchCond->set_page_count(pageCount);
  //cmd.set_charid(pUser->id);
  //PROTOBUF(cmd, send, len);
  //forwardRecord((const Cmd::UserCmd*)(send), len);
  return true;
}

bool GMCommandRuler::tradePrice(xSceneEntryDynamic* entry, const xLuaData &params)
{
  // GET_USER(entry);
  // // get param
  // DWORD id = 52101;   //12024 蓝色药水 52101

  // if (params.has("id"))
  // {
  //   id = params.getTableInt("id");
  // }

  // if (ItemConfig::getMe().getItemCFG(id) == nullptr)
  // {
  //   XERR << "[GM]" << pUser->name << "tradeSellItem, 策划表找不到物品" << id << XEND;
  //   return false;
  // }

  // Scene* scene = pUser->getScene();
  // if (!scene)
  // {
  //   XERR <<"[GM]" << pUser->name << "scene为空" << XEND;
  //   return false;
  // }

  // Cmd::ReqServerPriceRecordTradeCmd cmd;
  // cmd.set_itemid(id);
  // cmd.set_charid(pUser->id);
  // PROTOBUF(cmd, send, len);
  // forwardRecord((const Cmd::UserCmd*)(send), len);
  return true;
}

bool GMCommandRuler::tradeSell(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  // get param
  DWORD id = 52101;   //12024 蓝色药水 52101
  DWORD count = 100;
  DWORD price = 1000;
  DWORD refineLv = 0;

  if (params.has("id"))
  {
    id = params.getTableInt("id");
  }
  if (params.has("count"))
  {
    count = params.getTableInt("count");
  }

  if (params.has("price"))
  {
    price = params.getTableInt("price");
  } 
  
  //if (params.has("guid"))
  //{
  //  guid = params.getTableString("guid");
  //}
  
  if (ItemConfig::getMe().getItemCFG(id) == nullptr)
  {
    XERR << "[GM]" << pUser->name << "tradeSellItem, 策划表找不到物品" << id << XEND;
    return false;
  }

  Scene* scene = pUser->getScene();
  if (!scene)
  {
    XERR << "[GM]" << pUser->name << "scene为空" << XEND;
    return false;
  }

  std::string guid  = pUser->getPackage().getPackage(EPACKTYPE_MAIN)->getGUIDByType(id);

  ItemBase* pItem = pUser->getPackage().getPackage(EPACKTYPE_MAIN)->getItem(guid);
  if (pItem == nullptr)
  {
    XERR << "[交易-出售][背包里找不到该物品] user =" << pUser->id << "itemid =" << id << "count =" << count << XEND;
    return false;
  }
  // check equip
  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pItem);
  if (pEquip != nullptr)
  {
    refineLv = pEquip->getRefineLv();
  }

 // ChatMessage::sendSysMsg(pUser, EDBGMSGTYPE_TEST, "[GM] 找到guid：%s", guid.c_str());

  {
    ChatRetCmd message;
    message.set_id(1111);
    message.set_name("交易");
    message.set_channel(ECHAT_CHANNEL_FRIEND);
    std::stringstream ss;
    ss << "[GM] 为物品itemid：" << id << " 找到guid：" << guid;
    message.set_str(ss.str());
    PROTOBUF(message, send, len);
    pUser->sendCmdToMe(send, len);
  }
  
  Cmd::SellItemRecordTradeCmd cmd;
  TradeItemBaseInfo* pItemInfo = cmd.mutable_item_info();
  pItemInfo->set_count(count);
  pItemInfo->set_price(price);
  pItemInfo->set_itemid(id);
  pItemInfo->set_guid(guid);
  pItemInfo->set_refine_lv(refineLv);
  cmd.set_charid(pUser->id);
  PROTOBUF(cmd, send, len);
  forwardRecord((const Cmd::UserCmd*)(send), len);
  return true;
}

bool GMCommandRuler::tradeBuy(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  // get param
  DWORD id = 52101;   //12024 蓝色药水 52101
  DWORD count = 100;
  DWORD price = 100;
  QWORD orderId = 0;
  if (params.has("id"))
  {
    id = params.getTableInt("id");
  }
  if (params.has("count"))
  {
    count = params.getTableInt("count");
  }

  if (params.has("price"))
  {
    price = params.getTableInt("price");
  }

  if (params.has("orderid"))
  {
    orderId = (QWORD) params.getTableFloat("orderid");
  }
  
  if (ItemConfig::getMe().getItemCFG(id) == nullptr)
  {
    XERR << "[GM]" << pUser->name << "tradeSellItem, 策划表找不到物品" << id << XEND;
    return false;
  }

  Scene* scene = pUser->getScene();
  if (!scene)
  {
    XERR << "[GM]" << pUser->name << "scene为空" << XEND;
    return false;
  }

  Cmd::BuyItemRecordTradeCmd cmd;
  TradeItemBaseInfo* pItemInfo = cmd.mutable_item_info();

  if (orderId > 0)
  {
    pItemInfo->set_order_id(orderId);
    pItemInfo->set_count(1);
  }
  else
  {
    pItemInfo->set_count(count);
    pItemInfo->set_price(price);
  }
  pItemInfo->set_itemid(id);
  cmd.set_charid(pUser->id);
  PROTOBUF(cmd, send, len);
  forwardRecord((const Cmd::UserCmd*)(send), len);
  return true;
}

bool GMCommandRuler::tradeCancel(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  // get param
  QWORD orderid = 23;   //挂单数据库id
  if (params.has("orderid"))
  {
    orderid = params.getTableInt("orderid");
  }

  Scene* scene = pUser->getScene();
  if (!scene)
  {
    XERR << "[GM]" << pUser->name << "scene为空" << XEND;
    return false;
  }

  Cmd::CancelItemRecordTrade cmd;
  cmd.set_charid(pUser->id);
  cmd.set_order_id(orderid);
  PROTOBUF(cmd, send, len);
  forwardRecord((const Cmd::UserCmd*)(send), len);
  return true;
}

bool GMCommandRuler::forwardRecord(const Cmd::UserCmd* cmd, unsigned short len)
{
  if (!cmd)
    return false;
  if (len > MAX_USER_DATA_SIZE)
  {
    XINF << "[UserCmd]" << cmd->cmd << cmd->param << "big size cmd:" << len << XEND;
  }  
  DWORD totalLen = len + sizeof(ForwardRecrodCmdGatewayCmd);
  BUFFER_CMD_SIZE(rev, ForwardRecrodCmdGatewayCmd, totalLen);
  
  rev->len = len;
  bcopy(cmd, rev->data, (DWORD)len);
  return thisServer->sendCmdToRecord(rev, totalLen);
}

bool GMCommandRuler::refreshtower(xSceneEntryDynamic* entry, const xLuaData &params)
{
  RefreshTower cmd;
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::compensateuser(xSceneEntryDynamic* entry, const xLuaData &params)
{
  enum ECompensateType
  {
    ECOMPENSATE_SKILL = 1,
  };
  GET_USER(entry);

  if (params.has("etype") == false || params.has("id") == false)
    return false;
  DWORD id = params.getTableInt("id");
  ECompensateType eType = static_cast<ECompensateType> (params.getTableInt("etype"));
  switch(eType)
  {
    case ECOMPENSATE_SKILL:
      {
        if (pUser->getFighter() == nullptr)
          return false;
        pUser->getFighter()->getSkill().forceEnableSkill(id);
        XLOG << "[GM], 对玩家:" << pUser->name << pUser->id << "强制发放技能:" << id << XEND;
      }
      break;
    default:
      break;
  }
  return true;
}

bool GMCommandRuler::speffect(xSceneEntryDynamic* entry, const xLuaData &params)
{
  if (entry == nullptr)
    return false;
  // get param
  if (!params.has("speffectid"))
  {
    return false;
  }

  DWORD effectId = params.getTableInt("speffectid");

  if (!params.has("id1"))
  {
    return false;
  }
  QWORD slaveid = 0;
  if (params.has("id1"))
  {
    slaveid = params.getTableQWORD("id1");
  }

  if (params.getTableInt("target") == 1)
  {
    xSceneEntryDynamic* pTarget = xSceneEntryDynamic::getEntryByID(slaveid);
    if (pTarget != nullptr)
    {
      TSetQWORD targetSet;
      targetSet.insert(entry->id);
      return pTarget->getSpEffect().add(effectId, targetSet);
    }
  }
  else
  {
    TSetQWORD targetSet;
    targetSet.insert(slaveid);
    return entry->getSpEffect().add(effectId, targetSet);
  }
  return false;
}

bool GMCommandRuler::changearrow(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (!params.has("id"))
    return false;
  DWORD typeID = params.getTableInt("id");
  pUser->getPackage().changeArrow(typeID);
  return true;
}

bool GMCommandRuler::guildlevelup(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD dwNum = params.getTableInt("num");
  DWORD dwGuildID = params.getTableInt("guildid");
  if (dwNum == 0)
    return false;

  GuildLevelUpSocialCmd cmd;
  cmd.set_charid(pUser->id);
  cmd.set_addlevel(dwNum);
  cmd.set_guildid(dwGuildID);
  cmd.set_guildname(params.getTableString("name"));
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::questitem(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD dwQuestID = params.getTableInt("questid");
  DWORD dwItemID = params.getTableInt("id");
  DWORD dwNum = params.getTableInt("num") == 0 ? 1 : params.getTableInt("num");

  if (pUser->getQuest().getQuest(dwQuestID) != nullptr)
  {
    XERR << "[GM指令-任务道具]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用失败,任务 :" << dwQuestID << "已存在" << XEND;
    return false;
  }

  ItemInfo oItem;
  oItem.set_id(dwItemID);
  oItem.set_count(dwNum);
  oItem.set_source(ESOURCE_QUEST);
  /*BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[GM指令-任务道具]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用失败,获取" << EPACKTYPE_MAIN << "失败" << XEND;
    return false;
  }
  if (pMainPack->checkAddItem(oItem) == false)
  {
    XERR << "[GM指令-任务道具]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用失败,无法添加" << oItem.ShortDebugString() << "到" << EPACKTYPE_MAIN << XEND;
    return false;
  }*/

  if (pUser->getQuest().acceptQuest(dwQuestID, true) == false)
  {
    XERR << "[GM指令-任务道具]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用失败,任务 :" << dwQuestID << "接取失败" << XEND;
    return false;
  }
  //pMainPack->addItemFull(oItem);
  pUser->getPackage().addItem(oItem, EPACKMETHOD_NOCHECK);
  XLOG << "[GM指令-任务道具]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "使用成功,成功接取任务 :" << dwQuestID << "获得" << oItem.ShortDebugString() << XEND;
  return true;
}

bool GMCommandRuler::movetrack(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  if (!entry->getScene())
    return false;

  if (params.has("open"))
  {
    bool open = params.getTableInt("open") != 0;
    if (open)
    {
      NpcDefine def;
      def.setID(1361); // 蜂蜜
      def.setPos(pUser->getPos());
      SceneNpc *npc = SceneNpcManager::getMe().createNpc(def, pUser->getScene());
      if (npc)
      {
        npc->setClearTime(now() + 2);
      }

      pUser->bOpenMoveTrack = true;
    }
    else
    {
      pUser->bOpenMoveTrack = false;
    }
  }

  // 展示附近npc 位置
  if (params.has("npc"))
  {
    DWORD npcid = params.getTableInt("npc");
    xSceneEntrySet npcset;
    pUser->getScene()->getEntryList(pUser->getPos(), 10, npcset);
    for (auto &s : npcset)
    {
      SceneNpc* pNpc = dynamic_cast<SceneNpc*> (s);
      if (!pNpc || pNpc->getNpcID() != npcid)
        continue;

      NpcDefine def;
      def.setID(1362); // 毒液
      def.setPos(pNpc->getPos());
      SceneNpc *npc = SceneNpcManager::getMe().createNpc(def, pUser->getScene());
      if (npc)
      {
        npc->setClearTime(now() + 2);
      }
    }
  }
  return true;
}

bool GMCommandRuler::setzone(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD id = params.getTableInt("id");

  DWORD zoneid = thisServer->getRegionID() * 10000 + id;

  pUser->getUserSceneData().setZoneID(zoneid);

  return true;
}

bool GMCommandRuler::itemimage(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD npcId = 0;
  if (params.has("npcid"))
  {
    npcId = params.getTableInt("npcid");
  }
  if (0 == npcId)
  {
    XLOG << "[GM-道具镜像]:失败，错误的配置 npcid 为0" << pUser->name << pUser->id << "地图" << pUser->getMapID() << XEND;
    return false;
  }

  QWORD qwTeamID = pUser->getTeamID();
  if (qwTeamID == 0)
  {
    XLOG << "[GM-道具镜像]:失败，没有组队" << pUser->name << pUser->id << "地图" << pUser->getMapID() << XEND;
    return false;
  }

  DWORD raid = MapConfig::getMe().getItemImageRaid(pUser->getMapID());
  if (raid == 0)
  {
    MsgManager::sendMsg(pUser->id, 71);  
    XLOG << "[GM-道具镜像]:失败，找不到对应的raid" << pUser->name << pUser->id << "地图" << pUser->getMapID() << XEND;
    return false;
  }

  const SDressStageCFG rCFG = MiscConfig::getMe().getDressStageCFG();
  if(DressUpStageMgr::getMe().checkStageDistance(pUser->getPos(), MiscConfig::getMe().getItemImageCFG().dwRange + rCFG.m_dwStageRange, pUser->getScene()))
    return false;

  std::string key = RedisManager::getMe().getKeyByParam(thisServer->getRegionID(), EREDISKEYTYPE_ITEMIMAGE, qwTeamID, thisServer->getZoneID(), "user");
  QWORD qwUser = 0;
  RedisManager::getMe().getData(key, qwUser);

  if (qwUser)
  {
    MsgManager::sendMsg(pUser->id, 70);
    XLOG << "[GM-道具镜像]:失败，已经有队友开启道具镜像" << pUser->name << pUser->id << "地图" << pUser->getMapID() << XEND;
    return false;
  }
  
  if (!entry->getScene()->m_oImages.add(raid, pUser, pUser->getPos(), MiscConfig::getMe().getItemImageCFG().dwRange, 0, npcId))
  {
    XLOG << "[GM-道具镜像]:失败，道具镜像无法添加到地图" << pUser->name << pUser->id << "地图" << pUser->getMapID() << XEND;
    return false;
  }

  RedisManager::getMe().setData(key, pUser->id, 30 * 60);
  pUser->sendItemImage(true);
  XLOG << "[GM-道具镜像]:创建成功" << pUser->name << pUser->id << "地图" << pUser->getMapID()<<"raid"<<raid<< XEND;
  return true;
}

bool GMCommandRuler::itemmusic(xSceneEntryDynamic * entry, const xLuaData &params)
{
  GET_USER(entry);

  std::string soundUri = "Rocker";

  if (params.has("uri"))
  {
    soundUri = params.getTableString("uri");
  }

  DWORD npcid = 0;
  if (params.has("npcid"))
  {
    npcid = params.getTableInt("npcid");
  }

  DWORD range = 10;
  if (params.has("range"))
  {
    range = params.getTableInt("range");
  }

  DWORD expireTime = 30;
  if (params.has("expiretime"))
  {
    expireTime = params.getTableInt("expiretime");
  }
  DWORD itemId = params.getTableInt("id");

  return pUser->getItemMusic().startMusicItem(soundUri, expireTime, npcid, range, 2, itemId);
}

bool GMCommandRuler::addhandnpc(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  if (params.has("del"))
  {
    pUser->getHandNpc().delHandNpc();
    return true;
  }

  if (params.has("body"))
  {
    SHandNpcData sdata;
    sdata.dwBody = params.getTableInt("body");
    sdata.dwHead = params.getTableInt("head");
    sdata.dwHair = params.getTableInt("hair");
    sdata.dwHairColor = params.getTableInt("haircolor");
    sdata.dwSpEffectID = params.getTableInt("speffect");
    sdata.strName = params.getTableString("name");
    DWORD time = params.getTableInt("time");
    return pUser->getHandNpc().addHandNpc(sdata, time);
  }
  return pUser->getHandNpc().addHandNpc();
}

bool GMCommandRuler::toy_smile(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  // check param valid
  DWORD dwSelfExpression = params.getTableInt("selfexpression");
  DWORD dwExpression = params.getTableInt("expression");
  const SExpression* pSelfBase = TableManager::getMe().getExpressionCFG(dwSelfExpression);
  const SExpression* pBase = TableManager::getMe().getExpressionCFG(dwExpression);
  if (pSelfBase == nullptr || pBase == nullptr)
  {
    MsgManager::sendMsg(pUser->id, 10, MsgParams("表情不存在"));
    return false;
  }

  DWORD dwBuffID = params.getTableInt("buff");
  TPtrBufferState pState = BufferManager::getMe().getBuffById(dwBuffID);
  if (pState == nullptr)
  {
    MsgManager::sendMsg(pUser->id, 10, MsgParams("buff不存在"));
    return false;
  }

  if (params.getTableInt("GM_Target") != 1)
  {
    MsgManager::sendMsg(pUser->id, 10, MsgParams("没有目标"));
    return false;
  }

  SceneUser* pTarget = SceneUserManager::getMe().getUserByID(params.getTableQWORD("id1"));
  if (pTarget == nullptr)
  {
    MsgManager::sendMsg(pUser->id, 10, MsgParams("没有目标"));
    return false;
  }

  // 9屏内有MVP时, 提示不能使用
  if (pTarget->getScene())
  {
    xSceneEntrySet npcset;
    pTarget->getScene()->getEntryListInNine(SCENE_ENTRY_NPC, pTarget->getPos(), npcset);
    for (auto &s : npcset)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
      if (npc && npc->isBoss())
      {
        MsgManager::sendMsg(pUser->id, 933);
        return false;
      }
    }
  }

  // add buff
  if (pTarget->m_oBuff.add(dwBuffID) == false)
  {
    MsgManager::sendMsg(pUser->id, 10, MsgParams("添加buff失败"));
    return false;
  }

  // break function
  if (pTarget->m_oHands.has() == true)
    pTarget->m_oHands.breakup();
  //pTarget->getLine().breakUpAll();
  pTarget->m_oSkillProcessor.breakSkill(pUser->id);
  //
  pTarget->seatUp();

  // play expressiovn update
  UserActionNtf selfcmd;
  selfcmd.set_charid(pUser->id);
  selfcmd.set_type(EUSERACTIONTYPE_EXPRESSION);
  selfcmd.set_value(dwSelfExpression);
  PROTOBUF(selfcmd, selfsend, selflen);
  pUser->sendCmdToNine(selfsend, selflen);

  UserActionNtf cmd;
  cmd.set_charid(pTarget->id);
  cmd.set_type(EUSERACTIONTYPE_EXPRESSION);
  cmd.set_value(dwExpression);
  PROTOBUF(cmd, send, len);
  pTarget->sendCmdToNine(send, len);

  // update direction
  float x = pTarget->getPos().x - pUser->getPos().x;
  float z = pTarget->getPos().z - pUser->getPos().z;
  float angle = 0.0f;
  if (x > 0 && z > 0)
    angle = atan(abs(x) / abs(z)) * 180 / 3.14f + 180;
  else if (x < 0 && z > 0)
    angle = 180 - (atan(abs(x) / abs(z)) * 180 / 3.14f);
  else if (x < 0 && z < 0)
    angle = atan(abs(x) / abs(z)) * 180 / 3.14f;
  else if (x > 0 && z < 0)
    angle = 360 - atan(abs(x) / abs(z)) * 180 / 3.14f;
  pTarget->getUserSceneData().setDir(static_cast<DWORD>(angle * ONE_THOUSAND));

  XLOG << "[玩具-痒痒棒]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "对" << pTarget->accid << pTarget->id << pTarget->getProfession() << pTarget->name << "使用了痒痒棒, 对方转了" << angle << "° 并播放了" << dwExpression << "表情" << XEND;
  return true;
}

bool GMCommandRuler::activity(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  //check can use in the map
  Scene* pScene = pUser->getScene();
  if (!pScene)
    return false;
  
  DWORD id = 0;           //activity id
  if (params.has("id"))
  {
    id = params.getTableInt("id");
  }
  
  const SActivityCFG* pCfg = ActivityConfig::getMe().getActivityCFG(id);
  if (!pCfg)
  {
    XERR << "[活动-GM发起] 找不到活动配置 " << pUser->name << pUser->id << "activity id" << id << "mapid" << pScene->getMapID() << XEND;
    return false;
  }
  
  if (!pCfg->checkMap(pScene->getMapID()))
  {
    XERR << "[活动-GM发起] 错误的地图" << pUser->name << pUser->id << "activity id" << id << "mapid" << pScene->getMapID() << XEND;
    MsgManager::sendMsg(pUser->id, 71);
    return false;
  }
  
  ActivityTestAndSetSessionCmd cmd;
  cmd.set_id(id);
  cmd.set_mapid(pScene->getMapID());
  cmd.set_charid(pUser->id);  
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  XLOG << "[活动-GM发起] " << pUser->name << pUser->accid << pUser->id << "activity id" << id << "mapid" << pScene->getMapID() << XEND;
  return true;
}

bool GMCommandRuler::use_gift_code(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  
  std::string code = "XDROTAU4GS7Z";
  if (params.has("code"))
  {
    code = params.getTableString("code");
  }
  
  UseGiftCodeSocialCmd cmd;
  cmd.set_code(code);
  PROTOBUF(cmd, send, len);
  thisServer->forwardCmdToSessionUser(pUser->id, send, len);
  XLOG << "[GM-礼包码兑换] " << pUser->name << pUser->accid << pUser->id << code << XEND;
  return true;
}

bool GMCommandRuler::jumpzone(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  // check same zoneid
  DWORD dwNextZone = pUser->getTmpJumpZone();
  if (getClientZoneID(thisServer->getZoneID()) == dwNextZone)
  {
    XERR << "[GM-切线]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "请求切换到" << dwNextZone << "线失败,已经在该线中" << XEND;
    return false;
  }
  bool isFirst = true;
  std::string strItem = "";
  const SZoneMiscCFG& rCFG = MiscConfig::getMe().getZoneCFG();

  // check max lv
  if (CommonConfig::m_bJumpZoneCheckLevel)
  {
    DWORD dwMaxLv = ChatManager_SC::getMe().getZoneMaxBaseLv(getServerZoneID(thisServer->getZoneID(), dwNextZone));
    if (dwMaxLv < rCFG.dwJumpBaseLv && pUser->getUserSceneData().getRolelv() > dwMaxLv)
    {
      MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_LINE_MAXLEVEL);
      return false;
    }
  }

  // get zone status
  EZoneStatus eStatus = ChatManager_SC::getMe().getZoneStatus(getServerZoneID(thisServer->getZoneID(), dwNextZone));
  if (eStatus == EZONESTATUS_MIN)
  {
    XERR << "[GM-玩家切线]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "请求切换到" << dwNextZone << "线失败,线状态 :" << eStatus << XEND;
    return false;
  }

  // 添加mvp攻击切线保护buff
  pUser->m_oBuff.add(MiscConfig::getMe().getSystemCFG().dwZoneBossLimitBuff);

  // check return guild zone
  if (getClientZoneID(pUser->getGuild().zoneid()) != dwNextZone)
  {
    // check resource
    if (pUser->getVar().getVarValue(EVARTYPE_FIRST_EXCHANGEZONE) != 0)
    {
      BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
      if (pMainPack == nullptr)
      {
        XERR << "[GM-玩家切线]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "请求切换到" << dwNextZone << "线失败,未找到背包" << XEND;
        return false;
      }

      const TVecItemInfo& vecCost = rCFG.getJumpCost(eStatus);
      if (vecCost.empty() == true || pMainPack->checkItemCount(vecCost) == false)
      {
        XERR << "[GM-玩家切线]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "请求切换到" << dwNextZone << "线失败,材料不足" << XEND;
        return false;
      }
      pMainPack->reduceItem(vecCost, ESOURCE_JUMPZONE);

      stringstream ss;
      for (auto &v : vecCost)
      {
        ss << v.id() << "," << v.count() << ";";
      }
      strItem = ss.str();
      isFirst = false;
    }
    else
    {
      isFirst = true;
      pUser->getVar().setVarValue(EVARTYPE_FIRST_EXCHANGEZONE, 1);
    }

    /*// 添加mvp攻击切线保护buff
    pUser->m_oBuff.add(MiscConfig::getMe().getSystemCFG().dwZoneBossLimitBuff);*/
  }

  // get real zoneid
  DWORD dwZoneID = getServerZoneID(thisServer->getZoneID(), dwNextZone);
  pUser->getUserSceneData().setZoneID(dwZoneID, EJUMPZONE_USER);

  QWORD eid = xTime::getCurUSec();
  EVENT_TYPE eType = EventType_Jumpzone;
  PlatLogManager::getMe().eventLog(thisServer,
    pUser->getUserSceneData().getPlatformId(),
    pUser->getZoneID(),
    pUser->accid,
    pUser->id,
    eid,
    0,  /*charge */
    eType, 0, 1);

  PlatLogManager::getMe().JumpzoneLog(thisServer,
    pUser->accid,
    pUser->id,
    eType,
    eid,
    getClientZoneID(thisServer->getZoneID()),
    dwNextZone,
    isFirst,
    strItem);
  
  pUser->stopSendInactiveLog();

  XLOG << "[GM-玩家切线]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "切换到" << dwZoneID << "线" << XEND;
  return true;
}

bool GMCommandRuler::usedepositcard(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  
  DWORD id = 0;           
  if (!params.has("id"))
  {
    return false;
  }
  id = params.getTableInt("id");
  return pUser->getDeposit().useCard(id);
}

bool GMCommandRuler::clearBattletime(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  pUser->antiAddictRefresh(true);
  return true;
}

bool GMCommandRuler::setCredit(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  DWORD value = 0;
  value = params.getTableInt("value");
  pUser->getUserSceneData().setCredit(value);
  return true;
}

bool GMCommandRuler::playcharge(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  DWORD id = 0;
  id = params.getTableInt("id");
  Cmd::ChargePlayUserCmd cmd;
  cmd.add_chargeids(id);
  pUser->playChargeNpc(cmd);
  return true;
}

bool GMCommandRuler::addweaponpet(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("id") == false)
    return false;
  bool bDel = params.getTableInt("del") == 1;

  DWORD id = params.getTableInt("id");
  if (!bDel)
  {
    if (params.has("timetype") == false)
      return false;
    DWORD time = params.getTableInt("timetype");
    EEmployType eType = static_cast<EEmployType> (time);
    if (eType <= EEMPLOYTYPE_MIN || eType >= EEMPLOYTYPE_MAX)
      return false;
    return pUser->getWeaponPet().add(id, eType);
  }
  else
  {
    return pUser->getWeaponPet().del(id);
  }
  return true;
}

bool GMCommandRuler::lotterytest(xSceneEntryDynamic* entry, const xLuaData& params)
{
  #ifndef _DEBUG
    return true;
  #endif

  GET_USER(entry);

  DWORD type = params.getTableInt("type");
  DWORD ticket = params.getTableInt("ticket");
  DWORD num = params.getTableInt("num");
  DWORD year = params.getTableInt("year");
  DWORD month = params.getTableInt("month");

  std::map<DWORD, DWORD> mapItems; // <itemId, itemCount>
  for(DWORD i=1; i<=num; i++)
  {
    DWORD itemType = 0;
    if(ELotteryType_Magic == static_cast<ELotteryType>(type))
    {
      itemType = MiscConfig::getMe().getLotteryCFG().randomItemType(ticket);
      if(0 == itemType)
      {
        XERR << "[GM-扭蛋概率测试] 失败，itemType = 0" << XEND;
        return false;
      }
    }

    const SLotteryCFG* pLotteryCFG = ItemConfig::getMe().getLotteryCFG(type, year, month, itemType);
    if(!pLotteryCFG)
    {
      XERR << "[GM-扭蛋概率测试] 失败，get lottery config failed!" << XEND;
      return false;
    }

    std::vector<std::tuple<DWORD, DWORD, DWORD>> items;
    DWORD itemId = 0, itemCount = 1, itemRate = 0;

    if(ticket)
    {
      if(!pLotteryCFG->randomWithHideWeight(itemId, itemCount, pUser->getUserSceneData().getGender(), itemRate))
      {
        XERR << "[GM-扭蛋概率测试] 失败，ticket random failed!" << XEND;
        return false;
      }
    }
    else
    {
      if(!pLotteryCFG->random(itemId, itemCount, pUser->getUserSceneData().getGender(), itemRate))
      {
        XERR << "[GM-扭蛋概率测试] 失败，random failed!" << XEND;
        return false;
      }
    }

    items.push_back(std::make_tuple(itemId, itemCount, itemRate));
    mapItems[itemId]++;
  }

  XLOG << "[GM-扭蛋概率测试]开始，type:" << type << "ticket:" << ticket << "num:" << num << "year:" << year << "month:" << month << XEND;
  std::stringstream stream;
  stream.str("");
  for(auto& m : mapItems)
    stream << "[GM-扭蛋概率测试], itemId:" << m.first << ", itemCount:" << m.second << ", 概率：" << m.second << "/" << num << "\n";
  MsgManager::sendMsg(entry->id, 4001, MsgParams(stream.str()));
  XLOG << stream.str() << XEND;
  XLOG << "[GM-扭蛋概率测试]结束！" << XEND;
  return true;
}

bool GMCommandRuler::refinetest(xSceneEntryDynamic* entry, const xLuaData& params)
{
  #ifndef _DEBUG
    return true;
  #endif

  if (entry == nullptr)
    return false;

  DWORD equiptype = params.getTableInt("equiptype");
  DWORD quality = params.getTableInt("quality");
  DWORD personnum = params.getTableInt("person_num");
  DWORD count = params.getTableInt("count");
  DWORD maxlv = params.getTableInt("maxlv");

  if (equiptype == 0 || quality == 0 || personnum == 0 || count == 0)
    return false;

  struct testCost
  {
    DWORD equipnum = 0;
    DWORD itemnum = 0;
    bool have = false;
  };
  typedef std::vector<testCost> testOnePerson;
  testOnePerson oneResult;

  typedef std::vector<testOnePerson> testAllResult;
  testAllResult allResult;
  allResult.resize(personnum);

  const SRefineActionCFG& rCFG = MiscConfig::getMe().getRefineActionCFG();

  auto testrefine = [&](testOnePerson& myresult) -> DWORD
  {
    myresult.clear();
    myresult.resize(maxlv);

    DWORD templv = 0;
    bool lastfail = false;
    for (DWORD i = 0; i < count; ++i)
    {
      if (templv == maxlv)
      {
        XDBG << "[精炼-测试], 精炼次数:" << i << "达到要求等级" << XEND;
        return i;
      }
      const SRefineCFG* pCFG = ItemManager::getMe().getRefineCFG((EItemType)equiptype, (EQualityType)quality, templv + 1);
      if (pCFG == nullptr)
        continue;

      DWORD successRate = pCFG->sComposeRate.dwSuccessRate;
      DWORD failStatyRate = pCFG->sComposeRate.dwFailStayRate;
      DWORD failNoDamageRate = pCFG->sComposeRate.dwFailNoDamageRate;

      bool randSuccess = false;
      if (templv >= rCFG.dwBeginChangeRateLv)
      {
        DWORD dwRepairChRate = rCFG.fRepairPerRate * ONE_THOUSAND;
        DWORD dwChSuccessRate = successRate * ONE_THOUSAND + dwRepairChRate;
        if (lastfail)
        {
          DWORD dwChRate = rCFG.fLastFailAddRate * ONE_THOUSAND;
          dwChSuccessRate += dwChRate;
        }
        else
        {
          DWORD dwChRate = rCFG.fLastSuccessDecRate * ONE_THOUSAND;
          dwChSuccessRate = dwChSuccessRate > dwChRate ? dwChSuccessRate - dwChRate : 0;
        }
        if ((DWORD)randBetween(1, 100 * ONE_THOUSAND) <= dwChSuccessRate)
          randSuccess = true;
      }
      else
      {
        if ((DWORD)randBetween(1, 100) <= successRate)
          randSuccess = true;
      }

      myresult[templv].itemnum ++;

      if (randSuccess)
      {
        myresult[templv].have = true;

        templv ++;
        lastfail = false;
      }
      else
      {
        lastfail = true;
        //损坏
        if ((DWORD)randBetween(1, 100) > failNoDamageRate)
        {
          myresult[templv].equipnum ++;
        }
        //退级
        if ((DWORD)randBetween(1, 100) > failStatyRate)
        {
          if (templv) templv --;
        }
      }
    }
    return count;
  };

  for (auto &v : allResult)
  {
    testrefine(v);
  }

  std::sort(allResult.begin(), allResult.end(), [&](const testOnePerson& r1, const testOnePerson& r2) -> bool
  {
    testOnePerson myr1 = r1;
    testOnePerson myr2 = r2;
    for (DWORD i = 0; i < maxlv; ++i)
    {
      if (i == 0)
        continue;
      myr1[i].itemnum += myr1[i-1].itemnum;
      myr1[i].equipnum += myr1[i-1].equipnum;
      myr2[i].itemnum += myr2[i-1].itemnum;
      myr2[i].equipnum += myr2[i-1].equipnum;
    }
    for (int i = maxlv - 1; i >= 0; --i)
    {
      if (myr1[i].have == false && myr2[i].have == false)
        continue;
      if (myr1[i].have == false)
        return false;
      if (myr2[i].have == false)
        return true;
      return myr1[i].itemnum < myr2[i].itemnum;
    }
    return false;
  });

  XDBG << "[精炼-测试], 测试等级:" << maxlv << "测试装备:" << equiptype << "品质" << quality << "测试人数:" << personnum << "每人测试次数" << count << XEND;

  std::stringstream stream;
  stream.str("");
  stream << "[精炼-测试], 测试等级:" << maxlv << "测试装备:" << equiptype << "品质" << quality << "测试人数:" << personnum << "每人测试次数" << count << "\n";
  string str = stream.str();
  MsgManager::sendMsg(entry->id, 4001, MsgParams(str));

  XDBG << "[精炼-测试], 表现最好:" << XEND;

  stream.str("");
  stream << "[精炼-测试], 表现最好:" << "\n";
  auto printlog = [&](const testOnePerson& r)
  {
    DWORD itemnum = 0;
    DWORD repairnum = 0;
    DWORD mymaxlv = 0;
    for (DWORD i = 0; i < maxlv; ++i)
    {
      XDBG << i << "级 升" << i + 1 << "级" << ": 精炼次数" << r[i].itemnum << " 修复次数" << r[i].equipnum << XEND;
      stream << i << "级 升" << i + 1 << "级" << ": 精炼次数" << r[i].itemnum << " 修复次数" << r[i].equipnum << "\n";
      itemnum += r[i].itemnum;
      repairnum += r[i].equipnum;
      if (r[i].have)
        mymaxlv = i + 1;
    }
    XDBG << "总的精炼次数:" << itemnum << "总的修复次数" << repairnum << "最高等级:" << mymaxlv << XEND;
    stream << "总的精炼次数:" << itemnum << "总的修复次数" << repairnum << "最高等级:" << mymaxlv << "\n";
  };
  auto it = allResult[0];
  printlog(it);
  str = stream.str();
  MsgManager::sendMsg(entry->id, 4001, MsgParams(str));

  XDBG << "[精炼-测试], 表现最差:" << XEND;
  stream.str("");
  stream << "[精炼-测试], 表现最差:" << "\n";
  it = allResult[personnum - 1];
  printlog(it);
  str = stream.str();
  MsgManager::sendMsg(entry->id, 4001, MsgParams(str));

  XDBG << "[精炼-测试], 表现中等:" << XEND;
  stream.str("");
  stream << "[精炼-测试], 表现中等:" << "\n";
  it = allResult[(personnum - 1) / 2];
  printlog(it);
  str = stream.str();
  MsgManager::sendMsg(entry->id, 4001, MsgParams(str));

  XDBG << "[精炼-测试], 平均表现:" << XEND;
  stream.str("");
  stream << "[精炼-测试], 平均表现:" << "\n";
  testOnePerson aveone;
  aveone.resize(maxlv);
  DWORD succnum = 0;
  for (auto &v : allResult)
  {
    for (DWORD i = 0; i < maxlv; ++i)
    {
      aveone[i].itemnum += v[i].itemnum;
      aveone[i].equipnum += v[i].equipnum;
    }
    if (v[maxlv - 1].have)
      succnum ++;
  }
  for (auto &v : aveone)
  {
    v.itemnum /= personnum;
    v.equipnum /= personnum;
    v.have = true;
  }
  printlog(aveone);
  str = stream.str();
  MsgManager::sendMsg(entry->id, 4001, MsgParams(str));

  stream.str("");
  stream << "[精炼-测试], 达成精炼" << maxlv << "玩家数:" << succnum << "/" << personnum << "\n";
  XDBG << "[精炼-测试], 达成精炼" << maxlv << "玩家数:" << succnum << "/" << personnum << XEND;
  str = stream.str();
  MsgManager::sendMsg(entry->id, 4001, MsgParams(str));

  return true;
}

bool GMCommandRuler::unlockweaponpet(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  if (params.has("num"))
  {
    DWORD num = params.getTableInt("num");
    pUser->getWeaponPet().setMaxSize(num + pUser->getWeaponPet().getMaxSize());
  }
  if (params.has("id"))
  {
    DWORD id = params.getTableInt("id");
    pUser->getWeaponPet().unlock(id);
  }
  return true;
}

bool GMCommandRuler::seenpc(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (params.has("id") == false)
    return false;
  DWORD id = params.getTableInt("id");
  const string& action = params.getTableString("action");
  bool isdel = params.getTableInt("del") != 0;
  if (isdel)
  {
    if (action == "see")
      pUser->delSeeNpc(id);
    else if (action == "hide")
      pUser->delHideNpc(id);
  }
  else
  {
    if (action == "see")
      pUser->addSeeNpc(id);
    else if (action == "hide")
      pUser->addHideNpc(id);
  }
  return true;
}

bool GMCommandRuler::reducehp(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (entry == nullptr)
    return false;
  if (params.has("hp"))
  {
    DWORD limit = params.getTableInt("limit");
    DWORD dechp = params.getTableInt("hp");
    DWORD myhp = entry->getAttr(EATTRTYPE_HP);
    if (limit && myhp < limit + dechp)
      dechp = myhp > limit ? myhp - limit : 0;
    entry->changeHp(-(int)dechp, entry);
    XDBG << "[GM-reducehp], 对象:" << entry->name << entry->id << "扣除血量:" << dechp << XEND;
  }

  if (params.has("hpper"))
  {
    DWORD dechp = entry->getAttr(EATTRTYPE_MAXHP) * params.getTableFloat("hpper");
    entry->changeHp(-(int)dechp, entry);
  }

  if (params.has("now_hpper"))
  {
    DWORD dechp = entry->getAttr(EATTRTYPE_HP) * params.getTableFloat("now_hpper");
    entry->changeHp(-(int)dechp, entry);
  }
  return true;
}

bool GMCommandRuler::decsealcount(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  DWORD nowcnt = pUser->getVar().getVarValue(EVARTYPE_SEAL);

  bool bAdd = params.getTableInt("isadd") == 1;
  if (!bAdd)
    nowcnt += 1;
  else
    nowcnt = nowcnt > 0 ? nowcnt - 1: 0;

  pUser->getVar().setVarValue(EVARTYPE_SEAL, nowcnt);
  XLOG << "[GM-扣除裂隙次数], 玩家:" << pUser->name << pUser->id << "当前次数:" << nowcnt << XEND;
  return true;
};

bool GMCommandRuler::changequest(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (!params.has("questid") || !params.has("checkstep") || !params.has("gostep"))
    return false;
  DWORD questid = params.getTableInt("questid");
  DWORD checkstep = params.getTableInt("checkstep");
  DWORD gostep = params.getTableInt("gostep");

  if (pUser->getQuest().hasAcceptQuest(questid) == false)
    return true;
  DWORD mystep = pUser->getQuest().getQuestStep(questid);
  if (mystep >= checkstep)
  {
    pUser->getQuest().setQuestStep(questid, gostep);
    XLOG << "[GM-changequest], 更换任务step, 玩家:" << pUser->name << pUser->id << "任务ID:" << questid << "原来步骤:" << mystep << "设置步骤:" << gostep << XEND;
  }

  return true;
}

bool GMCommandRuler::addquota(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  if (!params.has("num"))
    return false;

  QWORD count = params.getTableQWORD("num");
  if (count == 0)
    return true;
  
  DWORD mailId = params.getTableInt("mailid");
  DWORD expireday = params.getTableInt("expireday");
  pUser->getDeposit().addQuota(count,Cmd::EQuotaType_G_Charge, mailId, expireday);    //
  return true;
}

bool GMCommandRuler::subquota(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  if (!params.has("num"))
    return false;

  QWORD count = params.getTableQWORD("num");
  if (count == 0)
    return true;
  pUser->getDeposit().subQuota(count, EQuotaType_C_Give);  
  return true;
}

bool GMCommandRuler::clearquota(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  pUser->getDeposit().clearQuota();
  return true;
}

bool GMCommandRuler::joinpvp(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // get param
  if (!params.has("t"))
    return false;
  
  EPvpType type = static_cast<EPvpType>(params.getTableInt("t"));  
  QWORD roomId = params.getTableQWORD("id");
  bool isQuick = params.getTableInt("quick");
  
  JoinRoomCCmd cmd;
  cmd.set_type(type);
  cmd.set_roomid(roomId);
  cmd.set_isquick(isQuick);
  
  PROTOBUF(cmd, send, len);
 
  thisServer->forwardCmdToSessionUser(pUser->id, send, len);
  XDBG << "[GM-加入竞技场] charid" << pUser->id << "type" << type << "roomid" << roomId << "isquick" << isQuick << XEND;
  return true;
}

bool GMCommandRuler::deletePassword(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DeletePwdSessionCmd cmd;
  cmd.set_charid(pUser->id);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

  return true;
}

bool GMCommandRuler::questnpc(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD dwQuestID = params.getTableInt("questid");
  DWORD dwNpcID = params.getTableInt("npcid");
  DWORD dwMapID = params.getTableInt("mapid");
  if (dwQuestID == 0 || dwNpcID == 0 || dwMapID == 0)
  {
    XERR << "[GM指令-任务npc]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "设置questid :" << dwQuestID << "npcid :" << dwNpcID << "mapid :" << dwMapID << "失败,参数不对" << XEND;
    return false;
  }
  pUser->getQuestNpc().setQuestNpcInfo(dwQuestID, dwNpcID, dwMapID);
  XLOG << "[GM指令-任务npc]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "设置questid :" << dwQuestID << "npcid :" << dwNpcID << "mapid :" << dwMapID << "成功" << XEND;
  return true;
}

bool GMCommandRuler::catchpet(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  QWORD qwTargetID = params.getTableQWORD("id1");
  if (qwTargetID == 0)
  {
    XERR << "[GM-宠物捕捉], 缺少目标ID, 玩家" << pUser->name << pUser->id << XEND;
    return false;
  }

  SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(qwTargetID);
  if (npc == nullptr)
  {
    XERR << "[GM-宠物捕捉], 目标不合法:" << qwTargetID << "玩家:" << pUser->name << pUser->id << XEND;
    return false;
  }
  DWORD npcid = npc->getNpcID();

  TSetDWORD npcids;
  auto getids = [&](const string& key, xLuaData& d)
  {
    npcids.insert(d.getInt());
  };
  xLuaData npcdata = params.getData("npcid");
  npcdata.foreach(getids);
  if (npcids.find(npcid) == npcids.end())
  {
    XERR << "[GM-宠物捕捉], 目标不合法:" << qwTargetID << "玩家:" << pUser->name << pUser->id << XEND;
    return false;
  }

  pUser->getUserPet().preCatchPet(npc);
  return true;
}

bool GMCommandRuler::precatchpet(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (0 == strcmp(params.getTableString("action"), "build_relation"))
  {
    float range = 10;
    if (params.has("range"))
      range = params.getTableFloat("range");

    if (params.has("npcid") == false)
      return false;

    Scene* pScene = pUser->getScene();
    if (pScene == nullptr)
      return false;

    DWORD npcid = params.getTableInt("npcid");
    xSceneEntrySet npcset;
    pScene->getEntryListInBlock(SCENE_ENTRY_NPC, pUser->getPos(), range, npcset);

    SceneNpc* npc = nullptr;
    for (auto &s : npcset)
    {
      SceneNpc* p = dynamic_cast<SceneNpc*> (s);
      if (p == nullptr)
        continue;
      if (p->getNpcID() == npcid)
      {
        npc = p;
        break;
      }
    }
    if (npc == nullptr)
    {
      XERR << "[GM-precatchpet], 玩家:" << pUser->name << pUser->id << "npcid:" << npcid << "未找到" << XEND;
      return false;
    }

    if (pUser->getUserPet().preCatchPet(npc) == false)
    {
      XERR << "[GM-precatchpet], 玩家:" << pUser->name << pUser->id << "npcid:" << npcid << "建立关系失败" << XEND;
      return false;
    }
    if (params.getTableInt("no_disperse") == 1)
    {
      QWORD catchid = pUser->getUserPet().getCatchPetID();
      SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(catchid);
      CatchPetNpc* pPet = dynamic_cast<CatchPetNpc*> (npc);
      if (pPet)
        pPet->setNoDisperse();
    }
    XLOG << "[GM-precatchpet], 玩家:" << pUser->name << pUser->id << "npcid:" << npcid << "建立关系成功" << XEND;
  }
  else if (0 == strcmp(params.getTableString("action"), "add_full_value"))
  {
    QWORD catchid = pUser->getUserPet().getCatchPetID();
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(catchid);
    CatchPetNpc* pPet = dynamic_cast<CatchPetNpc*> (npc);
    if (pPet == nullptr)
    {
      XERR << "[GM-precatchpet], 玩家:" << pUser->name << pUser->id << "找不到捕捉npc" << XEND;
      return false;
    }
    pPet->addCatchValue(100);
    XLOG << "[GM-precatchpet], 玩家:" << pUser->name << pUser->id << "添加捕获值成功, 捕获npc:" << pPet->name << pPet->id << XEND;
  }
  else if (0 == strcmp(params.getTableString("action"), "catch"))
  {
    QWORD catchid = pUser->getUserPet().getCatchPetID();
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(catchid);
    CatchPetNpc* pPet = dynamic_cast<CatchPetNpc*> (npc);
    if (pPet == nullptr)
    {
      XERR << "[GM-precatchpet], catch 玩家:" << pUser->name << pUser->id << "找不到捕捉npc" << XEND;
      return false;
    }
    pPet->catchMe();
  }
  else
  {
    XERR << "[GM-precatchpet], 未指定有效action" << "玩家:" << pUser->name << pUser->id << XEND;
    return false;
  }

  return true;
}

bool GMCommandRuler::genderreward(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  string gender;
  if (pUser->getUserSceneData().getGender() == EGENDER_MALE)
    gender = "male";
  else
    gender = "female";

  DWORD dwMenuId = params.getTableInt(gender);
  if (dwMenuId == 0)
    return false;
  const SMenuCFG* pCFG = MenuConfig::getMe().getMenuCFG(dwMenuId);
  if (pCFG == nullptr)
    return false;
  if (pUser->getMenu().isOpen(pCFG->id) == true)
  {
    MsgManager::sendMsg(pUser->id, 3616);
    XERR << "[GM指令-gendereward] menu已经解锁过" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "性别" << gender << "menuid" << dwMenuId << "失败" << XEND;
    return false;
  }
  pUser->getMenu().m_setValidMenus.insert(pCFG->id);
  pUser->getMenu().processMenuEvent(pCFG->id);
  NewMenu cmd;
  cmd.set_animplay(true);
  cmd.add_list(pCFG->id);
  PROTOBUF(cmd, send, len);
  pUser->sendCmdToMe(send, len);
  pUser->getEvent().onMenuOpen(dwMenuId);
  XLOG << "[GM指令-gendereward]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
    << "性别" << gender << "menuid" << dwMenuId << "成功" << XEND;
  return true;
}

bool GMCommandRuler::codeused(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  string guid = params.getTableString("guid");
  if (guid.empty())
  {
    XERR << "[道具-礼包码] 设置兑换记录，guid为空, charid" << pUser->id << XEND;
    return false;
  }

  BasePackage*pPkg = nullptr;
  ItemCode*pItemCode = dynamic_cast<ItemCode*>(pUser->getPackage().getItem(guid, &pPkg));
  if (!pItemCode)
  {
    XERR << "[道具-礼包码] 设置兑换记录，找不到道具, charid" << pUser->id << "guid" << guid << XEND;
    return false;
  }
  pItemCode->setExchanged(true);
  if (pPkg)
    pPkg->setUpdateIDs(guid);

  XLOG << "[道具-礼包码] 设置兑换记录成功, charid" << pUser->id << "guid" << guid << XEND;
  return true;
}

bool GMCommandRuler::codeitem(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  string guid = params.getTableString("guid");
  if (guid.empty())
  {
    XERR << "[道具-礼包码] 使用兑换码，guid为空, charid" << pUser->id << XEND;
    return false;
  }

  UseCodItemCmd cmd;
  cmd.set_guid(guid);
  pUser->getPackage().useItemCode(cmd);
  return true;
}

bool GMCommandRuler::setlotterycnt(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD t = params.getTableInt("t");    //扭蛋机机类型
  if (!ELotteryType_IsValid(t))
    return false;

  DWORD count = params.getTableInt("count");
  DWORD opt = params.getTableInt("opt");

  ELotteryType lotteryType = ELotteryType(t);
  EVarType varType = pUser->getLottery().getVarCntType(lotteryType);
  EAccVarType varAccType = pUser->getLottery().getAccVarType(lotteryType);

  if(0 == opt)
  {
    pUser->getVar().setVarValue(varType, count);
    pUser->getVar().setAccVarValue(varAccType, count);
  }
  else if(1 == opt)
    pUser->getVar().setVarValue(varType, count);
  else if(2 == opt)
    pUser->getVar().setAccVarValue(varAccType, count);
  else
    return false;

  XLOG << "[GM-扭蛋机次数] 设置扭蛋机次数" << pUser->id << pUser->name << "扭蛋机类型" << lotteryType << "opt" << opt << "vartype" << varType << "varAccType" << varAccType << "count" << count << XEND;
  return true;
}

bool GMCommandRuler::clearlotterylimit(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  DWORD t = params.getTableInt("t");    
  if (!ELotteryType_IsValid(t))
    return false;
  ELotteryType lotteryType = ELotteryType(t);
  EOptionType eOptType = Lottery::getOptionType(lotteryType);
  
  //0 不限制 1限制，默认是限制
  DWORD flag = params.getTableInt("flag");
  pUser->getUserSceneData().setOption(eOptType, flag);

  XLOG << "[GM-扭蛋限制次数] 设置是否限制" << pUser->id << pUser->name << "扭蛋机类型" << lotteryType << "eOptType" << eOptType << "限制" << flag << XEND;
  return true;
}

bool GMCommandRuler::marry(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  
  if (pUser->getUserWedding().getMaritalState() != EMARITAL_RESERVED)
  {
    XERR <<"[GM-结婚]" <<pUser->id <<pUser->name <<"不是订婚状态" <<pUser->getUserWedding().getMaritalState() << XEND;
    return  false;
  }
    
  MarrySCmd cmd;
  cmd.set_charid1(pUser->id);
  cmd.set_charid2(pUser->getUserWedding().getWeddingInfo().charid2());
  cmd.set_weddingid(pUser->getUserWedding().getWeddingInfo().id());
  PROTOBUF(cmd, send, len);  
  thisServer->sendSCmdToWeddingServer(pUser->id, pUser->name, send, len);
  XLOG << "[GM-结婚] 发送到婚礼服" << pUser->id << pUser->name << "id"<<cmd.weddingid() << XEND;
  return true;
}

bool GMCommandRuler::divorce(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser->getUserWedding().getMaritalState() != EMARITAL_MARRIED)
  {
    XERR << "[GM-结婚]" << pUser->id << pUser->name << "不是结婚状态" << pUser->getUserWedding().getMaritalState() << XEND;
    return  false;
  }
  
  EGiveUpType et = EGiveUpType_Together;
  DWORD t = params.getTableInt("t");
  if (EGiveUpType_IsValid(t))
  {
    et = static_cast<EGiveUpType> (t);
  }  
  ReqDivorceCCmd cmd;
  cmd.set_id(pUser->getUserWedding().getWeddingInfo().id());
  cmd.set_type(et);
  PROTOBUF(cmd, send, len);
  thisServer->sendUserCmdToWeddingServer(pUser->id, pUser->name, send, len);
  XLOG << "[GM-离婚] 发送到婚礼服" << pUser->id << pUser->name << "id" << cmd.id() << XEND;
  return true;
}

bool GMCommandRuler::divorcemail(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser->getUserWedding().getMaritalState() != EMARITAL_MARRIED)
    return false;
  
  //12104 亲爱的冒险家，您的伴侣%s已经发起了强制离婚的申请，TA最快将在5天后解除与你的婚姻关系，如果可以的话还是抢救一下吧！
  MsgParams mailParams;
  mailParams.addString(pUser->name);
  bool ret = MailManager::getMe().sendMail(pUser->getUserWedding().getWeddingParnter(), 12104, mailParams);
  XLOG <<"[婚礼-强制离婚-邮件通知对方] " <<pUser->id <<pUser->name << pUser->getUserWedding().getWeddingParnter() << XEND;
  return ret;
}

bool GMCommandRuler::forcedivorce(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser->getUserWedding().getMaritalState() != EMARITAL_MARRIED)
  {
    XERR << "[GM-结婚]" << pUser->id << pUser->name << "不是结婚状态" << pUser->getUserWedding().getMaritalState() << XEND;
    return  false;
  }

  EGiveUpType et = EGiveUpType_Force;
  ReqDivorceCCmd cmd;
  cmd.set_id(pUser->getUserWedding().getWeddingInfo().id());
  cmd.set_type(et);
  PROTOBUF(cmd, send, len);
  thisServer->sendUserCmdToWeddingServer(pUser->id, pUser->name, send, len);
  XLOG << "[GM-强制离婚] 发送到婚礼服" << pUser->id << pUser->name << "id" << cmd.id() << XEND;
  return true;
}

bool GMCommandRuler::pvecardeffect(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (entry == nullptr)
    return false;
  DScene* pScene = dynamic_cast<DScene*>(entry->getScene());
  if (pScene == nullptr)
    return false;

  xLuaData idsd = params.getData("effectid");
  TSetDWORD ids;
  idsd.getIDList(ids);
  for (auto &s : ids)
  {
    PveCardBase* pCardEffect = PveCardManager::getMe().getCardEffect(s);
    if (pCardEffect == nullptr)
    {
      XERR << "[GM-pvecardeffect], 执行失败, id:" << s << "对象:" << entry->name << entry->id << XEND;
      continue;
    }

    pCardEffect->doEffect(pScene);
    XLOG << "[GM-pvecardeffct], 执行成功, 对象:" << entry->name << entry->id << "效果id:" << s << XEND;
  }

  return true;
}

bool GMCommandRuler::servant(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD opt = params.getTableInt("option");
  if(opt == 1)    //添加好感度
  {
    DWORD dwValue = params.getTableInt("value");
    pUser->getServant().addFavoribility(dwValue);
  }
  else if(opt == 2)   //设置女仆
  {
    DWORD servantid = params.getTableInt("value");
    if(MiscConfig::getMe().getServantCFG().isExist(servantid) == false)
      return false;
    if(pUser->getMenu().isOpen(EMENUID_SERVANT) == false)
    {
      pUser->getMenu().m_setValidMenus.emplace(EMENUID_SERVANT);
      pUser->getMenu().processMenuEvent(EMENUID_SERVANT);
    }

    bool replace = params.getTableInt("firstservant") == 0;
    pUser->getServant().setServant(replace, servantid);
  }
  else if(opt == 3)
  {
    DWORD dwID = params.getTableInt("value");
    const SServantItemCFG* pItemCFG = ServantConfig::getMe().getServantCFG(dwID);
    if(pItemCFG == nullptr)
      return false;
    RecommendItemInfo* pItem = pUser->getServant().getRecommendItem(dwID);
    if(pItem && pItem->status() >= ERECOMMEND_STATUS_RECEIVE)
      return false;
    pUser->getServant().setRecommendStatus(pItem, ERECOMMEND_STATUS_RECEIVE, false);
    pUser->getServant().refreshRcommendInfo(dwID);
  }
  else if(opt == 4)
  {
    DWORD dwID = params.getTableInt("value");
    GrowthItemInfo* pItem = pUser->getServant().getGrowthItem(dwID);
    if(pItem && pItem->status() >= EGROWTH_STATUS_RECEIVE)
      return false;
    if(pItem && pUser->getServant().checkExistCurGroup(dwID / SERVANT_GROWTH_ID_PARAM))
    {
      pUser->getServant().setGrowthStatus(pItem, EGROWTH_STATUS_RECEIVE, false);
      pUser->getServant().refreshGrowthInfo(dwID, false);
    }
  }
  else if(opt == 5)
  {
    DWORD dwID = params.getTableInt("value");
    if(pUser->getServant().checkExistCurGroup(dwID))
      pUser->getServant().finishGrowthGroup(dwID);
  }

  XLOG << "[GM-仆人] 设置" << pUser->id << pUser->name << "操作类型" << opt << XEND;
  return true;
}

bool GMCommandRuler::branchremove(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  DWORD dwID = params.getTableInt("branch");
  if(0 == dwID) return false;

  pUser->m_oProfession.remove(dwID);
  return true;
}

bool GMCommandRuler::branchrestore(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  pUser->m_oProfession.restore();
  return true;
}

// 分支测试
bool GMCommandRuler::branchcmd(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  DWORD dwCmd = params.getTableInt("cmd");
  if(0 == dwCmd) return false;
  DWORD dwBranch = params.getTableInt("branch");

  switch(dwCmd)
  {
    case 1:
      {
        Cmd::ProfessionQueryUserCmd cmd;
        pUser->m_oProfession.queryBranchs(cmd);
      }
      break;
    case 2:
      {
        pUser->buyProfession(dwBranch);
      }
      break;
    case 3:
      {
        pUser->changeProfession(dwBranch);
      }
      break;
    default:
      return false;
  }

  return true;
}

bool GMCommandRuler::resetmainchar(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  pUser->setMainCharId(0);
  return true;
}

bool GMCommandRuler::slotcmd(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  DWORD dwCmd = params.getTableInt("cmd");
  if(0 == dwCmd) return false;
  DWORD dwSlotID = params.getTableInt("slotid");

  switch(dwCmd)
  {
    case 1:
      {
        //保存 存档
        std::string record_name = params.getTableString("name");
        SaveRecordUserCmd msg;
        msg.set_slotid(dwSlotID);
        msg.set_record_name(record_name);

        pUser->getUserRecords().userSaveRecord(msg);
      }
      break;
    case 2:
      {
        //读取存档
        DWORD multi = params.getTableInt("multi");
        LoadRecordUserCmd msg;
        msg.set_slotid(dwSlotID);
        if (multi > 0)
          pUser->getUserRecords().userLoadRecord(msg, true);
        else
          pUser->getUserRecords().userLoadRecord(msg);
      }
      break;
    case 3:
      {
        //删除存档
        DeleteRecordUserCmd msg;
        msg.set_slotid(dwSlotID);

        pUser->getUserRecords().userDeleteRecord(msg);
      }
      break;
    case 4:
      {
        //购买存档位
        BuyRecordSlotUserCmd msg;
        msg.set_slotid(dwSlotID);

        pUser->getUserRecords().userBuySlot(msg);
      }
      break;
    case 5:
      {
        //更改存档名
        std::string record_name = params.getTableString("name");
        ChangeRecordNameUserCmd msg;
        msg.set_slotid(dwSlotID);
        msg.set_record_name(record_name);

        pUser->getUserRecords().userChangeRecordName(msg);
      }
      break;
    default:
      return false;
  }

  return true;
}

// --------------------------------- scene gm ----------------------------------

GmSceneCmd GMSceneCmds[]=
{
  {"summon", GMCommandRuler::scene_summon, HUMAN_NORMAL, "场景招怪"},
  {"clearnpc", GMCommandRuler::scene_clearnpc, HUMAN_NORMAL, "场景清怪"},
  {"dsummon", GMCommandRuler::dscene_summon, HUMAN_NORMAL, "动态场景招怪" },
  {"dclearnpc", GMCommandRuler::dscene_clearnpc, HUMAN_NORMAL, "动态场景清怪" },
  {"clearvisiblenpc", GMCommandRuler::scene_clearvisiblenpc, HUMAN_NORMAL, "场景清小地图可见怪"},
  {"show_npc", GMCommandRuler::scene_shownpc, HUMAN_NORMAL, "场景显示怪物"},
  {"hide_npc", GMCommandRuler::scene_hidenpc, HUMAN_NORMAL, "场景隐藏怪物"},
  {"set_weather", GMCommandRuler::scene_weather, HUMAN_NORMAL, "场景设置天气"},
  {"set_sky", GMCommandRuler::scene_sky, HUMAN_NORMAL, "场景设置天空"},
  {"setenv", GMCommandRuler::setenv, HUMAN_NORMAL, "场景设置天空天气" },
  {"setbgm", GMCommandRuler::setbgm, HUMAN_NORMAL, "场景设置bgm" },
  {"scene_se", GMCommandRuler::scene_se, HUMAN_NORMAL, "场景播放音效" },

  {"replace_reward", GMCommandRuler::scene_replace_reward, HUMAN_NORMAL, "场景改变奖励"},
  {"recover_reward", GMCommandRuler::scene_recover_reward, HUMAN_NORMAL, "场景恢复改变奖励"},
  {"startactivity", GMCommandRuler::startactivity, HUMAN_NORMAL, "开启活动"},
  {"stopactivity", GMCommandRuler::stopactivity, HUMAN_NORMAL, "关闭活动" },

  {"startglobalactivity", GMCommandRuler::startglobalactivity, HUMAN_NORMAL, "开启全服活动"},
  {"stopglobalactivity", GMCommandRuler::stopglobalactivity, HUMAN_NORMAL, "关闭全服活动" },

  {"add_extra_reward", GMCommandRuler::scene_add_extra_reward, HUMAN_NORMAL, "添加额外怪物掉落" },
  {"del_extra_reward", GMCommandRuler::scene_del_extra_reward, HUMAN_NORMAL, "删除额外怪物掉落" },

  {"replace_board_reward", GMCommandRuler::scene_replace_board_reward, HUMAN_NORMAL, "添加看板奖励替换" },
  {"recover_board_reward", GMCommandRuler::scene_recover_board_reward, HUMAN_NORMAL, "删除看板奖励替换" },

  {"add_event_reward", GMCommandRuler::scene_add_event_reward, HUMAN_NORMAL, "活动添加完成XX额外奖励" },
  {"del_event_reward", GMCommandRuler::scene_del_event_reward, HUMAN_NORMAL, "活动删除完成XX额外奖励" },

  {"add_double_event_reward", GMCommandRuler::scene_add_double_event_reward, HUMAN_NORMAL, "活动添加完成XX多倍奖励" },
  {"del_double_event_reward", GMCommandRuler::scene_del_double_event_reward, HUMAN_NORMAL, "活动删除完成XX多倍奖励" },

  {"open_npcfunc", GMCommandRuler::scene_open_npcfunc, HUMAN_NORMAL, "开启npc功能" },
  {"close_npcfunc", GMCommandRuler::scene_close_npcfunc, HUMAN_NORMAL, "关闭npc功能" },

  {"reset_guild_raid", GMCommandRuler::resetGuildRaid, HUMAN_NORMAL, "重置公会副本" },
  {"recover_guild_raid", GMCommandRuler::recoverGuildRaid, HUMAN_NORMAL, "取消公会副本重置标志" },

  {"changebody", GMCommandRuler::scene_changebody, HUMAN_NORMAL, "场景怪物变身" },
  {"showseat", GMCommandRuler::scene_showseat, HUMAN_NORMAL, "场景摇摇乐作为显示" },
  {"gvg_tool", GMCommandRuler::scene_gvg, HUMAN_NORMAL, "gvg操作" },
  {"rangebgm", GMCommandRuler::scene_rangebgm, HUMAN_NORMAL, "场景改变区域bgm" },
  {"rangesky", GMCommandRuler::scene_rangesky, HUMAN_NORMAL, "场景改变区域天空" },
  {"rangeweather", GMCommandRuler::scene_rangeweather, HUMAN_NORMAL, "场景改变区域天气" },
  {"scene_effect", GMCommandRuler::scene_effect, HUMAN_NORMAL, "场景循环特效" },
  {"scene_dropitem", GMCommandRuler::scene_dropitem, HUMAN_NORMAL, "场景掉落道具" },
  {"scene_gear", GMCommandRuler::scene_gear, HUMAN_NORMAL, "设置机关状态" },
  {"scene_kickalluser", GMCommandRuler::scene_kickalluser, HUMAN_NORMAL, "踢掉地图里的所有人" },  
  {"scene_addquest", GMCommandRuler::scene_addquest, HUMAN_NORMAL, "下发地图所有人任务" },
  {"scene_delquest", GMCommandRuler::scene_delquest, HUMAN_NORMAL, "移除地图所有人任务" },
};

bool GMCommandRuler::scene_execute(Scene* pScene, const xLuaData& data)
{
  if (data.has("type") == false)
    return false;
  GmSceneFun pFun = 0;
  for(DWORD n=0;n<(sizeof(GMSceneCmds)/sizeof(GmSceneCmd));n++)
  {
    if(0==strncmp(GMSceneCmds[n].cmd, data.getTableString("type"), MAX_NAMESIZE))
    {
      pFun = GMSceneCmds[n].p;
    }
  }
  if (!pFun) return false;

  DWORD mapId = pScene == nullptr ? 0 : pScene->getMapID();
  if (data.has("Timer_Key"))
  {
    QWORD keyID = data.getTableInt("Timer_Key");
    keyID = keyID * 10000 + mapId;

    // 防止Session重启, 场景重复执行Timer相同指令
    if (MapConfig::getMe().checkHaveTimerKey(keyID))
    {
      XLOG << "[Timer-执行], 场景已执行过该指令" << keyID <<"mapid" <<mapId <<"原始keyid" <<keyID/10000 << XEND;
      return true;
    }
    if ((*pFun)(pScene, data))
    {
      MapConfig::getMe().addTimerKey(keyID);
      return true;
    }
    return false;
  }

  return (*pFun)(pScene, data);
}

bool GMCommandRuler::scene_summon(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;
  DWORD num = 1;
  if (params.has("num"))
    num = params.getTableInt("num");

  NpcDefine def;
  def.load(params);
  xPos pos = def.getPos();
  bool randomPos = false;
  if (pos.empty())
  {
    randomPos = true;
  }
  for (DWORD i = 0; i < num; ++i)
  {
    if (randomPos)
    {
      if (pScene->getRandPos(pos))
        def.setPos(pos);
    }
    SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(def, pScene);
    if (pNpc == nullptr)
    {
      XERR << "[Scene-GM], 招怪失败" << def.getID() << XEND;
      return false;
    }
    if(def.getVisibleInMap() == true)
    {
      pScene->addVisibleNpc(pNpc);
    }
    pScene->addSummonNpc(pNpc->id);
  }
  return true;
}

bool GMCommandRuler::scene_clearnpc(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr || params.has("id") == false)
    return false;

  DWORD npcid = params.getTableInt("id");
  const set<QWORD>& npcset = pScene->getSummonedNpcs();
  for (auto &q : npcset)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(q);
    if (npc == nullptr || npc->getNpcID() != npcid)
      continue;
    npc->removeAtonce();
  }

  return true;
}

bool GMCommandRuler::dscene_summon(Scene* pScene, const xLuaData& params)
{
  DWORD raidId = params.getTableInt("raidid");
  XLOG << "[dscene_summon-gm] raidid" << raidId <<params.getString() << XEND;
  if (raidId == 0)
    return false;

  DWORD num = 1;
  if (params.has("num"))
    num = params.getTableInt("num");
  
  NpcDefine def;
  def.load(params);
  xPos pos = def.getPos();
  bool randomPos = false;
  if (pos.empty())
  {
    randomPos = true;
  }

  auto func = [&](xEntry *e)->bool
  {
    DScene* pDs = dynamic_cast<DScene*>(e);
    if (pDs == nullptr)
      return false;
    if (pDs->getRaidID() != raidId)
      return false;

     for (DWORD i = 0; i < num; ++i)
     {
       if (randomPos)
       {
         if (pDs->getRandPos(pos))
           def.setPos(pos);
       }
       SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(def, pDs);
       if (pNpc == nullptr)
       {
         XERR << "[Scene-GM], 副本招怪失败" << def.getID() << XEND;
         return false;
       }
       if (def.getVisibleInMap() == true)
       {
         pDs->addVisibleNpc(pNpc);
         XLOG << "[Scene-GM] 副本招怪, 场景招小地图可见怪" << "Mapid" << pDs->getMapID() << "raidid" << pDs->getRaidID() << def.getID() << "总数" << num << XEND;
       }
       pDs->addSummonNpc(pNpc->id);
     }
     return true;
  };
  SceneManager::getMe().forEach2(func);  

  SceneManager::getMe().m_mapDsceneSummon[raidId] = params;
  return true;
}

bool GMCommandRuler::dscene_clearnpc(Scene* pScene, const xLuaData& params)
{
  DWORD raidId = params.getTableInt("raidid");
  XLOG << "[dscene_clearnpc-gm] raidid" << raidId << params.getString() << XEND;
  if (raidId == 0)
    return false;

  if (params.has("id") == false)
    return false;

  DWORD npcid = params.getTableInt("id");

  auto func = [&](xEntry *e)->bool
  {
    DScene* pDs = dynamic_cast<DScene*>(e);
    if (pDs == nullptr)
      return true;

    if (pDs->getRaidID() != raidId)
      return true;

    const set<QWORD>& npcset = pDs->getSummonedNpcs();
    for (auto &q : npcset)
    {
      SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(q);
      if (npc == nullptr || npc->getNpcID() != npcid)
        continue;
      XLOG << "[Scene-GM] 副本清空npc" << "Mapid" << pDs->getMapID() << "raidid" << pDs->getRaidID() << "npcid" << npc->getNpcID() << "总数" << npcset.size() << XEND;
      npc->removeAtonce();
    }
    return true;
  };

  SceneManager::getMe().forEach2(func);
  SceneManager::getMe().m_mapDsceneSummon.erase(raidId);
  return true;
}


bool GMCommandRuler::scene_clearvisiblenpc(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr || params.has("id") == false)
    return false;

  DWORD npcid = params.getTableInt("id");
  std::map<QWORD, SVisibleNpc> npcs = pScene->getVisibleNpcs();
  for (auto &q : npcs)
  {
    SceneNpc* npc = SceneNpcManager::getMe().getNpcByTempID(q.first);
    if (npc == nullptr || npc->getNpcID() != npcid)
      continue;
    pScene->delVisibleNpc(npc);
    npc->removeAtonce();
    XLOG << "[npc-小地图可见] 删除, npcid:" << npcid << XEND;
  }

  return true;
}

bool GMCommandRuler::scene_shownpc(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr || params.has("uniqueid") == false)
    return false;

  DWORD m_dwUniqueID = params.getTableInt("uniqueid");
  if (m_dwUniqueID == 0)
    return false;

  SceneNpcManager::getMe().delUniqueID(pScene, m_dwUniqueID);

  const SceneObject *pObject = pScene->getSceneObject();
  if (pObject)
  {
    const list<SceneNpcTemplate>& npclist = pObject->getNpcList();
    for (auto &s : npclist)
    {
      if (s.m_oDefine.getUniqueID() == m_dwUniqueID)
      {
        SceneNpcManager::getMe().createNpc(pScene, s.m_oDefine, s.m_dwNum);
        break;
      }
    }
    XLOG << "[scene-gm], shownpc, uniqueid:" << m_dwUniqueID << XEND;
  }

  return true;
}

bool GMCommandRuler::scene_hidenpc(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr || params.has("uniqueid") == false)
    return false;

  DWORD m_dwUniqueID = params.getTableInt("uniqueid");
  SceneNpcManager::getMe().delUniqueID(pScene, m_dwUniqueID);
  XLOG << "[scene-gm], hidenpc, uniqueid:" << m_dwUniqueID << XEND;

  return true;
}

bool GMCommandRuler::scene_weather(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr || params.has("id") == false)
    return false;
  DWORD weather = params.getTableInt("id");
  DWORD mode = params.getTableInt("mode");
  pScene->setEnvSetting(mode, 0, weather);
  return true;
}

bool GMCommandRuler::scene_sky(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr || params.has("id") == false)
    return false;
  DWORD sky = params.getTableInt("id");
  DWORD mode = params.getTableInt("mode");
  pScene->setEnvSetting(mode, sky, 0);
  return true;
}

bool GMCommandRuler::setenv(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr || params.has("mode") == false)
    return false;
  DWORD mode = params.getTableInt("mode");
  DWORD sky = params.getTableInt("sky");
  DWORD weather = params.getTableInt("weather");

  pScene->setEnvSetting(mode, sky, weather);
  return true;
}

bool GMCommandRuler::setbgm(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;
  string bgmPath;

  Cmd::EBgmType type = EBGM_TYPE_QUEST;
  if (params.has("type1"))
  {
    type = static_cast<EBgmType>(params.getTableInt("type1"));
  }

  if (params.has("bgm"))
  {
    bgmPath = params.getTableString("bgm");
  }
  DWORD times = 0;   //循环播放
  if (params.has("times"))
  {
    times = params.getTableInt("times");
  }
  bool play = false;
  if (params.has("play"))
  {
    play = params.getTableInt("play");
  }

  pScene->setBgm(type, play, times, bgmPath);

  return true;
}

bool GMCommandRuler::startactivity(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;
  DWORD id = 0;           //activity id
  if (params.has("id"))
  {
    id = params.getTableInt("id");
  }

  const SActivityCFG* pCfg = ActivityConfig::getMe().getActivityCFG(id);
  if (!pCfg)
  {
    XERR << "[活动-GM发起] 找不到活动配置 " << "activity id" << id << "mapid" << pScene->getMapID() << XEND;
    return false;
  }

  if (!pCfg->checkMap(pScene->getMapID()))
  {
    XINF << "[活动-GM发起] 错误的地图" << "activity id" << id << "mapid" << pScene->getMapID() << XEND;
    return false;
  }

  ActivityTestAndSetSessionCmd cmd;
  cmd.set_id(id);
  cmd.set_mapid(pScene->getMapID());
  //cmd.set_charid(0);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  XLOG << "[活动-GM发起] " << "activity id" << id << "mapid" << pScene->getMapID() << XEND;
  return true;
}

bool GMCommandRuler::stopactivity(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;
  DWORD id = 0;           //activity id
  if (params.has("id"))
  {
    id = params.getTableInt("id");
  }
  ActivityManager::getMe().stopActivity(id);
  return true;
}

bool GMCommandRuler::startglobalactivity(Scene* pScene, const xLuaData& params)
{
  DWORD id = 0;           //activity id
  if (params.has("id"))
  {
    id = params.getTableInt("id");
  }

  ActivityManager::getMe().addGlobalActivity(id);
  XLOG << "[全服活动-Scene] 开启" << "activity id" << id << XEND;
  return true;
}

bool GMCommandRuler::stopglobalactivity(Scene* pScene, const xLuaData& params)
{
  DWORD id = 0;           //activity id
  if (params.has("id"))
  {
    id = params.getTableInt("id");
  }

  ActivityManager::getMe().delGlobalActivity(id);
  XLOG << "[全服活动-Scene] 结束" << "activity id" << id << XEND;
  return true;
}

bool GMCommandRuler::scene_se(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;

  Cmd::SoundEffectCmd cmd;    
  cmd.set_se(params.getTableString("se"));
  PROTOBUF(cmd, send, len);
  pScene->sendCmdToAll(send, len);
  return true;
}

bool GMCommandRuler::scene_replace_reward(Scene* pScene, const xLuaData& params)
{
  if (!params.has("id") || !params.has("newid"))
    return false;

  DWORD id = params.getTableInt("id");
  DWORD newid = params.getTableInt("newid");

  if (RewardConfig::getMe().addReplace(id, newid) == false)
  {
    XERR << "[scene-gm], reward替换失败, id:" << id << "newid:" << newid <<XEND;
    return false;
  }

  return true;
}

bool GMCommandRuler::scene_recover_reward(Scene* pScene, const xLuaData& params)
{
  if (!params.has("id"))
    return false;

  DWORD id = params.getTableInt("id");
  if (RewardConfig::getMe().delReplace(id) == false)
  {
    XERR << "[scene-gm], reward恢复失败, id:" << id << XEND;
    return false;
  }

  return true;
}

bool GMCommandRuler::scene_replace_board_reward(Scene* pScene, const xLuaData& params)
{
  std::map<DWORD, TSetDWORD> mapIndex2Rwds;
  auto getrwds = [&](const string& key, xLuaData& data)
  {
    if (data.has("index") == false)
      return;
    TSetDWORD& rwds = mapIndex2Rwds[data.getTableInt("index")];
    auto getids = [&] (const string& k, xLuaData& d)
    {
      rwds.insert(d.getInt());
    };
    data.getMutableData("reward").foreach(getids);
  };
  xLuaData copydata = params;
  copydata.foreach(getrwds);

  if (mapIndex2Rwds.empty())
  {
    XERR << "[看板奖励-替换], 配置不合法" << XEND;
    return false;
  }

  for (auto &m : mapIndex2Rwds)
  {
    QuestConfig::getMe().addReplace(m.first, m.second);
  }

  SceneUserManager::getMe().syncWantedQuest();
  return true;
}

bool GMCommandRuler::scene_recover_board_reward(Scene* pScene, const xLuaData& params)
{
  TSetDWORD indexs;
  auto getindex = [&](const string& key, xLuaData& data)
  {
    indexs.insert(data.getInt());
  };
  xLuaData datas = params.getData("index");
  datas.foreach(getindex);

  if (indexs.empty())
  {
    XERR << "[看板奖励-替换], 删除替换, 配置不合法" << XEND;
    return false;
  }

  for (auto &s : indexs)
  {
    QuestConfig::getMe().delReplace(s);
  }

  return true;
}

bool GMCommandRuler::scene_add_extra_reward(Scene* pScene, const xLuaData& params)
{
  if (!params.has("reward"))
    return false;

  if (params.has("npcid"))
  {
    TSetDWORD npcset;
    TSetDWORD rwdset;
    DWORD type = 1;
    auto getids = [&](const string& key, xLuaData& data)
    {
      if (type == 1)
        npcset.insert(data.getInt());
      else if (type == 2)
        rwdset.insert(data.getInt());
    };
    xLuaData npcdata = params.getData("npcid");
    type = 1;
    npcdata.foreach(getids);
    xLuaData rwddata = params.getData("reward");
    type = 2;
    rwddata.foreach(getids);

    for (auto &s : npcset)
    {
      RewardConfig::getMe().addExtraRwd(s, rwdset);
    }
  }

  else
  {
    SExtraNpcRwd stRwd;

    auto gettypes = [&](const string& key, xLuaData& data)
    {
      const string& name = data.getString();
      ENpcType eType = NpcConfig::getMe().getNpcType(name);
      if (eType == ENPCTYPE_MIN)
      {
        XERR << "[GM-add_extra_reward], 怪物类型配置错误" << name << XEND;
        return;
      }
      stRwd.setNpcTypes.insert(eType);
    };
    xLuaData typedata = params.getData("npctype");
    typedata.foreach(gettypes);

    auto getzonetypes = [&](const string& key, xLuaData& data)
    {
      const string& name = data.getString();
      ENpcZoneType eType = NpcConfig::getMe().getZoneType(name);
      if (eType == ENPCZONE_MIN)
      {
        XERR << "[GM-add_extra_reward], 怪物区域配置错误" << name << XEND;
        return;
      }
      stRwd.setZoneTypes.insert(eType);
    };
    xLuaData zonedata = params.getData("zone");
    zonedata.foreach(getzonetypes);

    auto getids = [&](const string& key, xLuaData& data)
    {
      stRwd.setExtraIDs.insert(data.getInt());
    };
    xLuaData rwddata = params.getData("reward");
    rwddata.foreach(getids);

    auto getraces = [&](const string& key, xLuaData& data)
    {
      stRwd.setRaceTypes.insert(NpcConfig::getMe().getRaceType(data.getString()));
    };
    xLuaData racedata = params.getData("racetype");
    racedata.foreach(getraces);

    auto getnatures = [&](const string& key, xLuaData& data)
    {
      stRwd.setNatureTypes.insert(NpcConfig::getMe().getNatureType(data.getString()));
    };
    xLuaData naturedata = params.getData("naturetype");
    naturedata.foreach(getnatures);

    stRwd.exclude = params.getTableInt("exclude");
    stRwd.uniqueID = params.getTableInt("uniqueID");

    RewardConfig::getMe().addExtraRwd(stRwd);
  }

  return true;
}

bool GMCommandRuler::scene_del_extra_reward(Scene* pScene, const xLuaData& params)
{
  if (!params.has("reward"))
    return false;
  if (params.has("npcid"))
  {
    TSetDWORD npcset;
    TSetDWORD rwdset;
    DWORD type = 1;
    auto getids = [&](const string& key, xLuaData& data)
    {
      if (type == 1)
        npcset.insert(data.getInt());
      else if (type == 2)
        rwdset.insert(data.getInt());
    };
    xLuaData npcdata = params.getData("npcid");
    type = 1;
    npcdata.foreach(getids);
    xLuaData rwddata = params.getData("reward");
    type = 2;
    rwddata.foreach(getids);

    for (auto &s : npcset)
    {
      RewardConfig::getMe().delExtraRwd(s, rwdset);
    }
  }

  else
  {
    SExtraNpcRwd stRwd;
    stRwd.uniqueID = params.getTableInt("uniqueID");

    RewardConfig::getMe().delExtraRwd(stRwd);
  }

  return true;
}

bool GMCommandRuler::scene_add_event_reward(Scene* pScene, const xLuaData& params)
{
  if (params.has("name") == false)
    return false;
  string name = params.getTableString("name");
  SExtraRewardData sData;
  EExtraRewardType eType;
  if (name == "board_quest")
    eType = EEXTRAREWARD_WANTEDQUEST;
  else if (name == "resist_monster")
    eType = EEXTRAREWARD_DAILYMONSTER;
  else if (name == "seal")
    eType = EEXTRAREWARD_SEAL;
  else if (name == "laboratory")
    eType = EEXTRAREWARD_LABORATORY;
  else if (name == "endless_tower")
    eType = EEXTRAREWARD_ENDLESS;
  else if (name == "guild_quest")
    eType = EEXTRAREWARD_GUILD_QUEST;
  else if (name == "guild_donate")
    eType = EEXTRAREWARD_GUILD_DONATE;
  else if (name == "pve_card")
    eType = EEXTRAREWARD_PVECARD;
  else if (name == "gvg")
    eType = EEXTRAREWARD_GVG;
  else if (name == "super_gvg")
    eType = EEXTRAREWARD_SUPERGVG;
  else
  {
    XERR << "[GM-事件奖励-添加], 未指定正确类型" << name << XEND;
    return false;
  }

  sData.eType = eType;
  sData.dwNeedTimes = params.getTableInt("need_times");
  sData.dwDayRewardTimes = params.getTableInt("day_limits");
  sData.dwAccLimit = params.getTableInt("acclimit");
  auto getrwds = [&](const string& key, xLuaData& d)
  {
    sData.setRewards.insert(d.getInt());
  };
  xLuaData tmpdata = params.getData("reward");
  tmpdata.foreach(getrwds);

  auto condrwds = [&](const string& key, xLuaData& d)
  {
    DWORD param = atoi(key.c_str());
    TSetDWORD& ids = sData.mapParam2Rewards[param];
    d.getIDList(ids);
  };
  xLuaData cond = params.getData("param_reward");
  cond.foreach(condrwds);

  return RewardConfig::getMe().addExtraRwd(eType, sData);
}

bool GMCommandRuler::scene_del_event_reward(Scene* pScene, const xLuaData& params)
{
  if (params.has("name") == false)
    return false;

  string name = params.getTableString("name");
  EExtraRewardType eType;
  if (name == "board_quest")
    eType = EEXTRAREWARD_WANTEDQUEST;
  else if (name == "resist_monster")
    eType = EEXTRAREWARD_DAILYMONSTER;
  else if (name == "seal")
    eType = EEXTRAREWARD_SEAL;
  else if (name == "laboratory")
    eType = EEXTRAREWARD_LABORATORY;
  else if (name == "endless_tower")
    eType = EEXTRAREWARD_ENDLESS;
  else if (name == "guild_quest")
    eType = EEXTRAREWARD_GUILD_QUEST;
  else if (name == "guild_donate")
    eType = EEXTRAREWARD_GUILD_DONATE;
  else if (name == "pve_card")
    eType = EEXTRAREWARD_PVECARD;
  else if (name == "gvg")
    eType = EEXTRAREWARD_GVG;
  else if (name == "super_gvg")
    eType = EEXTRAREWARD_SUPERGVG;
  else
  {
    XERR << "[GM-事件奖励-删除], 未指定正确类型" << name << XEND;
    return false;
  }

  return RewardConfig::getMe().delExtraRwd(eType);
}

bool GMCommandRuler::scene_add_double_event_reward(Scene* pScene, const xLuaData& params)
{
  if (params.has("name") == false)
    return false;
  string name = params.getTableString("name");
  SDoubleRewardData sData;
  EDoubleRewardType eType;
  if (name == "board_quest")
    eType = EDOUBLEREWARD_WANTEDQUEST;
  else if (name == "resist_monster")
    eType = EDOUBLEREWARD_DAILYMONSTER;
  else if (name == "seal")
    eType = EDOUBLEREWARD_SEAL;
  else if (name == "laboratory")
    eType = EDOUBLEREWARD_LABORATORY;
  else if (name == "endless_tower")
    eType = EDOUBLEREWARD_ENDLESS;
  else if (name == "pve_card")
    eType = EDOUBLEREWARD_PVECARD;
  else
  {
    XERR << "[GM-事件双倍奖励-添加], 未指定正确类型" << name << XEND;
    return false;
  }

  sData.eType = eType;
  sData.dwNeedTimes = params.getTableInt("need_times");
  sData.dwDayRewardTimes = params.getTableInt("day_limits");
  sData.dwTimes = params.getTableInt("reward_times");
  sData.dwAccLimit = params.getTableInt("acclimit");

  return RewardConfig::getMe().addDoubleRwd(eType, sData);
}

bool GMCommandRuler::scene_del_double_event_reward(Scene* pScene, const xLuaData& params)
{
  if (params.has("name") == false)
    return false;

  string name = params.getTableString("name");
  EDoubleRewardType eType;
  if (name == "board_quest")
    eType = EDOUBLEREWARD_WANTEDQUEST;
  else if (name == "resist_monster")
    eType = EDOUBLEREWARD_DAILYMONSTER;
  else if (name == "seal")
    eType = EDOUBLEREWARD_SEAL;
  else if (name == "laboratory")
    eType = EDOUBLEREWARD_LABORATORY;
  else if (name == "endless_tower")
    eType = EDOUBLEREWARD_ENDLESS;
  else if (name == "pve_card")
    eType = EDOUBLEREWARD_PVECARD;
  else
  {
    XERR << "[GM-事件双倍奖励-删除], 未指定正确类型" << name << XEND;
    return false;
  }

  return RewardConfig::getMe().delDoubleRwd(eType);
}

bool GMCommandRuler::scene_open_npcfunc(Scene* pScene, const xLuaData& params)
{
  if (params.has("npcid") == false)
    return false;

  DWORD npcid = params.getTableInt("npcid");
  SNpcCFG*pNpc = const_cast<SNpcCFG*>(NpcConfig::getMe().getNpcCFG(npcid));
  if (!pNpc)
  {
    XERR << "[GM-开启npc功能], 失败，找不到npc" << npcid << XEND;
    return false;
  }

  std::map<DWORD, string> mapNpcFunc;
  auto getreqfunc = [&](const string& str, xLuaData& data)
  {
    DWORD dwId = data.getTableInt("type");
    std::stringstream ss;
    data.toJsonString(ss);
    if (dwId != 0)
    {
      mapNpcFunc.insert(std::make_pair(dwId, ss.str()));
      XDBG << "[GM-开启npc功能]" << npcid << dwId << ss.str() << XEND;
    }
  };

  xLuaData data = params.getData("npcfunction");
  data.foreach(getreqfunc);
    
  pNpc->stNpcFunc.addNpcFunction(mapNpcFunc);
  
  XLOG << "[GM-开启npc功能], 成功，npc" << npcid <<"通知给地图玩家"<<(pScene == nullptr?"没有":"通知了") << XEND;
  return true;
  //return ActivityConfig::getMe().openNpcFunc(npcid, funcs);
}

bool GMCommandRuler::scene_close_npcfunc(Scene* pScene, const xLuaData& params)
{
  if (params.has("npcid") == false)
    return false;

  DWORD npcid = params.getTableInt("npcid");
  SNpcCFG*pNpc = const_cast<SNpcCFG*>(NpcConfig::getMe().getNpcCFG(npcid));
  if (!pNpc)
  {
    XERR << "[GM-关闭npc功能], 失败，找不到npc" << npcid << XEND;
    return false;
  }

  TSetDWORD funcs;

  auto getid = [&](const string& key, xLuaData& data)
  {
    funcs.insert(data.getInt());
  };

  xLuaData data = params.getData("npcfunction");
  data.foreach(getid);
  pNpc->stNpcFunc.delNpcFunction(funcs);
  
  XLOG << "[GM-关闭npc功能], 成功，npc" << npcid << "通知给地图玩家" << (pScene == nullptr ? "没有" : "通知了") << XEND;
  return true;
}

bool GMCommandRuler::scene_rangebgm(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;
  if (params.has("pos") == false)
    return false;

  xPos pos;
  xLuaData posd = params.getData("pos");
  pos.x = posd.getTableFloat("1");
  pos.y = posd.getTableFloat("2");
  pos.z = posd.getTableFloat("3");

  bool del = params.getTableInt("del") == 1;

  if (del)
  {
    xSceneEntrySet set;
    pScene->getEntryListInBlock(SCENE_ENTRY_ACT, pos, 3, set);
    for (auto &s : set)
    {
      SceneActEvent* pActEvent = dynamic_cast<SceneActEvent*> (s);
      if (pActEvent && pActEvent->getEventType() == SCENE_EVENT_BGM)
      {
        pActEvent->setClearState();
        XLOG << "[scene-rangebgm], 删除bgm, id:" << pActEvent->id << "位置:" << pos.x << pos.y << pos.z << XEND;
      }
    }
  }
  else
  {
    DWORD range = params.getTableInt("range");
    if (range == 0)
    {
      XERR << "[scene-rangebgm],添加bmg失败, 未填写有效范围, range = 0" << XEND;
      return false;
    }
    if (params.has("bgm") == false)
    {
      XERR << "[scene-rangebgm], 添加bgm失败, 未填写有效bgm" << XEND;
      return false;
    }
    SceneActBase* pAct = SceneActManager::getMe().createSceneAct(pScene, pos, range, 0, EACTTYPE_SCENEEVENT);
    if (pAct == nullptr)
      return false;
    SceneActEvent* pActEvent = dynamic_cast<SceneActEvent*> (pAct);
    if (pActEvent == nullptr)
    {
      SAFE_DELETE(pAct);
      return false;
    }

    pActEvent->setBgm(params.getTableString("bgm"));
    if (params.has("cleartime"))
    {
      DWORD time2del = now() + params.getTableInt("cleartime");
      pActEvent->setClearTime(time2del);
    }

    pActEvent->enterScene(pScene);

    if (params.getTableInt("wedding") == 1)
      SceneWeddingMgr::getMe().addSceneEvent(pActEvent->id);

    XLOG << "[scene-rangebgm], 成功添加bgm, 地图:" << pScene->id << "位置:" << pos.x << pos.y << pos.z << "bgm:" << params.getTableString("bgm") << "act id:" << pActEvent->id << XEND;
  }

  return true;
}

bool GMCommandRuler::scene_rangesky(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;
  if (params.has("pos") == false)
    return false;

  xPos pos;
  xLuaData posd = params.getData("pos");
  pos.x = posd.getTableFloat("1");
  pos.y = posd.getTableFloat("2");
  pos.z = posd.getTableFloat("3");

  bool del = params.getTableInt("del") == 1;

  if (del)
  {
    xSceneEntrySet set;
    pScene->getEntryListInBlock(SCENE_ENTRY_ACT, pos, 3, set);
    for (auto &s : set)
    {
      SceneActEvent* pActEvent = dynamic_cast<SceneActEvent*> (s);
      if (pActEvent && pActEvent->getEventType() == SCENE_EVENT_SKY)
      {
        pActEvent->setClearState();
        XLOG << "[scene-rangesky], 删除sky, id:" << pActEvent->id << "位置:" << pos.x << pos.y << pos.z << XEND;
      }
    }
  }
  else
  {
    DWORD range = params.getTableInt("range");
    if (range == 0)
    {
      XERR << "[scene-rangesky],添加sky失败, 未填写有效范围, range = 0" << XEND;
      return false;
    }
    if (params.has("sky") == false)
    {
      XERR << "[scene-rangesky], 添加sky失败, 未填写有效sky" << XEND;
      return false;
    }
    SceneActBase* pAct = SceneActManager::getMe().createSceneAct(pScene, pos, range, 0, EACTTYPE_SCENEEVENT);
    if (pAct == nullptr)
      return false;
    SceneActEvent* pActEvent = dynamic_cast<SceneActEvent*> (pAct);
    if (pActEvent == nullptr)
    {
      SAFE_DELETE(pAct);
      return false;
    }

    DWORD skyid = params.getTableInt("sky");
    pActEvent->setSky(skyid);
    if (params.has("cleartime"))
    {
      DWORD time2del = now() + params.getTableInt("cleartime");
      pActEvent->setClearTime(time2del);
    }

    pActEvent->enterScene(pScene);
    if (params.getTableInt("wedding") == 1)
      SceneWeddingMgr::getMe().addSceneEvent(pActEvent->id);
    XLOG << "[scene-rangesky], 成功添加sky, 地图:" << pScene->id << "位置:" << pos.x << pos.y << pos.z << "sky:" << skyid << "act id:" << pActEvent->id << XEND;
  }

  return true;
}

bool GMCommandRuler::scene_rangeweather(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;
  if (params.has("pos") == false)
    return false;

  xPos pos;
  xLuaData posd = params.getData("pos");
  pos.x = posd.getTableFloat("1");
  pos.y = posd.getTableFloat("2");
  pos.z = posd.getTableFloat("3");

  bool del = params.getTableInt("del") == 1;

  if (del)
  {
    xSceneEntrySet set;
    pScene->getEntryListInBlock(SCENE_ENTRY_ACT, pos, 3, set);
    for (auto &s : set)
    {
      SceneActEvent* pActEvent = dynamic_cast<SceneActEvent*> (s);
      if (pActEvent && pActEvent->getEventType() == SCENE_EVENT_WEATHER)
      {
        pActEvent->setClearState();
        XLOG << "[scene-rangeweather], 删除weather, id:" << pActEvent->id << "位置:" << pos.x << pos.y << pos.z << XEND;
      }
    }
  }
  else
  {
    DWORD range = params.getTableInt("range");
    if (range == 0)
    {
      XERR << "[scene-rangeweather],添加weather失败, 未填写有效范围, range = 0" << XEND;
      return false;
    }
    if (params.has("weather") == false)
    {
      XERR << "[scene-rangeweather], 添加weather失败, 未填写有效weather" << XEND;
      return false;
    }
    SceneActBase* pAct = SceneActManager::getMe().createSceneAct(pScene, pos, range, 0, EACTTYPE_SCENEEVENT);
    if (pAct == nullptr)
      return false;
    SceneActEvent* pActEvent = dynamic_cast<SceneActEvent*> (pAct);
    if (pActEvent == nullptr)
    {
      SAFE_DELETE(pAct);
      return false;
    }

    DWORD weatherid = params.getTableInt("weather");
    pActEvent->setWeather(weatherid);
    if (params.has("cleartime"))
    {
      DWORD time2del = now() + params.getTableInt("cleartime");
      pActEvent->setClearTime(time2del);
    }

    pActEvent->enterScene(pScene);
    if (params.getTableInt("wedding") == 1)
      SceneWeddingMgr::getMe().addSceneEvent(pActEvent->id);
    XLOG << "[scene-rangeweather], 成功添加weather, 地图:" << pScene->id << "位置:" << pos.x << pos.y << pos.z << "weather:" << weatherid  << "act id:" << pActEvent->id << XEND;
  }

  return true;
}

bool GMCommandRuler::scene_effect(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;

  if (params.has("effect") == false || params.has("pos") == false)
    return false;

  xPos pos;
  xLuaData posd = params.getData("pos");
  pos.x = posd.getTableFloat("1");
  pos.y = posd.getTableFloat("2");
  pos.z = posd.getTableFloat("3");

  SceneActBase* pAct = SceneActManager::getMe().createSceneAct(pScene, pos, 0, 0, EACTTYPE_EFFECT);
  if (pAct == nullptr)
    return false;

  SceneActEffect* pActEffect = dynamic_cast<SceneActEffect*> (pAct);
  if (pActEffect == nullptr)
  {
    SAFE_DELETE(pAct);
    return false;
  }

  DWORD effecttime = params.getTableInt("cleartime");
  effecttime = effecttime == 0 ? 99999999 : effecttime;

  string effectstr = params.getTableString("effect");
  pActEffect->setEffectInfo(effectstr, effecttime, 0, params.getTableInt("dir"));

  pActEffect->setClearTime(now() + effecttime);
  pActEffect->enterScene(pScene);

  if (params.getTableInt("wedding") == 1)
    SceneWeddingMgr::getMe().addSceneEvent(pActEffect->id);

  XLOG << "[GM场景-特效], 创建成功, id:" << pActEffect->id << "地图:" << pScene->name << pScene->id << XEND;
  return true;
}

bool GMCommandRuler::resetGuildRaid(Scene* pScene, const xLuaData& params)
{
  if (params.has("time") == false)
    return false;

  SceneManager::getMe().closeSceneByType(SCENE_TYPE_GUILD_RAID);
  DWORD time = params.getTableInt("time");
  XLOG << "[GM-公会副本重置], 重置时间戳:" << time << XEND;
  return GuildRaidConfig::getMe().resetRaid(time);
}

bool GMCommandRuler::recoverGuildRaid(Scene* pScene, const xLuaData& params)
{
  GuildRaidConfig::getMe().clearResetVersion();
  XLOG << "[GM-公会副本重置], 取消重置标志" << XEND;
  return true;
}


bool GMCommandRuler::activity_reward(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD dwVarType = params.getTableInt("var");
  pUser->getQuest().resetVarReward();
  if (pUser->getQuest().checkVarReward(dwVarType) == true)
  {
    XERR << "[场景玩家-活动奖励]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "var :" << dwVarType << "今日已领取过" << XEND;
    return false;
  }

  DWORD dwReward = params.getTableInt("reward");
  TVecItemInfo vecItems;
  if (RewardManager::roll(dwReward, pUser, vecItems, ESOURCE_DOG) == false)
  {
    XERR << "[场景玩家-活动奖励]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "reward :" << dwReward << "随机奖励失败" << XEND;
    return false;
  }
  /*MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr)
  {
    XERR << "[场景玩家-活动奖励]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "获取背包失败" << XEND;
    return false;
  }
  if (pMainPack->checkAddItem(vecItems) == false)
  {
    XERR << "[场景玩家-活动奖励]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "添加到背包失败,超过背包上限" << XEND;
    return false;
  }*/
  for (auto &v : vecItems)
    v.set_source(ESOURCE_DOG);

  //pMainPack->addItemFull(vecItems);
  pUser->getPackage().addItem(vecItems, EPACKMETHOD_NOCHECK);
  pUser->getQuest().addVarReward(dwVarType);
  XLOG << "[场景玩家-活动奖励]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "成功领取活动奖励 :" << "活动类型" << dwVarType;
  for (auto &v : vecItems)
    XLOG << "id :" << v.id() << "count :" << v.count();
  XLOG << XEND;
  return true;
}

bool GMCommandRuler::sendmsg(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);
  if (params.has("id") == false)
    return false;
  MsgManager::sendMsg(pUser->id, params.getTableInt("id"));
  return true;
}

bool GMCommandRuler::dropReward(xSceneEntryDynamic* entry, const xLuaData &params)
{
  //GET_USER(entry);
  if (entry == nullptr)
    return false;
  Scene* scene = entry->getScene();
  if (scene == nullptr)
    return false;

  DWORD id = params.getTableInt("id");
  if (id == 0 || RewardConfig::getMe().getRewardCFG(id) == nullptr)
  {
    XERR << "[GM-dropReward], 配置错误, id为" << id << "在Table_Reward.txt中未找到" << XEND;
    return false;
  }
  ESource esource = ESOURCE_GM;
  if (params.has("GM_ESource"))
  {
    DWORD source = params.getTableInt("GM_ESource");
    esource = static_cast<ESource>(source);
  }
  TVecItemInfo vecItemInfo;
  RewardManager::roll(id, nullptr, vecItemInfo, esource);

  if (vecItemInfo.empty())
  {
    XLOG << "[GM-dropReward], 未随机到掉落, id" << id << XEND;
    return true;
  }

  float fRange = MiscConfig::getMe().getSceneItemCFG().getRange(vecItemInfo.size());
  if (params.has("range"))
    fRange = params.getTableFloat("range");
  Cmd::AddMapItem cmd;
  for (auto &v : vecItemInfo)
  {
    DWORD extraTime = MiscConfig::getMe().getSceneItemCFG().dwDropInterval;
    xPos dest;
    if (scene->getRandPos(entry->getPos(), fRange, dest) == false)
    {
      XERR << "[GM]" << entry->name << "dropReward 获取随机位置失败" << XEND;
      return false;
    }

    SceneItem* pItem = SceneItemManager::getMe().createSceneItem(scene, v, dest);
    if (pItem == nullptr)
    {
        XERR << "[GM]" << entry->name << "dropReward SceneItem 创建失败" << XEND;
        return false;
    }
    if (entry->getEntryType() == SCENE_ENTRY_USER)
      pItem->addOwner(entry->id);
    pItem->fillMapItemData(cmd.add_items(), extraTime);
    XLOG << "[GM-DropReward], 奖励掉落, 来自:" << entry->name << entry->id << "奖励ID:" << id << "item:" << v.id() << v.count() << XEND;
  }
  PROTOBUF(cmd, send, len);
  scene->sendCmdToNine(entry->getPos(), send, len);
  return true;
}

bool GMCommandRuler::playdialog(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  if (params.has("id") == false)
    return false;
  DWORD id = params.getTableInt("id");
  if (TableManager::getMe().getDialogCFG(id) == nullptr)
  {
    XERR << "[GM-playdialog], 找不到id" << id << XEND;
    return false;
  }
  UserActionNtf cmd;
  cmd.set_type(EUSERACTIONTYPE_DIALOG);
  cmd.set_value(id);
  cmd.set_charid(pUser->id);
  PROTOBUF(cmd, send, len);

  pUser->sendCmdToMe(send, len);
  return true;
}

bool GMCommandRuler::unlockmanual(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  DWORD dwType = params.getTableInt("type");
  if (dwType <= EMANUALTYPE_MIN || dwType >= EMANUALTYPE_MAX || EManualType_IsValid(dwType) == false)
  {
    XERR << "[GM指令-冒险手册]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "解锁 type :" << dwType << "失败,type不合法" << XEND;
    return false;
  }
  EManualType eType = static_cast<EManualType>(dwType);
  DWORD dwID = params.getTableInt("id");
  bool bSuccess = false;
  switch (eType)
  {
    case EMANUALTYPE_MIN:
      break;
    case EMANUALTYPE_FASHION:
      break;
    case EMANUALTYPE_CARD:
    case EMANUALTYPE_EQUIP:
    case EMANUALTYPE_ITEM:
    case EMANUALTYPE_COLLECTION:
      pUser->getManual().onItemAdd(dwID, true, true, true, ESOURCE_MAX);
      bSuccess = true;
      break;
    case EMANUALTYPE_MOUNT:
      break;
    case EMANUALTYPE_MONSTER:
    case EMANUALTYPE_NPC:
      bSuccess = true;
      pUser->getManual().onKillMonster(dwID, true);
      break;
    case EMANUALTYPE_HOMEPAGE:
      break;
    case EMANUALTYPE_MAP:
      bSuccess = true;
      pUser->getManual().onEnterMap(dwID, true);
      break;
    case EMANUALTYPE_ACHIEVE:
      break;
    case EMANUALTYPE_SCENERY:
      bSuccess = true;
      pUser->getManual().onScenery(dwID, true);
      break;
    case EMANUALTYPE_MAX:
      break;
    default:
      break;
  }

  if (bSuccess)
    XLOG << "[GM指令-冒险手册]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "解锁 type :" << dwType << "id :" << dwID << "成功" << XEND;
  else
    XERR << "[GM指令-冒险手册]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "解锁 type :" << dwType << "id :" << dwID << "失败" << XEND;

  return true;
}

bool GMCommandRuler::pickup(xSceneEntryDynamic* entry, const xLuaData &params)
{
  GET_USER(entry);

  if (params.has("show") == false)
    return false;

  EPickupMode eMode = MiscConfig::getMe().getItemCFG().ePickupMode;
  if (eMode == EPICKUPMODE_CLIENT)
    MsgManager::sendMsg(pUser->id, 10, MsgParams("当前为客户端捡取模式"));
  else if (eMode == EPICKUPMODE_SERVER)
    MsgManager::sendMsg(pUser->id, 10, MsgParams("当前为服务端捡取模式"));
  else
  {
    XERR << "[GM指令-捡取]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "查询捡取模式失败" << XEND;
    return false;
  }

  return true;
}

bool GMCommandRuler::npcmove(xSceneEntryDynamic* entry, const xLuaData &params)
{
  if (entry == nullptr || entry->getScene() == nullptr)
    return false;
  if (params.has("uniqueid") == false || params.has("pos") == false)
    return false;

  DWORD uniqueid = params.getTableInt("uniqueid");
  if (uniqueid == 0)
    return false;
  xPos pos;
  pos.x = params.getData("pos").getTableFloat("1");
  pos.y = params.getData("pos").getTableFloat("2");
  pos.z = params.getData("pos").getTableFloat("3");

  xSceneEntrySet npcset;
  entry->getScene()->getAllEntryList(SCENE_ENTRY_NPC, npcset);
  for (auto &s : npcset)
  {
    SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
    if (npc == nullptr || npc->define.getUniqueID() != uniqueid)
      continue;
    if (npc->m_oMove.empty() == false)
      npc->m_oMove.stop();

    npc->m_ai.moveTo(pos);
    XLOG << "[GM-npcmove], 设置npc:" << npc->name << "uniqueid:" << uniqueid << "移动路径:" << pos.x << pos.y << pos.z << XEND;
  }

  return true;
}

bool GMCommandRuler::testtool(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (0 == strcmp(params.getTableString("name"), "mvp"))
  {
    if (pUser->getScene() == nullptr)
      return false;

    DWORD id = params.getTableInt("id");
    xSceneEntrySet set;
    pUser->getScene()->getAllEntryList(SCENE_ENTRY_NPC, set);
    for (auto &it : set)
    {
      SceneNpc *npc = (SceneNpc *)it;
      if (npc->getNpcID() == id && npc->getNpcType() == ENPCTYPE_MVP)
      {
        npc->testPrintMvpInfo(pUser);
      }
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "graid"))
  {
    DWORD gp = 1;
    if (params.has("id"))
      gp = params.getTableInt("id");

    DWORD index = params.getTableInt("index");
    if (index == 0)
    {
      const SGuildRaidInfo* pEntranceMap = GuildRaidConfig::getMe().getGuildEntrance(gp);
      if (pEntranceMap == nullptr)
        return false;
      CreateDMapParams param;
      param.qwCharID = pUser->id;
      param.dwRaidID = pEntranceMap->dwMapID;
      param.m_dwGuildRaidIndex = pEntranceMap->getMapIndex();
      SceneManager::getMe().createDScene(param);
    }
    else
    {
      const SGuildRaidInfo* pEntranceMap = GuildRaidConfig::getMe().getGuildRaidInfo(index);
      if (pEntranceMap == nullptr)
        return false;
      CreateDMapParams param;
      param.qwCharID = pUser->id;
      param.dwRaidID = pEntranceMap->dwMapID;
      param.m_dwGuildRaidIndex = pEntranceMap->getMapIndex();
      SceneManager::getMe().createDScene(param);
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "create_graid"))
  {
    GuildRaidConfig::getMe().createGuildRaid(true);
  }
  else if (0 == strcmp(params.getTableString("name"), "quickoffstore"))
  {
    BasePackage* pPack = pUser->getPackage().getPackage(EPACKTYPE_STORE);
    if (pPack == nullptr)
      return false;

    // testtool name=quickoffstore item=47013 num=2 lock=1
    DWORD itemid = params.getTableInt("item");
    DWORD num = params.getTableInt("num");
    DWORD lock = params.getTableInt("lock");

    const TSetItemBase& items = pPack->getItemBaseList(itemid);
    set<string> guids;
    for (auto it = items.begin(); it != items.end(); ++it)
      guids.insert((*it)->getGUID());

    for (auto guid : guids) {
      if (num-- <= 0)
        break;
      pUser->getPackage().equip(EEQUIPOPER_OFFSTORE, EEQUIPPOS_MIN, guid, false, lock);
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "cat"))
  {
    if (0 == strcmp(params.getTableString("action"), "expire"))
    {
      DWORD catid = params.getTableInt("id");
      pUser->getWeaponPet().setExpire(catid);
    }
    else if (0 == strcmp(params.getTableString("action"), "handcat"))
    {
      QWORD catid = pUser->id * 1000 + 1;
      if (params.has("id"))
      {
        catid = pUser->id * 1000 + params.getTableInt("id");
      }
      if (params.getTableInt("del"))
      {
        pUser->getWeaponPet().breakHand(catid);
      }
      else
      {
        pUser->getWeaponPet().inviteHand(catid);
      }
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "catchpet"))
  {
    Scene* pScene = pUser->getScene();
    if (pScene == nullptr)
      return false;
    xSceneEntrySet set;
    pScene->getEntryList(pUser->getPos(), 5, set);
    for (auto &s : set)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
      if (npc == nullptr)
        continue;
      if (npc->getNpcType() == ENPCTYPE_CATCHNPC)
      {
        CatchPetNpc* pcatch = dynamic_cast<CatchPetNpc*> (npc);
        if (pcatch)
          pcatch->catchMe();
        return true;
      }
    }
    DWORD npcid = params.getTableInt("npc");
    for (auto &s : set)
    {
      SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
      if (npc == nullptr)
        continue;
      if (npc->getNpcID() != npcid)
        continue;
      pUser->getUserPet().preCatchPet(npc);
      return true;
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "pet"))
  {
    if (0 == strcmp(params.getTableString("action"), "add"))
    {
      DWORD petid = params.getTableInt("id");
      EggData testdata;
      testdata.set_id(petid);
      testdata.set_name("poli-1");
      pUser->getUserPet().addPet(testdata, true);
    }
    else if (0 == strcmp(params.getTableString("action"), "egg"))
    {
      DWORD petid = params.getTableInt("id");
      pUser->getUserPet().toEgg(petid);
    }
    else if (0 == strcmp(params.getTableString("action"), "del"))
    {
      DWORD petid = params.getTableInt("id");
      pUser->getUserPet().del(petid);
    }
    else if (0 == strcmp(params.getTableString("action"), "hand"))
    {
      if (params.getTableInt("del") == 1)
        pUser->getUserPet().testhand(false);
      else
        pUser->getUserPet().testhand(true);
    }
    else if (0 == strcmp(params.getTableString("action"), "food"))
    {
      Scene* pScene = pUser->getScene();
      if (pScene == nullptr)
        return false;
      xSceneEntrySet set;
      pScene->getEntryList(pUser->getPos(), 10, set);
      DWORD npcid = params.getTableInt("npcid");
      for (auto &s : set)
      {
        SceneNpc* npc = dynamic_cast<SceneNpc*> (s);
        if (npc == nullptr)
          continue;
        if (npc->getNpcID() != npcid)
          continue;
        pUser->getUserPet().onSeeFood(npc);
        return true;
      }
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "gvg"))
  {
    if (0 == strcmp(params.getTableString("action"), "onoff"))
    {
      if (params.getTableInt("open") == 1)
      {
        bool super = params.getTableInt("super");
        DWORD supertime = params.getTableInt("supertime");
        GuildCityManager::getMe().openFireAtonce(super, supertime);
      }
      else
      {
        GuildCityManager::getMe().stopFireAtonce();
      }
    }
    else if (0 == strcmp(params.getTableString("action"), "occupy"))
    {
      QWORD guild = pUser->getGuild().id();
      if (params.has("guild"))
        guild = params.getTableInt("guild");
      DWORD flagid = params.getTableInt("city");
      GuildCityManager::getMe().updateCityInfoToGuild(flagid, guild);
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "md5"))
  {
    if (0 == strcmp(params.getTableString("action"), "show"))
    {
      const list<PhotoMd5>& listMd5 = pUser->getUserSceneData().getPhotoMd5List();
      QueryMd5ListPhotoCmd cmd;
      for (auto &l : listMd5)
        cmd.add_item()->CopyFrom(l);

      MsgManager::sendMsg(pUser->id, 4, MsgParams(cmd.ShortDebugString()));
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "range_event"))
  {
    if (0 == strcmp(params.getTableString("action"), "bgm"))
      GMCommandRuler::scene_rangebgm(pUser->getScene(), params);
    if (0 == strcmp(params.getTableString("action"), "weather"))
      GMCommandRuler::scene_rangeweather(pUser->getScene(), params);
    if (0 == strcmp(params.getTableString("action"), "sky"))
      GMCommandRuler::scene_rangesky(pUser->getScene(), params);
  }
  else if (0 == strcmp(params.getTableString("name"), "wedding"))
  {
    // 设置假数据, 仅测试使用
    if (0 == strcmp(params.getTableString("action"), "test_wedding_begin"))
    {
      QWORD charid1 = pUser->id;
      QWORD charid2 = 0;
      for (auto &m : pUser->getTeam().getTeamMemberList())
      {
        if (m.first != charid1)
        {
          charid2 = m.first;
          break;
        }
      }
      if (charid2 == 0)
        return true;

      StartWeddingSCmd cmd;
      WeddingInfo* info = cmd.mutable_weddinginfo();

      info->set_id(randBetween(1,9999999));
      info->set_status(EWeddingStatus_Reserve);
      info->set_charid1(charid1);
      info->set_charid2(charid2);
      info->set_starttime(now());
      info->set_endtime(now() + 3600);
      info->set_zoneid(thisServer->getZoneID());

      SceneWeddingMgr::getMe().startWedding(cmd);
    }
    else if (0 == strcmp(params.getTableString("action"), "go_to_wedding"))
    {
      QWORD wedid = SceneWeddingMgr::getMe().getWeddingInfo().id();
      DWORD raid = params.getTableInt("raid");
      if (wedid)
      {
        CreateDMapParams param;
        param.qwCharID = pUser->id;
        param.dwRaidID = raid;
        param.m_qwRoomId = wedid;
        SceneManager::getMe().createDScene(param);
      }
    }
  }
  else if ( 0 == strcmp(params.getTableString("name"), "pvecard"))
  {
    if (0 == strcmp(params.getTableString("action"), "create"))
    {
      PveCardConfig::getMe().randSystemCard(true);
    }
    else if (0 == strcmp(params.getTableString("action"), "shuffle"))
    {
      DWORD raidid = params.getTableInt("raid");
      DWORD index = params.getTableInt("index");
      TVecDWORD vecResult;
      PveCardConfig::getMe().shuffleCard(raidid, index, vecResult);
      XDBG << "[GM-Pve卡牌], 测试打乱, 排序如下:";
      for (auto &v : vecResult)
        XDBG << v;
      XDBG << XEND;
    }
    else if (0 == strcmp(params.getTableString("action"), "msg"))
    {
      DWORD id = params.getTableInt("id");
      switch (id)
      {
        case 1:
          {
            SelectPveCardCmd cmd;
            cmd.set_index(1);
            PROTOBUF(cmd, send, len);
            thisServer->forwardCmdToSceneUser(pUser->id, send, len);
          }
          break;
        case 2:
          {
            BeginFirePveCardCmd cmd;
            PROTOBUF(cmd, send, len);
            thisServer->forwardCmdToSceneUser(pUser->id, send, len);
          }
          break;
        default:
          break;
      }
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "pvp"))
  {
    if (0 == strcmp(params.getTableString("action"), "leave"))
    {
      EPvpType type = static_cast<EPvpType>(params.getTableInt("t"));
      QWORD roomId = params.getTableQWORD("id");
      LeaveRoomCCmd cmd;
      cmd.set_type(type);
      cmd.set_roomid(roomId);
      PROTOBUF(cmd, send, len);
      thisServer->forwardCmdToSessionUser(pUser->id, send, len);
    }
  }
  else if ( 0 == strcmp(params.getTableString("name"), "sugvg"))
  {
    if (0 == strcmp(params.getTableString("action"), "gjoin"))
    {
      if (params.has("guildids"))
      {
        TSetDWORD ids;
        xLuaData iddata = params.getData("guildids");
        iddata.getIDList(ids);
        JoinSuperGvgGuildSCmd cmd;
        cmd.set_supergvgtime(now());
        for (auto &s : ids)
        {
          cmd.set_guildid(s);
          PROTOBUF(cmd, send, len);
          thisServer->sendCmdToSession(send, len);
        }
      }
      else
      {
        QWORD guild = pUser->getGuild().id();
        if (params.has("guildid"))
          guild = params.getTableQWORD("guildid");
        JoinSuperGvgGuildSCmd cmd;
        cmd.set_guildid(guild);
        cmd.set_supergvgtime(now());
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToSession(send, len);
      }
    }
    else if (0 == strcmp(params.getTableString("action"), "fire"))
    {
      SuperGvgScene* pGScene = dynamic_cast<SuperGvgScene*>(pUser->getScene());
      if (pGScene == nullptr)
        return false;
      pGScene->openFireAtonce();
    }
    else if (0 == strcmp(params.getTableString("action"), "gm_begin"))
    {
      if (params.has("join_time") == false || params.has("guildids") == false)
        return false;
      DWORD hour = params.getData("join_time").getTableInt("1");
      DWORD min = params.getData("join_time").getTableInt("2");
      DWORD daystart = xTime::getDayStart(now());
      DWORD jointime = daystart + hour * 3600 + min * 60;
      if (GuildCityManager::getMe().setTestSuperGvgBeginTime(jointime) == false)
        return false;
      TSetDWORD ids;
      xLuaData iddata = params.getData("guildids");
      iddata.getIDList(ids);
      JoinSuperGvgGuildSCmd cmd;
      cmd.set_supergvgtime(jointime);;
      XLOG << "[GM-决战报名成功], 开始匹配时间:" << hour << min << jointime << "报名公会:";
      for (auto &s : ids)
      {
        cmd.set_guildid(s);
        PROTOBUF(cmd, send, len);
        thisServer->sendCmdToSession(send, len);
        XLOG << s;
      }
      XLOG << XEND;
    }
  }
  else if ( 0 == strcmp(params.getTableString("name"), "match"))
  {
    if (0 == strcmp(params.getTableString("action"), "teampws"))
    {
      SceneGMTestMatchSCmd cmd;
      cmd.set_etype(EMATCHGM_JOINTEAMPWS);
      if (params.has("frequency"))
        cmd.set_frequency(params.getTableInt("frequency"));
      cmd.set_interval(params.getTableInt("interval"));
      cmd.set_lasttime(params.getTableInt("time"));
      if (params.has("score"))
      {
        DWORD min = params.getData("score").getTableInt("1");
        DWORD max = params.getData("score").getTableInt("2");
        cmd.add_params(min);
        cmd.add_params(max);
      }
      PROTOBUF(cmd, send, len);
      thisServer->sendCmdToSession(send, len);
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "dead_boss"))
  {
    if (0 == strcmp(params.getTableString("action"), "invite"))
    {
      InviteSummonBossFubenCmd cmd;
      PROTOBUF(cmd, send, len);
      thisServer->forwardCmdToSceneUser(pUser->id, send, len);
    }
    if (0 == strcmp(params.getTableString("action"), "replay"))
    {
      ReplySummonBossFubenCmd cmd;
      cmd.set_charid(pUser->id);
      cmd.set_agree(params.getTableInt("reject") == 0);
      PROTOBUF(cmd, send, len);
      thisServer->forwardCmdToSceneUser(pUser->id, send, len);
    }
  }
  else if (0 == strcmp(params.getTableString("name"), "cheat_tag"))
  {
    DWORD interval = params.getData("interval").getInt();
    DWORD frame = params.getData("frame").getInt();

    CheatTagUserCmd cmd;
    cmd.set_interval(interval);
    cmd.set_frame(frame);
    XLOG << "[反脚本-测试] charid:" << pUser->id << " interval:" << interval << " frame:" << frame << XEND;
    pUser->getCheatTag().recCheatTag(cmd);
  }
  return true;
}

bool GMCommandRuler::equip(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr)
  {
    XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "未发现装备背包" << XEND;
    return false;
  }

  DWORD dwPos = params.getTableInt("pos");
  if (dwPos <= EEQUIPPOS_MIN || dwPos >= EEQUIPPOS_MAX || EEquipPos_IsValid(dwPos) == false)
  {
    XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "不合法" << XEND;
    return false;
  }

  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getEquip(static_cast<EEquipPos>(dwPos)));
  if (pEquip == nullptr)
  {
    XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "未发现装备" << XEND;
    return false;
  }

  if (params.has("refine") == true)
  {
    const SItemCFG* pCFG = pEquip->getCFG();
    if (pCFG == nullptr || pCFG->isForbid(EFORBID_REFINE) == true)
    {
      MsgManager::sendMsg(pUser->id, 1358);
      XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "精炼等级变更失败,该装备无法精炼" << XEND;
      return false;
    }
    DWORD dwLv = params.getTableInt("refinelv");
    if (dwLv <= pEquip->getRefineLv())
    {
      MsgManager::sendMsg(pUser->id, 1358);
      XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "pos :" << dwPos << "精炼等级变更为" << dwLv << "级失败,该装备精炼等级" << pEquip->getRefineLv() << "高于设置等级" << dwLv << XEND;
      return false;
    }
    if (dwLv > pCFG->dwMaxRefineLv)
    {
      MsgManager::sendMsg(pUser->id, 1358);
      XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "pos :" << dwPos << "精炼等级变更为" << dwLv << "级失败,该装备精炼等级" << pEquip->getRefineLv() << "高于设置等级" << dwLv << XEND;
      return false;
    }

    pEquip->setRefineLv(dwLv);
    pEquipPack->setUpdateIDs(pEquip->getGUID());
    XLOG << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "精炼等级变更为" << dwLv << "级" << XEND;
  }

  if (params.has("upgrade") == true)
  {
    DWORD dwLv = params.getTableInt("upgradelv");
    pEquip->setLv(dwLv);
    pEquipPack->setUpdateIDs(pEquip->getGUID());
  }

  if (params.has("enchant") == true)
  {
    EnchantData& rEnchant = pEquip->getEnchantData();
    DWORD dwOpt = params.getTableInt("opt");
    if (dwOpt == 1)
    {
      if (params.has("attr") == true && params.has("value") == true)
      {
        DWORD dwAttr = params.getTableInt("attr");
        float fValue = params.getTableFloat("value");
        EnchantAttr* pAttr = rEnchant.add_attrs();
        if (pAttr == nullptr)
        {
          XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "创建attr protobuf失败" << XEND;
          return false;
        }
        const RoleData* pData = RoleDataConfig::getMe().getRoleData(dwAttr);
        if (pData == nullptr)
        {
          XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "失败 attr :" << dwAttr << "不合法" << XEND;
          return false;
        }
        pAttr->set_type(static_cast<EAttrType>(dwAttr));
        pAttr->set_value(pData->bPercent ? fValue * FLOAT_TO_DWORD : fValue);
        pEquipPack->setUpdateIDs(pEquip->getGUID());
      }

      if (params.has("id") == true && params.has("buffid") == true)
      {
        DWORD dwID = params.getTableInt("id");
        DWORD dwBuffID = params.getTableInt("buffid");

        EEnchantType enchantType = EENCHANTTYPE_MIN;
        for (auto ectype = EENCHANTTYPE_MIN + 1; ectype < EENCHANTTYPE_MAX; ++ectype) {
          const SEnchantCFG* pCfg = ItemConfig::getMe().getEnchantCFG(static_cast<EEnchantType>(ectype));
          if (pCfg == nullptr)
            continue;
          if (pCfg->getEnchantAttr(dwID) == nullptr)
            continue;
          enchantType = static_cast<EEnchantType>(ectype);
          break;
        }
        if (enchantType == EENCHANTTYPE_MIN) {
          XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "失败 enchantid :" << dwID << "不合法" << XEND;
          return false;
        }

        if (BufferManager::getMe().getBuffById(dwBuffID) == nullptr)
        {
          XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "失败 buffid :" << dwBuffID << "不合法" << XEND;
          return false;
        }
        EnchantExtra* pExtra = rEnchant.add_extras();
        if (pExtra == nullptr)
        {
          XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "创建extra protobuf失败" << XEND;
          return false;
        }
        pExtra->set_configid(dwID);
        pExtra->set_buffid(dwBuffID);
        rEnchant.set_type(enchantType);
        pEquipPack->setUpdateIDs(pEquip->getGUID());
      }
    }
    else if (dwOpt == 2)
    {
      if (params.has("attr") == true)
      {
        rEnchant.clear_attrs();
        pEquipPack->setUpdateIDs(pEquip->getGUID());
      }
      if (params.has("extra") == true)
      {
        rEnchant.clear_extras();
        pEquipPack->setUpdateIDs(pEquip->getGUID());
      }
    }
    else
    {
      XERR << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "opt :" << dwOpt << "未知命令" << XEND;
    }
  }

  pUser->setCollectMark(ECOLLECTTYPE_EQUIP);
  XLOG << "[GM指令-装备]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "pos :" << dwPos << "成功" << XEND;
  return true;
}

bool GMCommandRuler::resetShopSkill(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (pUser->getFighter() == nullptr)
    return false;
  return pUser->getFighter()->getSkill().resetShopSkill();
}

bool GMCommandRuler::randzeny(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  const xLuaData& zeny = params.getData("zeny");
  DWORD dwMin = zeny.getTableInt("1");
  DWORD dwMax = zeny.getTableInt("2");

  DWORD dwRand = randBetween(dwMin, dwMax);

  ItemInfo oItem;
  oItem.set_id(100);
  oItem.set_count(dwRand);
  oItem.set_source(ESOURCE_GM);
  /*BasePackage* pMainPack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pMainPack == nullptr)
  {
    XERR << "[GM指令-随机zeny]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "随机zeny失败, 未获取到" << EPACKTYPE_MAIN << XEND;
    return false;
  }*/

  //pMainPack->addItemFull(oItem, params.getTableInt("show") == 1);
  pUser->getPackage().addItem(oItem, EPACKMETHOD_NOCHECK);
  XLOG << "[GM指令-随机zeny]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "随机zeny成功,获得了" << dwRand << "个zeny" << XEND;
  return true;
}

bool GMCommandRuler::loveletter(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.getTableInt("GM_Target") != 1)
  {
    XERR << "[GM指令-发送情书]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "发送情书失败,没有目标" << XEND;
    return false;
  }

  QWORD qwTargetID = params.getTableQWORD("id1");
  SceneUser* pTarget = SceneUserManager::getMe().getUserByID(qwTargetID);
  if (pTarget == nullptr)
  {
    XERR << "[GM指令-发送情书]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "对 charid :" << qwTargetID << "发送情书失败,目标不在线" << XEND;
    return false;
  }
  
  DWORD dwType = params.getTableInt("type1");
  DWORD dwID = params.getTableInt("id");
  string bg = params.getTableString("bg");
  string content = params.getTableString("content");
  dwID = dwID == 0 ? TableManager::getMe().randLoveLetter(dwType) : 0;
  const SLoveLetter* pCFG = TableManager::getMe().getLoveLetterCFG(dwID);
  if (pCFG == nullptr)
  {
    XERR << "[GM指令-发送情书]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "对 charid :" << qwTargetID << "发送" << dwID << "情书失败,未在Table_LoveLetter.txt表找到" << XEND;
    return false;
  }

  LoveLetterNtf cmd;
  cmd.set_name(pUser->name);
  //cmd.set_content(pCFG->getContent());
  cmd.set_type(dwType);
  cmd.set_bg(bg);
  cmd.set_configid(dwID);
  if(ELETTERTYPE_CONSTELLATION == static_cast<ELetterType>(dwType) || ELETTERTYPE_CHRISTMAS == static_cast<ELetterType>(dwType))
  {
    DWORD letterID = GuidManager::getMe().getNextLetterID();
    cmd.set_letterid(letterID);
    pTarget->addLoveLetter(letterID, pUser->name, bg, dwID, content);
  }
  //if(ELETTERTYPE_CHRISTMAS == static_cast<ELetterType>(dwType))
  //{
  //  DWORD itemid = params.getTableInt("item");
  //  ItemInfo stInfo;
  //  stInfo.set_id(itemid);
  //  stInfo.set_count(1);
  //  stInfo.set_source(ESOURCE_REWARD);
  //  pUser->getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);
  //  pTarget->getPackage().addItem(stInfo, EPACKMETHOD_AVAILABLE);
  //}
  PROTOBUF(cmd, send, len);
  pTarget->sendCmdToMe(send, len);
  XLOG << "[GM指令-发送情书]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "对 charid :" << qwTargetID << "发送" << dwID <<"type1" << dwType << "背景" << bg << "成功" << XEND;
  return true;
}

bool GMCommandRuler::chat(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD dwType = params.getTableInt("type");
  if (dwType == 1)
  {
    DWORD dwStartTime = params.getTableInt("start");
    DWORD dwEndTime = params.getTableInt("end");

    QueryChatRecordCmd cmd;
    cmd.set_charid(pUser->id);
    cmd.set_targetid(params.getTableQWORD("targetid"));
    cmd.set_start(dwStartTime);
    cmd.set_end(dwEndTime);
    cmd.set_scenename(thisServer->getServerName());
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToData(send, len);
    XLOG << "[GM指令-聊天]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "成功查询" << cmd.ShortDebugString() << XEND;
  }
  else if (dwType == 2)
  {
    pUser->getVar().setVarValue(EVARTYPE_CHAT, 0);
    pUser->getUserChat().resetChat();
    XLOG << "[GM指令-聊天]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "设置存储成功" << XEND;
  }

  return true;
}

bool GMCommandRuler::kickpvpuser(xSceneEntryDynamic* entry, const xLuaData& params)
{
  QWORD userid = params.getTableQWORD("charid");
  DWORD zoneid = params.getTableInt("zoneid");

  KickUserFromPvpMatchSCmd cmd;
  cmd.set_charid(userid);
  cmd.set_zoneid(zoneid);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::resetpvp(xSceneEntryDynamic* entry, const xLuaData& params)
{
  ResetPvpMatchSCmd cmd;
  PROTOBUF(cmd, send, len);

  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::switchpvp(xSceneEntryDynamic* entry, const xLuaData& params)
{
  DWORD etype = params.getTableInt("etype");
  if (etype < EPVPTYPE_LLH || etype > EPVPTYPE_HLJS)
    return false;
  bool on = params.getTableInt("open") == 1;
  SwitchPvpMathcSCmd cmd;
  cmd.set_open(on);
  cmd.set_etype(static_cast<EPvpType>(etype));

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::setcardslot(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD itemId = params.getTableInt("itemid");
  if (itemId == 0)
    return false;

  const SItemCFG* pCFG = ItemManager::getMe().getItemCFG(itemId);
  if (pCFG == nullptr)
    return false;
 
  pUser->getPackage().setCardSlotForGM(itemId);

  XLOG << "[GM指令-设置装备洞数]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "itemId" << itemId << XEND;
  return true;
}


bool GMCommandRuler::firework(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (!pUser->getScene()) return false;

  //action 
  if (params.has("id") && params.has("style"))
  {
    xLuaData actionParams;
    actionParams.setData("style", params.getTableInt("style"), true);
    actionParams.setData("id", params.getTableInt("id"), true);
    playaction(entry, actionParams);
  }
  
  //effect  
  if (params.has("effect") && params.has("range"))
  {
    DWORD range = params.getTableInt("range");    
    DWORD dir = pUser->getUserSceneData().getDir();
    xPos newPos;
    pUser->getScene()->getPosByDir(pUser->getPos(), range, dir,newPos);
    xLuaData effectParams;
    effectParams.setData("effect", params.getTableString("effect"));
    xLuaData& pos = effectParams.getMutableData("pos");
    pos.setData("1", newPos.x, true);
    pos.setData("2", newPos.y, true);
    pos.setData("3", newPos.z, true);
    effect(entry, effectParams);
  }  
  return true;
}

bool GMCommandRuler::optguildraid(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  // format: optguildraid npcid=x opt=x level=x
  DWORD npc = params.getTableInt("npcid");
  DWORD opt = params.getTableInt("opt");
  DWORD level = params.getTableInt("level");

  switch (opt) {
  case EGUILDGATEOPT_UNLOCK:
    pUser->unlockGuildRaidGate(npc, level);
    break;
  case EGUILDGATEOPT_OPEN:
    pUser->openGuildRaidGate(npc);
    break;
  case EGUILDGATEOPT_ENTER:
    pUser->enterGuildRaid(npc);
    break;
  default:
    return false;
  }

  return true;
}

bool GMCommandRuler::oneclickrefine(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (BaseConfig::getMe().getBranch() == BRANCH_PUBLISH)
    return true;

  // format: oneclickrefine pos=x
  GET_USER(entry);

  EquipPackage* pEquipPack = dynamic_cast<EquipPackage*>(pUser->getPackage().getPackage(EPACKTYPE_EQUIP));
  if (pEquipPack == nullptr) {
    XERR << "[GM指令-一键精炼]" << pUser->accid << pUser->id << pUser->name << "未找到装备背包" << XEND;
    return false;
  }

  DWORD pos = params.getTableInt("pos");
  if (pos <= EEQUIPPOS_MIN || pos >= EEQUIPPOS_MAX || EEquipPos_IsValid(pos) == false) {
    XERR << "[GM指令-一键精炼]" << pUser->accid << pUser->id << pUser->name << "pos:" << pos << "非法" << XEND;
    return false;
  }

  ItemEquip* pEquip = dynamic_cast<ItemEquip*>(pEquipPack->getEquip(static_cast<EEquipPos>(pos)));
  if (pEquip == nullptr) {
    XERR << "[GM指令-一键精炼]" << pUser->accid << pUser->id << pUser->name << "pos:" << pos << "未找到装备" << XEND;
    return false;
  }

  const SItemCFG* pCfg = pEquip->getCFG();
  if (pCfg == nullptr) {
    XERR << "[GM指令-一键精炼]" << pUser->accid << pUser->id << pUser->name << "未找到ItemCFG" << XEND;
    return false;
  }
  if (pCfg->dwMaxRefineLv <= 0) {
    XERR << "[GM指令-一键精炼]" << pUser->accid << pUser->id << pUser->name << "最大可精炼等级为0" << XEND;
    return false;
  }

  MainPackage* pMainPack = dynamic_cast<MainPackage*>(pUser->getPackage().getPackage(EPACKTYPE_MAIN));
  if (pMainPack == nullptr) {
    XERR << "[GM指令-一键精炼]" << pUser->accid << pUser->id << pUser->name << "未找到装备背包" << XEND;
    return false;
  }

  do {
    if (pEquip->isDamaged()) {
      ItemBase* pStuff = nullptr;
      auto func = [&](ItemBase* pBase) {
        if (pBase == nullptr || pStuff)
          return;

        ItemEquip* pStuffEquip = dynamic_cast<ItemEquip*>(pBase);
        if (pStuffEquip == nullptr)
          return;

        if (pStuffEquip->getCFG() && pCfg->isRepairId(pStuffEquip->getCFG()->dwVID) && pStuffEquip->getRefineLv() <= MiscConfig::getMe().getRefineActionCFG().dwRepairMaxLv)
          pStuff = pBase;
      };
      pMainPack->foreach(func);

      if (pStuff) {
        if (pUser->getPackage().repair(pEquip->getGUID(), pStuff->getGUID()) == false)
          break;
      } else
        break;
    }

    const SRefineCFG* pRefineCfg = ItemManager::getMe().getRefineCFG(pEquip->getType(), pEquip->getQuality(), pEquip->getRefineLv() + 1);
    if (pRefineCfg == nullptr) {
      XERR << "[GM指令-一键精炼]" << pUser->accid << pUser->id << pUser->name << "未找到RefineCFG" << XEND;
      return false;
    }
    if (pRefineCfg->sComposeRate.vecCompose.size() <= 0) {
      XERR << "[GM指令-一键精炼]" << pUser->accid << pUser->id << pUser->name << "未找到composeid" << XEND;
      return false;
    }

    if (pCfg->dwMaxRefineLv <= pEquip->getRefineLv())
      break;

    //if (pUser->getPackage().refine(pEquip->getGUID(), pRefineCfg->sComposeRate.vecCompose.front(), 0, false, true) == false)
     // break;
  } while (true);

  return true;
}

bool GMCommandRuler::astrolabe(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  bool bResult = false;

  if (params.has("act")) {
    if (params.getTableInt("act") == 1) {
      // 激活星位
      // format: astrolabe act=1 id=x starid=x force=0
      if (params.getTableInt("force") == 0) {
        bResult = pUser->getAstrolabes().activateStar(params.getTableInt("id"), params.getTableInt("starid"));
        if (bResult)
          XLOG << "[GM指令-星盘-激活星位]" << pUser->accid << pUser->id << pUser->name
               << "星盘id:" << params.getTableInt("id") << "星位id:" << params.getTableInt("starid") << "激活成功" << XEND;
      } else if (params.getTableInt("force") == 1) {
        bResult = pUser->getAstrolabes().activateStarForce(params.getTableInt("id"), params.getTableInt("starid"));
        if (bResult)
          XLOG << "[GM指令-星盘-强制激活星位]" << pUser->accid << pUser->id << pUser->name
               << "星盘id:" << params.getTableInt("id") << "星位id:" << params.getTableInt("starid") << "激活成功" << XEND;
      }
    } else if (params.getTableInt("act") == 2) {
      // 激活星盘
      // format: astrolabe act=2 id=x force=0
      const SAstrolabeCFG* pCfg = AstrolabeConfig::getMe().getAstrolabe(params.getTableInt("id"));
      if (pCfg == nullptr) {
        bResult = false;
      } else {
        for (auto &v : pCfg->mapStar) {
          if (params.getTableInt("force") == 0 && pUser->getAstrolabes().activateStar(pCfg->dwId, v.second.dwId))
          {
            XLOG << "[GM指令-星盘-激活星盘-激活星位]" << pUser->accid << pUser->id << pUser->name
                 << "星盘id:" << pCfg->dwId << "星位id:" << v.second.dwId << "激活成功" << XEND;
            bResult = true;
          }
          else if (params.getTableInt("force") == 1 && pUser->getAstrolabes().activateStarForce(pCfg->dwId, v.second.dwId))
          {
            XLOG << "[GM指令-星盘-激活星盘-强制激活星位]" << pUser->accid << pUser->id << pUser->name
                 << "星盘id:" << pCfg->dwId << "星位id:" << v.second.dwId << "激活成功" << XEND;
            bResult = true;
          }
        }
      }
    } else if (params.getTableInt("act") == 3) {
      // 激活所有星盘
      // format: astrolabe act=3 force=0
      const TMapAstrolabeCFG& rCfg = AstrolabeConfig::getMe().getAllAstrolable();
      for (auto &r : rCfg) {
        for (auto &v : r.second.mapStar) {
          if (params.getTableInt("force") == 0 && pUser->getAstrolabes().activateStar(r.second.dwId, v.second.dwId))
          {
            XLOG << "[GM指令-星盘-激活所有星盘-激活星位]" << pUser->accid << pUser->id << pUser->name
                 << "星盘id:" << r.second.dwId << "星位id:" << v.second.dwId << "激活成功" << XEND;
            bResult = true;
          }
          else if (params.getTableInt("force") == 1 && pUser->getAstrolabes().activateStarForce(r.second.dwId, v.second.dwId))
          {
            XLOG << "[GM指令-星盘-激活所有星盘-强制激活星位]" << pUser->accid << pUser->id << pUser->name
                 << "星盘id:" << r.second.dwId << "星位id:" << v.second.dwId << "激活成功" << XEND;
            bResult = true;
          }
        }
      }
    }
  } else if (params.has("reset")) {
    // 重置星盘
    // format: astrolabe reset=1 type=1
    // DWORD type = params.getTableInt("type");
    // if (type <= EASTROLABETYPE_MIN || type >= EASTROLABETYPE_MAX || !EAstrolabeType_IsValid(type))
    //   return false;
    // pUser->getAstrolabes().resetForce(static_cast<EAstrolabeType>(type));
    pUser->getAstrolabes().resetAll(params.getTableInt("force") == 1, false);
    bResult = true;
    XLOG << "[GM指令-星盘-重置全盘]" << pUser->accid << pUser->id << pUser->name << "重置成功" << XEND;
  } else if (params.has("del")) {
    // 删除星位
    // format: astrolabe del=1 id=x starid=x
    bResult = pUser->getAstrolabes().delStarForce(params.getTableInt("id"), params.getTableInt("starid"));
    if (bResult)
      XLOG << "[GM指令-星盘-删除星位]" << pUser->accid << pUser->id << pUser->name
           << "星盘id:" << params.getTableInt("id") << "星位id:" << params.getTableInt("starid") << "删除成功" << XEND;
  }

  return bResult;
}

bool GMCommandRuler::resetaugury(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (!pUser)
    return false;
  DWORD opt = 0;
  if (params.has("extra"))
  {
    opt++;
    pUser->getVar().setVarValue(EVARTYPE_NEWAUGURY_EXTRACOUNT, 0);
  }
  if (params.has("default"))
  {
    opt++;
    pUser->getVar().setVarValue(EVARTYPE_NEWAUGURY_REWARD, 0);
  }

  if (opt)
  {
    pUser->getVar().setVarValue(EVARTYPE_NEWAUGURY_REWARD, 0);
    pUser->getVar().setVarValue(EVARTYPE_NEWAUGURY_EXTRACOUNT, 0);
  }
  XLOG << "[GM-占卜] 重置占卜次数" << "charid" << pUser->id << "opt" << opt << XEND;
  return true;
}

bool GMCommandRuler::showmini(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (!pUser->getScene()) return false;

  xSceneEntrySet set;
  pUser->getScene()->getAllEntryList(SCENE_ENTRY_NPC, set);
  for (auto &it : set)
  {
    SceneNpc *npc = (SceneNpc *)it;
    if (npc->m_eBossType == EBOSSTYPE_MINI && npc->isAlive())
    {
      Cmd::BossPosUserCmd message;
      ScenePos *p = message.mutable_pos();
      p->set_x(npc->getPos().getX());
      p->set_y(npc->getPos().getY());
      p->set_z(npc->getPos().getZ());
      PROTOBUF(message, send, len);
      pUser->sendCmdToMe(send, len);
      pUser->goTo(npc->getPos());
      break;
    }
  }
  return true;
}

bool GMCommandRuler::setvar(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD dwVar = params.getTableInt("var");
  if (dwVar <= EVARTYPE_MIN || dwVar >= EVARTYPE_MAX || EVarType_IsValid(dwVar) == false)
  {
    XERR << "[GM指令-设置变量]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "设置var :" << dwVar << "失败, var不合法" << XEND;
    return false;
  }

  DWORD dwValue = params.getTableInt("value");
  pUser->getVar().setVarValue(static_cast<EVarType>(dwVar), dwValue);
  return true;
}

bool GMCommandRuler::charge(xSceneEntryDynamic* entry, const xLuaData& params)
{
//#ifndef _DEBUG
//  return false;
//#endif

  GET_USER(entry);
  pUser->addCharge(params.getTableInt("num"));
  XLOG << "[GM指令-增加充值]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行成功" << XEND;
  return true;
}

bool GMCommandRuler::photo(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD type = params.getTableInt("type");
  if (type <= EPHOTOOPTTYPE_MIN || type >= EPHOTOOPTTYPE_MAX || !EPhotoOptType_IsValid(type))
    return false;

  PhotoOptCmd cmd;
  cmd.set_opttype(static_cast<EPhotoOptType>(type));
  cmd.set_index(params.getTableInt("index"));
  cmd.set_anglez(params.getTableInt("angle"));
  cmd.set_mapid(params.getTableInt("mapid"));

  PROTOBUF(cmd, send, len);
  pUser->doSceneUserPhotoCmd((const Cmd::UserCmd*)send, len);

  return true;
}

bool GMCommandRuler::delspeffect(xSceneEntryDynamic* entry, const xLuaData& params)
{
  if (entry == nullptr)
    return false;

  // format: speffectid={x,x,x}
  const xLuaData& ids = params.getData("speffectid");
  for (auto it = ids.m_table.begin(); it != ids.m_table.end(); ++it)
  {
    entry->getSpEffect().del(it->second.getInt(), 0);
  }

  return true;
}

bool GMCommandRuler::achieve(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD dwID = params.getTableInt("id");
  if(0 == dwID) return false;

  return pUser->getAchieve().finish(dwID);
}

bool GMCommandRuler::pet(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("baselvup") == true)
  {
    DWORD dwLv = params.getTableQWORD("baselvup");
    TListPet& listPet = pUser->getUserPet().getPetList();
    if (listPet.empty() == true)
    {
      MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_PET_EXP_ERR);
      return false;
    }
    for (auto &l : listPet)
    {
      if (!l.bLive)
      {
        MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_PET_EXP_ERR);
        return false;
      }

      QWORD qwTotalExp = 0;
      for (DWORD d = 0; d < dwLv; ++d)
      {
        const SPetBaseLvCFG* pCFG = PetConfig::getMe().getPetBaseLvCFG(l.dwLv + d + 1);
        if (pCFG != nullptr)
          qwTotalExp += pCFG->qwNewCurExp;
      }
      l.addBaseExp(qwTotalExp);
    }
  }

  if (params.has("friendlvup") == true)
  {
    DWORD dwLv = params.getTableQWORD("friendlvup");
    TListPet& listPet = pUser->getUserPet().getPetList();
    for (auto &l : listPet)
    {
      if (!l.bLive)
        continue;

      for (DWORD d = 0; d < dwLv; ++d)
      {
        const SPetFriendLvCFG* pCFG = PetConfig::getMe().getFriendLvCFG(l.dwID, l.dwFriendLv + d);
        const SPetFriendLvCFG* pNextCFG = PetConfig::getMe().getFriendLvCFG(l.dwID, l.dwFriendLv + d + 1);
        if (pCFG != nullptr && pNextCFG != nullptr && pNextCFG->qwExp >= pCFG->qwExp)
          l.addFriendExp(pNextCFG->qwExp - pCFG->qwExp);
      }
    }
  }

  if (params.has("exp") == true)
  {
    QWORD qwExp = params.getTableQWORD("exp");
    TListPet& listPet = pUser->getUserPet().getPetList();
    if (listPet.empty() == true)
    {
      MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_PET_EXP_ERR);
      return false;
    }
    for (auto &l : listPet)
    {
      if (!l.bLive)
      {
        MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_PET_EXP_ERR);
        return false;
      }
      l.addBaseExp(qwExp);
      MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_PET_EXP_GET, MsgParams{l.strName, static_cast<DWORD>(qwExp)});
    }
  }

  if (params.has("friendexp") == true)
  {
    QWORD qwFriendExp = params.getTableQWORD("friendexp");
    pUser->getUserPet().addFriendExp(qwFriendExp);
  }

  if (params.has("rewardexp") == true)
  {
    QWORD qwRewardExp = params.getTableQWORD("rewardexp");
    pUser->getUserPet().addRewardExp(qwRewardExp);
  }

  if (params.has("body") == true)
  {
    const xLuaData& body = params.getData("body");
    if (pUser->getUserPet().unlockExtraBody(body.getTableInt("petid"), body.getTableInt("id")) == false)
      return false;
  }

  if (params.has("unlockmanual") == true)
    pUser->getPetWork().unlockManual(false);

  if (params.has("workfrequency") == true)
    pUser->getPetWork().testFrequency();

  return true;
}

bool GMCommandRuler::cookerlvup(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  
  bool gm = false;
  if (params.has("gm"))
    gm = params.getTableInt("gm");
  if (gm)
  {
    const SCookerLevel*pCfg = TableManager::getMe().getCookerLevelCFG(pUser->getSceneFood().getCookerLv() + 1);
    if (pCfg == nullptr)
      return false;
    DWORD exp = pCfg->getNeedExp();
    pUser->getSceneFood().addCookerExp(exp);
  }

  return pUser->getSceneFood().cookerLvUp();
}
bool GMCommandRuler::tasterlvup(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  bool gm = false;
  if (params.has("gm"))
    gm = params.getTableInt("gm");
  if (gm)
  {
    const STasterLevel*pCfg = TableManager::getMe().getTasterLevelCFG(pUser->getSceneFood().getTasterLv() + 1);
    if (pCfg == nullptr)
      return false;
    DWORD exp = pCfg->getNeedExp();
    pUser->getSceneFood().addTasterExp(exp);
  }

  return pUser->getSceneFood().tasterLvUp();
}

bool GMCommandRuler::addcookerexp(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
 
  DWORD num = params.getTableInt("num");
  pUser->getSceneFood().addCookerExp(num);
  return true;
}
bool GMCommandRuler::addtasterexp(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD num = params.getTableInt("num");
  pUser->getSceneFood().addTasterExp(num);
  return true;
}

bool GMCommandRuler::addfooddataexp(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  
  DWORD t = params.getTableInt("t");
  DWORD itemId = params.getTableInt("itemid");
  DWORD exp = params.getTableInt("exp");
  
  if (!EFoodDataType_IsValid(t))
    return false;
  if (itemId == 0)
    return false;
  if (exp == 0)
    return false;
  EFoodDataType type = static_cast<EFoodDataType>(t);
  pUser->getSceneFood().addFoodDataExp(type, itemId, exp);
  return true;
}

bool GMCommandRuler::sceneeffect(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser->getScene() == nullptr)
    return false;

  if (params.has("effect") && params.has("range") && params.has("duration"))
  {
    DWORD count = SceneActManager::getMe().getEffectCount(pUser->id);
    if (count >= MiscConfig::getMe().getFoodCfg().dwMaxPutMatCount)
    {
      MsgManager::sendMsg(pUser->id, 3511);
      XLOG << "[料理-野餐垫] 不可放置，已达最大放置个数" << count << XEND;
      return false;
    }

    float range = params.getTableFloat("range");
    DWORD dir = pUser->getUserSceneData().getDir();
    xPos newPos;
    pUser->getScene()->getPosByDir(pUser->getPos(), range, dir, newPos);
    
    string path = params.getTableString("effect");

    DWORD effectDir = calcAngle(newPos, pUser->getPos()) * ONE_THOUSAND;
    
    SceneActBase* pActBase = SceneActManager::getMe().createSceneAct(pUser->getScene(), newPos, 0, 0, EACTTYPE_EFFECT);
    if (pActBase == nullptr)
      return false;
    SceneActEffect* pActEffect = dynamic_cast<SceneActEffect*> (pActBase);
    if (pActEffect)
      pActEffect->setEffectInfo(path, params.getTableInt("duration"), pUser->id, effectDir);
    if (!pActBase->enterScene(pUser->getScene()))
    {
      SceneActManager::getMe().delSceneAct(pActBase);
      return false;
    }
  }
  else
    return false;
  return true;
}

bool GMCommandRuler::clearsatiety(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  pUser->getSceneFood().clearSatiety();
  return true;
}

bool GMCommandRuler::unlockrecipe(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (!params.has("id"))
    return false;

  pUser->getSceneFood().unlockRecipe(params.getTableInt("id"));

  return true;
}

bool GMCommandRuler::advancepro(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (!params.has("id"))
    return false;

  DWORD profession = params.getTableInt("id");
  if (profession <= EPROFESSION_MIN || profession >= EPROFESSION_MAX || EProfession_IsValid(profession) == false)
    return false;

  TVecProfession pros;
  RoleConfig::getMe().getAdvanceProPath(static_cast<EProfession>(profession), pros);

  auto joblvup = [&]() {
    QWORD total = 0;
    for (DWORD i = 0; i < 999; ++i)
    {
      const SUserJobLvCFG* pCFG = JobLevelConfig::getMe().getJobLvCFG(pUser->getFighter()->getJobLv() + i + 1);
      if (pCFG)
        total += pCFG->needExp;
      else
        break;
    }
    pUser->addJobExp(total, ESOURCE_GM);
  };

  bool start = false;
  for (auto it = pros.rbegin(); it != pros.rend(); ++it)
  {
    if (!start && pUser->getProfession() == *it)
    {
      start = true;
      continue;
    }
    if (!start)
      continue;
    joblvup();
    pUser->exchangeProfession(*it);
  }
  if (start)
    joblvup();

  return true;
}

bool GMCommandRuler::tutorskill(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.getTableInt("GM_Target") == 0)
  {
    XERR << "[GM指令-导师]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未选择目标" << XEND;
    return false;
  }

  SceneUser* pTargetUser = SceneUserManager::getMe().getUserByID(params.getTableQWORD("id1"));
  if (pTargetUser == nullptr)
  {
    XERR << "[GM指令-导师]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未找到目标" << params.getTableQWORD("id1") << XEND;
    return false;
  }
  if (pTargetUser->isAlive() == false)
    return false;

  if (pUser->getSocial().checkRelation(pTargetUser->id, ESOCIALRELATION_STUDENT) == false ||
      pTargetUser->getSocial().checkRelation(pUser->id, ESOCIALRELATION_TUTOR) == false)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_TUTOR_STUDENT_ONLY);
    return false;
  }
  //const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();
  //if (pTargetUser->getLevel() < rCFG.dwStudentBaseLvReq || pTargetUser->getLevel() >= rCFG.dwTutorBaseLvReq)
  if (params.has("lv"))
  {
    DWORD lv1 = params.getData("lv").getTableInt("1");
    DWORD lv2 = params.getData("lv").getTableInt("2");
    if (pTargetUser->getLevel() < lv1 || pTargetUser->getLevel() >= lv2)
    {
      MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_TUTOR_STUDENT_OVER);
      return false;
    }
  }
  if (params.has("profession"))
  {
    DWORD profession = params.getTableInt("profession");
    DWORD userpro = pTargetUser->getProfession();
    if (userpro % 10 > profession)
    {
      MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_TUTOR_STUDENT_OVER);
      return false;
    }
  }

  DWORD dwBuffID = params.getTableInt("buff");
  const string& itemname = params.getTableString("name");
  if (dwBuffID != 0 && pTargetUser->getBuff().haveBuff(dwBuffID) == true)
  {
    MsgManager::sendMsg(pUser->id, 3227, MsgParams(itemname));
    return false;
  }

  DWORD skillid = params.getTableInt("skill");
  if (pUser->m_oSkillProcessor.useBuffSkill(pUser, pTargetUser, skillid, true) == false)
    return false;

  pUser->getServant().onFinishEvent(ETRIGGER_TUTOR_SKILL);
  pUser->getServant().onGrowthFinishEvent(ETRIGGER_TUTOR_SKILL);
  return true;
}

bool GMCommandRuler::tutor(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  /*if (params.getTableInt("GM_Target") == 0)
  {
    XERR << "[GM指令-导师]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未选择目标" << XEND;
    return false;
  }
  */

  SceneUser* pTargetUser = SceneUserManager::getMe().getUserByID(params.getTableQWORD("id1"));
  if (pTargetUser == nullptr)
  {
    XERR << "[GM指令-导师]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未找到目标" << params.getTableQWORD("id1") << XEND;
    return false;
  }
  if (pTargetUser->isAlive() == false)
  {
    XERR << "[GM指令-导师]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,目标" << params.getTableQWORD("id1") << "死亡" << XEND;
    return false;
  }
  /*
  if (pUser->getSocial().checkRelation(pTargetUser->id, ESOCIALRELATION_STUDENT) == false ||
      pTargetUser->getSocial().checkRelation(pUser->id, ESOCIALRELATION_TUTOR) == false)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_TUTOR_STUDENT_ONLY);
    return false;
  }
  const STutorMiscCFG& rCFG = MiscConfig::getMe().getTutorCFG();
  if (pTargetUser->getLevel() < rCFG.dwStudentBaseLvReq || pTargetUser->getLevel() >= rCFG.dwTutorBaseLvReq)
  {
    MsgManager::sendMsg(pUser->id, ESYSTEMMSG_ID_TUTOR_STUDENT_OVER);
    return false;
  }

  DWORD dwBuffID = params.getTableInt("buff");
  const string& itemname = params.getTableString("name");
  if (dwBuffID != 0 && pTargetUser->getBuff().haveBuff(dwBuffID) == true)
  {
    MsgManager::sendMsg(pUser->id, 3227, MsgParams(itemname));
    return false;
  }
  */

  const string& itemname = params.getTableString("name");
  DWORD dwID = GuidManager::getMe().getNextLetterID();
  xLuaData action = params.getData("actions");
  auto actionf = [&](const string& key, xLuaData& data)
  {
    pTargetUser->getUserAction().addAction(dwID, data.getInt());
  };
  action.foreach(actionf);

  const static TVecDWORD vecIDs = TVecDWORD{1000,1100};
  for (auto &v : vecIDs)
  {
    SysMsgAction* pSysAction = dynamic_cast<SysMsgAction*>(pTargetUser->getUserAction().getActionItem(dwID, v));
    if (pSysAction != nullptr)
    {
      pSysAction->getParams().addString(pUser->name);
      pSysAction->getParams().addString(itemname);
    }
  }

  return true;
}

// 完成看板任务, 直接跳到前往看板交付任务那一步骤
bool GMCommandRuler::finishboardquest(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  pair<DWORD, DWORD> curwanted;
  if (pUser->getQuest().getWantedQuest(curwanted) == false)
  {
    bool f = false;
    if (params.has("id"))
    {
      DWORD id = params.getTableInt("id");
      if (pUser->getQuest().acceptQuest(id, true) == false)
      {
        XERR << "[GM指令-直接完成看板任务]" << pUser->accid << pUser->id << pUser->name << "任务:" << id << "接取失败" << XEND;
        return false;
      }
      if (pUser->getQuest().getWantedQuest(curwanted))
        f = true;
    }

    if (f == false)
    {
      MsgManager::sendMsg(pUser->id, 3229);
      return false;
    }
  }
  const SQuestCFGEx* pCfg = QuestManager::getMe().getQuestCFG(curwanted.first);
  if (pCfg == nullptr || pCfg->eType != EQUESTTYPE_WANTED)
    return false;
  if (pCfg->vecSteps.size() < 2)
    return false;

  // 已达倒数第二步, 看板任务默认倒数第二步为前往看板交付任务步骤
  DWORD targetstep = pCfg->vecSteps.size() - 2;
  if (curwanted.second >= targetstep)
  {
    MsgManager::sendMsg(pUser->id, 3234, MsgParams(pCfg->getName()));
    return false;
  }

  // 先设置step, 再处理rewardhelp, 防止设置出错导致刷奖励
  if (pUser->getQuest().setQuestStep(curwanted.first, targetstep) == false)
  {
    XERR << "[GM指令-直接完成看板任务]" << pUser->accid << pUser->id << pUser->name << "任务:" << curwanted.first << "当前步骤:" << curwanted.second << "设定步骤:" << targetstep << XEND;
    return false;
  }

  // reward_help类型发放友情奖励, 若该步骤未执行过, 则单独执行
  for (DWORD i = 0; i < pCfg->vecSteps.size(); ++i)
  {
    if (pCfg->vecSteps[i]->getStepType() == EQUESTSTEP_REWRADHELP)
    {
      if (curwanted.second > i)
        continue;
      RewardHelpStep* pStep = dynamic_cast<RewardHelpStep*>(pCfg->vecSteps[i].get());
      if (pStep == nullptr)
      {
        XERR << "[GM指令-直接完成看板任务]" << pUser->accid << pUser->id << pUser->name << "任务:" << curwanted.first << "步骤:" << i << "获取rewardhelp步骤失败" << XEND;
        continue;
      }
      pStep->sendHelpReward(pUser);
    }
    else if (pCfg->vecSteps[i]->getStepType() == EQUESTSTEP_GMCMD)
    {
      GMCmdStep* pStep = dynamic_cast<GMCmdStep*>(pCfg->vecSteps[i].get());
      if (pStep == nullptr)
      {
        XERR << "[GM指令-直接完成看板任务]" << pUser->accid << pUser->id << pUser->name << "任务:" << curwanted.first << "步骤:" << i << "获取gm步骤失败" << XEND;
        continue;
      }
      xLuaData& oData = pStep->getData();
      const string& type = oData.getTableString("type");
      if (type == "follower" && GMCommandRuler::getMe().execute(pUser, oData) == false)
      {
        XERR << "[GM指令-直接完成看板任务]" << pUser->accid << pUser->id << pUser->name << "任务:" << curwanted.first << "步骤:" << i << "执行 follower gm步骤失败" << XEND;
        continue;
      }
    }
  }

  if (params.has("effectid"))
  {
    pUser->effect(params.getTableInt("effectid"));
  }

  if (params.has("submit"))
  {
    if (pUser->getQuest().finishQuest(curwanted.first) == false)
    {
      XERR << "[GM指令-直接完成看板任务]" << pUser->accid << pUser->id << pUser->name << "任务:" << curwanted.first << "提交任务失败" << XEND;
      return false;
    }
  }

  MsgManager::sendMsg(pUser->id, 3301, MsgParams(pCfg->getName()));
  XLOG << "[GM指令-直接完成看板任务]" << pUser->accid << pUser->id << pUser->name << "任务:" << curwanted.first << "当前步骤:" << curwanted.second << "设定步骤:" << targetstep << XEND;
  return true;
}

bool GMCommandRuler::settowermaxlayer(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD dwOpenTowerLayer = MiscConfig::getMe().getSystemCFG().dwOpenTowerLayer;
  if(dwOpenTowerLayer > pUser->getTower().getMaxLayer())
    dwOpenTowerLayer = pUser->getTower().getMaxLayer();
  for (DWORD d = 1; d <= dwOpenTowerLayer; ++d)
    pUser->getTower().passLayer(d, false);
  pUser->updateTeamTower();
  MsgManager::sendMsg(pUser->id, 3404, MsgParams(dwOpenTowerLayer));
  XLOG << "[GM-无限塔]" << pUser->name << "设置当前最高层: " << dwOpenTowerLayer << XEND;

  return true;
}

bool GMCommandRuler::being(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser == nullptr)
    return false;

  if (params.has("summon"))
  {
    pUser->getUserBeing().summon(params.getTableInt("summon"));
  }
  else if (params.has("revive"))
  {
    DWORD hppercent = 100;
    if (params.has("hp"))
      hppercent = params.getTableInt("hp");
    pUser->getUserBeing().revive(hppercent);
  }
  else if (params.has("exp"))
  {
    pUser->getUserBeing().addBaseExp(params.getTableQWORD("exp"));
  }
  else if (params.has("resetskill"))
  {
    if (params.getTableInt("resetskill") == 1)
      pUser->getUserBeing().resetAllSkill(pUser->getUserBeing().getCurBeingID());
    else
      pUser->getUserBeing().resetAllSkill(params.getTableInt("resetskill"));
  }
  else if (params.has("hp"))
  {
    SceneNpc* npc = pUser->getUserBeing().getCurBeingNpc();
    if (npc)
    {
      float hp = params.getTableInt("hp");
      float curhp = npc->getAttr(EATTRTYPE_HP);
      npc->changeHp(hp - curhp, npc);
    }
  }
  else if (params.has("unlockskill"))
  {
    SSceneBeingData* b = pUser->getUserBeing().getBeingData(params.getTableInt("unlockskill"));
    if (b)
      b->unlockSkill(nullptr);
  }
  else if (params.has("reset"))
  {
    pUser->getUserBeing().decSkillPoint(0);
  }
  else if (params.has("addskill"))
  {
    pUser->getUserBeing().addSkill(params.getTableInt("id"), params.getTableInt("addskill"));
  }

  return true;
}

bool GMCommandRuler::lottery(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD dwMethod = params.getTableInt("method");

  if (dwMethod == 1)
  {
    time_t tNow = xTime::getCurSec();
    DWORD dwYear = xTime::getYear(tNow);
    DWORD dwMonth = xTime::getMonth(tNow);
    const string& strRarity = params.getTableString("rarity");
    const SLotteryCFG* pCFG = ItemConfig::getMe().getLotteryCFG(1, dwYear, dwMonth, 0);
    if (pCFG == nullptr)
    {
      XERR << "[GM指令-lottery]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "执行 method :" << dwMethod << "year :" << dwYear << "month :" << dwMonth << "失败, 未在 Table_Lottery.txt 表中找到" << XEND;
      return false;
    }

    const SLotteryItemCFG* pItemCFG = pCFG->getItemCFG(strRarity);
    if (pItemCFG == nullptr)
    {
      XERR << "[GM指令-lottery]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "执行 method :" << dwMethod << "year :" << dwYear << "month :" << dwMonth << "rarity :" << strRarity << "失败, 未在 Table_Lottery.txt 表中找到" << XEND;
      return false;
    }

    ItemData oData;
    oData.mutable_base()->set_id(pItemCFG->getItemIdByGender(pUser->getUserSceneData().getGender()));
    oData.mutable_base()->set_count(1);

    if (pUser->getPackage().addItem(oData, EPACKMETHOD_NOCHECK) == false)
    {
      XERR << "[GM指令-lottery]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
        << "执行 method :" << dwMethod << "year :" << dwYear << "month :" << dwMonth << "rarity :" << strRarity << "失败, 添加失败" << XEND;
      return false;
    }

    XLOG << "[GM指令-lottery]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "执行 method :" << dwMethod << "year :" << dwYear << "month :" << dwMonth << "rarity :" << strRarity << "成功" << XEND;
  }
  else
  {
    return false;
  }

  return true;
}

bool GMCommandRuler::beingbody(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD beingid = params.getTableInt("beingid");
  if (beingid == 0)
    return false;

  if (0 == strcmp(params.getTableString("action"), "add"))
  {
    DWORD bodyid = params.getTableInt("bodyid");
    if (bodyid)
      pUser->getUserBeing().addBody(beingid, bodyid);
  }
  if (0 == strcmp(params.getTableString("action"), "change"))
  {
    DWORD bodyid = params.getTableInt("bodyid");
    if (bodyid)
    {
      SSceneBeingData* pData = pUser->getUserBeing().getCurBeingData();
      if (pData && pData->bLive && pData->dwID == beingid)
      {
        ExchangeProfession cmd;
        cmd.set_guid(pData->qwTempID);
        UserData* p = cmd.add_datas();
        if (p)
        {
          p->set_type(EUSERDATATYPE_BODY);
          p->set_value(bodyid);
        }
        PROTOBUF(cmd, send, len);
        pUser->sendCmdToNine(send, len);
      }

      pUser->getUserBeing().changeBody(beingid, bodyid);
    }
  }

  return true;
}

bool GMCommandRuler::addguildpray(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (pUser->getGuild().id() == 0)
    return false;



  DWORD dwPrayID = params.getTableInt("prayid");
  DWORD dwValue = params.getTableInt("value");

  DWORD dwMaxLv = GuildConfig::getMe().getMaxPrayLv();
  if(dwValue == 0 || dwValue > dwMaxLv)
  {
    dwValue = dwMaxLv;
  }

  GuildPrayGuildSCmd cmd;
  SocialUser sUser;
  pUser->toData(&sUser);
  cmd.mutable_user()->CopyFrom(sUser);
  cmd.set_prayid(dwPrayID);
  cmd.set_praylv(dwValue);
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::scene_changebody(Scene* pScene, const xLuaData& params)
{
  if (params.has("npc") == false)
    return false;

  const auto getbody = [&](const string& key, xLuaData& d)
  {
    DWORD npcid = d.getTableInt("1");
    DWORD buffid = d.getTableInt("2");
    SceneNpcManager::getMe().addChangeBodyNpc(npcid, buffid);
  };
  xLuaData data = params;
  data.getMutableData("npc").foreach(getbody);

  return true;
}

bool GMCommandRuler::breakequip(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD pos = params.getTableInt("pos");
  DWORD time = params.getTableInt("time");
  if (pos && time && pos > EEQUIPPOS_MIN && pos < EEQUIPPOS_MAX && EEquipPos_IsValid(pos))
    return pUser->getPackage().breakEquip(static_cast<EEquipPos>(pos), time, entry);

  return true;
}

bool GMCommandRuler::guild(xSceneEntryDynamic* entry, const xLuaData& params)
{
  //GET_USER(entry);

  stringstream ss;
  params.toJsonString(ss);
  if (ss.str().empty())
    return false;

  GMCommandGuildSCmd cmd;
  cmd.mutable_info()->set_zoneid(thisServer->getZoneID());
  cmd.mutable_info()->set_charid(entry == nullptr ? 0 : entry->id);
  if (params.has("guildid") == true)
    cmd.mutable_info()->set_guildid(params.getTableQWORD("guildid"));
  if (params.has("name") == true)
    cmd.mutable_info()->set_name(params.getTableString("name"));
  cmd.set_command(ss.str());
  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);
  return true;
}

bool GMCommandRuler::callteamer(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (pUser->getTeamID() == 0)
    return false;

  GTeam& myteam = pUser->getTeam();
  if (myteam.getTeamMemberList().size() <= 1)
    return false;

  Scene* pScene = pUser->getScene();
  if (pScene == nullptr)
    return false;

  if (pScene->getSceneType() != SCENE_TYPE_GUILD_FIRE)
    return false;

  CallTeamerUserCmd cmd;
  cmd.set_masterid(pUser->id);
  cmd.set_time(now() + 30);
  cmd.set_username(pUser->name);
  cmd.set_mapid(pScene->id);
  ScenePos* pPos = cmd.mutable_pos();
  if (pPos == nullptr)
    return false;

  for (auto &m : myteam.getTeamMemberList())
  {
    if (m.first == pUser->id)
      continue;
    xPos userpos = pUser->getPos();
    pScene->getRandPos(pUser->getPos(), 5, userpos);

    pPos->set_x(userpos.getX());
    pPos->set_y(userpos.getY());
    pPos->set_z(userpos.getZ());

    char sign[1024];
    bzero(sign, sizeof(sign));
    std::stringstream ss;
    ss.str("");
    ss << cmd.masterid() << "@" << cmd.time() << "@" << cmd.mapid() << "@" << pPos->x() << pPos->y() << pPos->z() <<"_" << "#$%^&";

    upyun_md5(ss.str().c_str(), ss.str().size(), sign);
    cmd.set_sign(sign);
    PROTOBUF(cmd, send, len);
    thisServer->sendCmdToMe(m.first, send, len);
  }

  XLOG << "[公会战], 发起召唤队友, 玩家:" << pUser->name << pUser->id << "当前地图:" << pScene->id << XEND;
  return true;
}

bool GMCommandRuler::guildchallenge(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.getTableInt("reset") == 1)
  {
    pUser->getGuildChallenge().resetProgress(true);
    XLOG << "[GM指令-重置公会挑战进度]" << pUser->accid << pUser->id << pUser->name << "重置成功" << XEND;
  }

  return true;
}

bool GMCommandRuler::scene_showseat(Scene* pScene, const xLuaData& params)
{
  if(pScene == nullptr)
    return false;

  SeatShowType eType = static_cast<SeatShowType>(params.getTableInt("showtype"));
  TSetDWORD setSeatIDs;
  xLuaData ids = params.getData("seatid");
  auto func = [&](const string& key, xLuaData& data)
  {
    setSeatIDs.insert(data.getInt());
  };
  ids.foreach(func);

  Cmd::ShowSeatUserCmd cmd;
  cmd.set_show(eType);
  if(eType == SEAT_SHOW_VISIBLE)
  {
    for(auto s : setSeatIDs)
    {
      pScene->addSeat(s);
      cmd.add_seatid(s);
    }
  }
  else if(eType == SEAT_SHOW_INVISIBLE)
  {
    for(auto s : setSeatIDs)
    {
      pScene->delSeat(s);
      cmd.add_seatid(s);
    }
  }
  else
    return false;

  PROTOBUF(cmd, send, len);
  pScene->sendCmdToAll(send, len);

  return true;
}

bool GMCommandRuler::addjoyvalue(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  EJoyActivityType etype = JOY_ACTIVITY_ATF;
  if (params.has("activity_type"))
    etype = static_cast<EJoyActivityType>(params.getTableInt("activity_type"));

  DWORD value = params.getTableInt("value");
  if (value == 0)
    return false;

  DWORD curvalue = pUser->getUserSceneData().getJoyByType(etype);
  DWORD maxvalue = MiscConfig::getMe().getJoyLimitByType(etype);

  if (params.has("msgid"))
  {
    if (curvalue < maxvalue)
      MsgManager::sendMsg(pUser->id, params.getTableInt("msgid"), MsgParams(value));
  }

  if (curvalue < maxvalue)
  {
    pUser->getUserSceneData().addJoyValue(etype, value);
    if (value + curvalue >= maxvalue)
      MsgManager::sendMsg(pUser->id, 3615);
    XLOG << "[GM-欢乐值], 欢乐值增加, 玩家:" << pUser->name << pUser->id << "增加值:" << value << "当前值:" << pUser->getUserSceneData().getTotalJoy() << XEND;
  }
  else
  {
    XLOG << "[GM-欢乐值], 欢乐值增加, 已达上限, 玩家:" << pUser->name << pUser->id << "增加值:" << value << "当前值:" << pUser->getUserSceneData().getTotalJoy() << XEND;
  }

  return true;
}

bool GMCommandRuler::addbattletime(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD time = params.getTableInt("time");
  if (time == 0)
    return false;
  pUser->addReBattleTime(time);

  MsgManager::sendMsg(pUser->id, 3640, MsgParams(time));
  XLOG << "[GM-战斗时长], 战斗时长增加, 玩家:" << pUser->name << pUser->id << "增加时长:" << time << XEND;
  return true;
}


bool GMCommandRuler::addmanualattributes(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD dwType = params.getTableInt("manualtype");
  EManualType eType = static_cast<EManualType>(dwType);
  pUser->getManual().addAttributes(eType);

  return true;
}

bool GMCommandRuler::manualleveldown(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD lv = params.getTableInt("level");
  return pUser->getManual().leveldown(lv);
}

bool GMCommandRuler::deltitle(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD title = params.getTableInt("title");
  ETitleType eType = static_cast<ETitleType>(params.getTableInt("etype"));

  return pUser->getTitle().delTitle(eType, title);
}

bool GMCommandRuler::addmaxjoblv(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD lv = params.getTableInt("value");
  pUser->getCurFighter()->setMaxJobLv(lv);

  return true;
}

bool GMCommandRuler::scene_gvg(Scene* pScene, const xLuaData& params)
{
  if (params.has("zoneid"))
  {
    DWORD zoneid = params.getTableInt("zoneid");
    if (thisServer->getZoneID() != zoneid)
      return true;
  }

  if (0 == strcmp(params.getTableString("action"), "onoff"))
  {
    bool open = params.getTableInt("open");

    if (params.has("city"))
    {
      DWORD cityid = params.getTableInt("city");
      GuildCity* pCity = GuildCityManager::getMe().getCityByID(cityid);
      if (pCity)
      {
        if (open)
          pCity->onFireOpen();
        else
          pCity->onFireClose();
        XLOG << "[GM-scene_gvg], 执行成功, 城池:" << cityid << "开启状态:" << open << XEND;
      }
      return true;
    }
    bool super = params.getTableInt("super");
    DWORD supertime = params.getTableInt("supertime");

    if (open)
      GuildCityManager::getMe().openFireAtonce(super, supertime);
    else
      GuildCityManager::getMe().stopFireAtonce();
    XLOG << "[GM-scene_gvg], 执行公会战开启/关闭成功, 开启状态:" << open << XEND;
  }

  else if (0 == strcmp(params.getTableString("action"), "occpy"))
  {
    QWORD guild = 0;
    if (params.has("guild"))
      guild = params.getTableInt("guild");
    DWORD flagid = params.getTableInt("city");
    GuildCityManager::getMe().updateCityInfoToGuild(flagid, guild);
    XLOG << "[GM-scene_gvg], 执行城池占领成功, 公会id:" << guild << "城池:" << flagid << XEND;
  }
  return true;
}

bool GMCommandRuler::marriageproposal(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (params.getTableInt("GM_Target") == 0)
  {
    XERR << "[GM指令-求婚]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未选择目标" << XEND;
    return false;
  }

  SceneUser* pOther = SceneUserManager::getMe().getUserByID(params.getTableQWORD("id1"));
  if (pOther == nullptr)
  {
    XERR << "[GM指令-求婚]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行失败,未找到目标" << params.getTableQWORD("id1") << XEND;
    return false;
  }
  if (pOther->isAlive() == false)
    return false;

  //检测对方黑名单检测
  if (pOther->getSocial().checkRelation(pUser->id, ESOCIALRELATION_BLACK) == true ||
      pOther->getSocial().checkRelation(pUser->id, ESOCIALRELATION_BLACK_FOREVER) == true)
  {
    XERR << "[GM指令-求婚]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name
      << "向" << pOther->accid << pOther->id << pOther->getProfession() << pOther->name << "求婚失败,黑名单" << XEND;
    return false;
  }

  //检测竞技场
  Scene* pScene = pUser->getScene();
  Scene* pOtherScene = pOther->getScene();
  if(pScene == nullptr || pOtherScene == nullptr)
  {
    XERR << "[GM指令-求婚]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "获取场景失败" << XEND;
    return false;
  }
  if(pScene->isPVPScene() || pScene->isGvg()
      || pOtherScene->isPVPScene() || pOtherScene->isGvg())
  {
    XERR << "[GM指令-求婚]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "禁止在pvp中使用" << XEND;
    return false;
  }

  DWORD itemid = params.getTableInt("itemid");
  pUser->getProposal().popTheQuest(pOther->id, itemid);

  return true;
}

bool GMCommandRuler::rmcodeitem(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  string code = params.getTableString("code");
  if (code.empty())
  {
    XERR << "[GM-删除兑换码道具]" << pUser->accid << pUser->id << pUser->name << "兑换码为空" << XEND;
    return false;
  }
  DWORD itemid = params.getTableInt("itemid");
  if (!itemid)
  {
    XERR << "[GM-删除兑换码道具]" << pUser->accid << pUser->id << pUser->name << "兑换码:" << code << "道具id为空" << XEND;
    return false;
  }

  auto rmitemf = [&](EPackType packtype) -> bool {
    BasePackage* pack = pUser->getPackage().getPackage(packtype);
    if (!pack)
      return false;
    const TSetItemBase& items = pack->getItemBaseList(itemid);
    for (auto item : items)
    {
      ItemCode* citem = dynamic_cast<ItemCode*>(item);
      if (citem && citem->getCode() == code)
      {
        pack->reduceItem(item->getGUID(), ESOURCE_KFC_ACTIVITY);
        XLOG << "[GM-删除兑换码道具]" << pUser->accid << pUser->id << pUser->name << "道具:" << itemid << "兑换码:" << code << "道具删除成功" << XEND;
        return true;
      }
    }
    return false;
  };

  if (rmitemf(EPACKTYPE_MAIN) || rmitemf(EPACKTYPE_TEMP_MAIN) || rmitemf(EPACKTYPE_PERSONAL_STORE) || rmitemf(EPACKTYPE_STORE))
    return true;

  return false;
}

bool GMCommandRuler::tower(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (pUser->getMenu().isOpen(EMENUID_ENDLESSTOWER) == false)
  {
    MsgManager::sendMsg(pUser->id, 56);
    return false;
  }

  const string& action = params.getTableString("action");
  SceneTower& rTower = pUser->getTower();

  if (action == "passlayer")
  {
    DWORD dwMin = params.getTableInt("min");
    DWORD dwMax = params.getTableInt("max");

    if (dwMin > dwMax)
    {
      XERR << "[GM指令-无限塔]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "通关失败,最小层" << dwMin << "大于最大层" << dwMax << XEND;
      return false;
    }

    bool bEnable = false;
    TVecItemInfo vecAllItemInfo;
    vector<pair<DWORD, float>> vecAllRewardIDs;
    for (DWORD d = dwMin; d <= dwMax; ++d)
    {
      if (rTower.isRewarded(d) == true)
        continue;

      if(rTower.getEverPassLayer(d) == false)
        continue;

      const STowerLayerCFG* pCFG = TowerConfig::getMe().getTowerLayerCFG(d);
      if (pCFG == nullptr)
      {
        XERR << "[GM指令-无限塔]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "通关失败,层" << d << "未在 Table_Tower.txt 表中找到" << XEND;
        return false;
      }

      TVecItemInfo vecItemInfo;
      vecAllRewardIDs.clear();
      for (auto v = pCFG->vecCurMonster.begin(); v != pCFG->vecCurMonster.end(); ++v)
      {
        pUser->getManual().onKillMonster(v->dwMonsterID);
        const SNpcCFG* pCFG = NpcConfig::getMe().getNpcCFG(v->dwMonsterID);
        if(pCFG != nullptr && pCFG->vecRelevancyIDs.empty() == false)
        {
          for(auto &c : pCFG->vecRelevancyIDs)
          {
            pUser->getManual().onKillMonster(c);
            pUser->getManual().onKillProcess(c);
          }
        }
        TowerScene::addMonsterReward(v->dwMonsterID, vecAllRewardIDs, d, 0, false);
      }
      TowerRewardPhase::rollAllReward(pUser, vecItemInfo, vecAllRewardIDs, d);
      combinItemInfo(vecAllItemInfo, vecItemInfo);
      pUser->getTaskExtraReward(ETASKEXTRAREWARDTYPE_ENDLESS, d);

      std::set<SceneUser*> userset = pUser->getTeamSceneUser();
      std::set<SceneUser*> helpuserset;
      helpuserset.emplace(pUser);
      for(auto &s : userset)
      {
        if(pUser->check2PosInNine(s) == false || s->id == pUser->id)
          continue;

        if (pCFG->dwMvpCount != 0)
          s->getHelpReward(helpuserset, EHELPTYPE_TOWER_MVP);
        else if(pCFG->dwMiniCount != 0)
          s->getHelpReward(helpuserset, EHELPTYPE_TOWER_MINI);
      }

      if (pCFG->dwMvpCount != 0)
        pUser->getHelpReward(userset, EHELPTYPE_TOWER_MVP, true);
      else if(pCFG->dwMiniCount != 0)
        pUser->getHelpReward(userset, EHELPTYPE_TOWER_MINI, true);

      bEnable = true;
      XLOG << "[GM指令-无限塔]  层数奖励: " << pUser->accid << pUser->id << pUser->name << "层数: " << d << XEND;
    }
    if (!bEnable)
    {
      MsgManager::sendMsg(pUser->id, 57);
      return false;
    }

    for (DWORD d = dwMin; d <= dwMax; ++d)
    {
      if (rTower.isRewarded(d) == false && rTower.getEverPassLayer(d) == true)
        rTower.passLayer(d, true, false);
    }
    if (vecAllItemInfo.empty() == false)
      pUser->getPackage().addItem(vecAllItemInfo, EPACKMETHOD_AVAILABLE);

    if (pUser->getTeam().getTeamID() != 0)
      pUser->updateTeamTower();
    rTower.sendTowerData();
    pUser->getServant().onGrowthFinishEvent(ETRIGGER_USE_CATAPULT);
    XLOG << "[GM指令-无限塔]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "通关成功,层" << dwMin << "->" << dwMax << XEND;
  }
  else
  {
    return false;
  }

  return true;
}

bool GMCommandRuler::laboratory(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  UserLaboratory& rLab = pUser->getLaboratory();
  const string& action = params.getTableString("action");

  if (action == "complete")
  {
    if (pUser->getMenu().isOpen(EMENUID_LABORATORY) == false)
    {
      MsgManager::sendMsg(pUser->id, 67);
      return false;
    }

    DWORD dwValue = params.getTableInt("value");
    DWORD dwTodayMaxPoint = rLab.getTodayMaxPoint();
    if (dwValue <= dwTodayMaxPoint)
    {
      MsgManager::sendMsg(pUser->id, 58);
      return false;
    }

    DWORD dwGarden = dwValue - dwTodayMaxPoint;
    if (dwGarden == 0)
    {
      XERR << "[GM指令-研究所]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "完成失败,乐园币随机结果为0" << XEND;
      return false;
    }

    std::set<SceneUser*> firstUserSet;
    std::set<SceneUser*> notFirstSet;
    std::set<SceneUser*> allUser;
    GTeam& rTeam = pUser->getTeam();
    const TMapGTeamMember& mapMember = rTeam.getTeamMemberList();
    for (auto &m : mapMember)
    {
      SceneUser* pMember = SceneUserManager::getMe().getUserByID(m.first);
      if (pMember == nullptr || pUser->check2PosInNine(pMember) == false)
        continue;

      DWORD dwTodayMaxPoint = pMember->getLaboratory().getTodayMaxPoint();
      if (dwTodayMaxPoint == 0)
        firstUserSet.insert(pMember);
      else
        notFirstSet.insert(pMember);
      allUser.insert(pMember);
    }

    DWORD finishcount = pUser->getVar().getVarValue(EVARTYPE_LABORATORY_COUNT) + 1;
    pUser->getVar().setVarValue(EVARTYPE_LABORATORY_COUNT, finishcount);
    const SLaboratoryCFG& rCFG = MiscConfig::getMe().getLaboratoryCFG();
    DWORD dwAddPoint = dwGarden * (rCFG.dwGarden + pUser->getLevel() * rCFG.fGarden);

    TVecItemInfo vecReward;
    DWORD doublereward = pUser->getDoubleReward(EDOUBLEREWARD_LABORATORY, vecReward);
    TVecItemInfo extrareward;
    if (doublereward <= 1)
      ActivityEventManager::getMe().getReward(pUser, EAEREWARDMODE_LABORATORY, finishcount, extrareward, doublereward);

    float zenyratio = pUser->m_oBuff.getExtraZenyRatio(EEXTRAREWARD_LABORATORY);
    if (zenyratio)
      XLOG << "[GM指令-研究所]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "完成, 额外zeny倍率:" << zenyratio << XEND;
    zenyratio += 1;
    dwGarden *= zenyratio;

    if (doublereward)
      dwGarden = dwGarden * doublereward;

    ItemData oData;
    oData.mutable_base()->set_id(ITEM_GARDEN);
    oData.mutable_base()->set_count(dwGarden);
    oData.mutable_base()->set_source(ESOURCE_LABORATORY);
    if (pUser->getPackage().addItem(oData, EPACKMETHOD_NOCHECK) == false)
    {
      XERR << "[GM指令-研究所]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "完成失败,添加" << oData.ShortDebugString() << "失败" << XEND;
      return false;
    }
    rLab.setTodayPoint(dwAddPoint + dwTodayMaxPoint);

    pUser->getExtraReward(EEXTRAREWARD_LABORATORY);
    if (pUser->getVar().getVarValue(EVARTYPE_LABORATORY_EXTASKREWARD) == 0)
    {
      pUser->getVar().setVarValue(EVARTYPE_LABORATORY_EXTASKREWARD, 1);
      pUser->getTaskExtraReward(ETASKEXTRAREWARDTYPE_INSTITUTE, 1);
    }

    if (extrareward.empty() == false)
    {
      XLOG << "[GM指令-研究所]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "完成成功,获得" << extrareward << XEND;
      pUser->getPackage().addItem(extrareward, EPACKMETHOD_AVAILABLE);
    }

    pUser->getTutorTask().onLaboratoryFinish();
    pUser->m_oBuff.onFinishEvent(EEXTRAREWARD_LABORATORY);
    pUser->getServant().onFinishEvent(ETRIGGER_LABORATORY);

    XLOG << "[GM指令-研究所]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "完成成功,获得" << oData.ShortDebugString() << "maxpoint" << dwValue << "->" << rLab.getTodayMaxPoint() << XEND;

    //platlog
    QWORD eid = xTime::getCurUSec();
    EVENT_TYPE eType = EventType_Complete_Laboratory;
    PlatLogManager::getMe().eventLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eid,
      pUser->getUserSceneData().getCharge(), eType, 0, 1);

    PlatLogManager::getMe().CompleteLog(thisServer,
      pUser->getUserSceneData().getPlatformId(),
      pUser->getZoneID(),
      pUser->accid,
      pUser->id,
      eType,
      eid,
      ECompleteType_Laboratory,
      1,
      dwTodayMaxPoint,
      EMONEYTYPE_GARDEN, dwGarden,
      pUser->getLevel());

    {
      StatisticsDefine::sendStatLog(thisServer, ESTATTYPE_YJS_PASS_COUNT, 0,0, pUser->getLevel(), (DWORD)1);
    }
    for (auto &user : notFirstSet)
      user->getHelpReward(firstUserSet, EHELPTYPE_LABORATORY);
    // 只要一起的人中有我的好友即可
    for (auto &user : firstUserSet)
      user->getHelpReward(allUser, EHELPTYPE_LABORATORY, true);
  }
  else
  {
    return false;
  }

  return true;
}

bool GMCommandRuler::carddecompose(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  BasePackage* pack = pUser->getPackage().getPackage(EPACKTYPE_MAIN);
  if (pack == nullptr)
    return false;
  DWORD id = params.getTableInt("id"), cnt = params.getTableInt("count");
  if (cnt <= 0) cnt = 1;
  const TSetItemBase& items = pack->getItemBaseList(id);
  if (items.empty())
    return false;

  map<string, DWORD> guids;
  for (auto item : items)
  {
    if (item->getCount() >= cnt)
    {
      guids[item->getGUID()] = cnt;
      break;
    }
  }
  if (guids.empty())
    return false;

  TVecItemInfo outs;
  if (pUser->getPackage().cardDecompose(guids, outs) == false)
    return false;

  XLOG << "[GM-卡片分解]" << pUser->accid << pUser->id << pUser->name << "卡片:" << id << "数量:" << cnt << "分解获得:";
  for (auto& item : outs)
    XLOG << item.id() << item.count();
  XLOG << "成功" << XEND;
  return true;
}

bool GMCommandRuler::tutorgrowreward(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.has("lv"))
  {
    if (pUser->getTutorTask().getGrowReward(params.getTableInt("lv")))
    {
      XLOG << "[GM-领取学生成长奖励]" << pUser->accid << pUser->id << pUser->name << "等级:" << params.getTableInt("lv") << "领取成功";
      return true;
    }
    else
    {
      XERR << "[GM-领取学生成长奖励]" << pUser->accid << pUser->id << pUser->name << "等级:" << params.getTableInt("lv") << "领取失败";
      return false;
    }
  }
  if (params.has("mail"))
  {
    pUser->getTutorTask().sendGrowRewardMail();
    XLOG << "[GM-领取学生成长奖励]" << pUser->accid << pUser->id << pUser->name << "发送成长奖励邮件";
    return true;
  }

  return true;
}

bool GMCommandRuler::scene_dropitem(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;

  DWORD id = params.getTableInt("id");
  DWORD count = params.getTableInt("count");
  count = count == 0 ? 1 : count;

  if (params.has("pos") == false || params.has("range") == false)
    return false;

  DWORD range = params.getTableInt("range");

  if (id == 0 && params.has("rand_one_id"))
  {
    xLuaData iddata = params.getData("rand_one_id");
    TSetDWORD allid;
    iddata.getIDList(allid);
    auto p = randomStlContainer(allid);
    if (p)
      id = *p;
  }

  if (ItemConfig::getMe().getItemCFG(id) == nullptr)
  {
    XERR << "[Scene-GM]" << pScene->id << "dropItem 掉落物品失败, 策划表找不到物品" << id << XEND;
    return false;
  }

  xPos pos;
  pos.x = params.getData("pos").getTableFloat("1");
  pos.y = params.getData("pos").getTableFloat("2");
  pos.z = params.getData("pos").getTableFloat("3");

  if (pScene->getValidPos(pos) == false)
  {
    XERR << "[Scene-GM], dropitem 位置不合法, 地图:" <<  pScene->id << "道具:" << id << XEND;
    return false;
  }

  Cmd::AddMapItem cmd;
  DWORD extraTime = MiscConfig::getMe().getSceneItemCFG().dwDropInterval;

  ItemInfo stInfo;
  stInfo.set_id(id);
  stInfo.set_count(1);
  stInfo.set_source(ESOURCE_GM);
  for (DWORD i = 0; i < count; ++i)
  {
    xPos dest;
    if (pScene->getRandPos(pos, range, dest) == false)
    {
      XERR << "[Scene-GM]" << pScene->id << "dropItem 获取随机位置失败" << XEND;
      return false;
    }

    if (params.has("GM_ESource"))
    {
      DWORD source = params.getTableInt("GM_ESource");
      ESource esource = static_cast<ESource>(source);
      stInfo.set_source(esource);
    }
    else
    {
      stInfo.set_source(ESOURCE_PICKUP);
    }

    SceneItem* pItem = SceneItemManager::getMe().createSceneItem(pScene, stInfo, dest);
    if (pItem == nullptr)
    {
        XERR << "[Scene-GM]" << pScene->id << "dropItem SceneItem 创建失败" << XEND;
        return false;
    }
    pItem->fillMapItemData(cmd.add_items(), extraTime);

    if (cmd.items_size() > 100)
    {
      PROTOBUF(cmd, send, len);
      pScene->sendCmdToNine(pos, send, len);
      cmd.Clear();
    }
  }
  // inform client
  if (cmd.items_size() <= 100)
  {
    PROTOBUF(cmd, send, len);
    pScene->sendCmdToNine(pos, send, len);
  }

  return true;
}

bool GMCommandRuler::user(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  const string& action = params.getTableString("action");
  EError eErr = EERROR_SUCCESS;

  if (action == "changegender")
    eErr = pUser->changeGender();
  else if (action == "resethair")
    pUser->getHairInfo().resetHair();
  else if (action == "reseteye")
    pUser->getEye().resetEye();
  else if (action == "equipoffinvalid")
    pUser->getPackage().equipOffInValidEquip();
  else if (action == "resetgenderequip")
    eErr = pUser->getPackage().resetGenderEquip(static_cast<EGender>(params.getTableInt("gender")));
  else if (action == "quickfight")
  {
    xLuaData data;
    GMCommandRuler::killer(entry, data);
    GMCommandRuler::god(entry, data);

    data.setData("attrtype", EATTRTYPE_MOVESPD);
    data.setData("attrvalue", 2);
    GMCommandRuler::setattr(entry, data);
  }

  return eErr == EERROR_SUCCESS;
}

bool GMCommandRuler::modifycharge(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  stringstream ss;
  params.toJsonString(ss);
  if (ss.str().empty())
    return false;

  ModifyDepositSocialCmd cmd;
  cmd.set_charid(pUser->id);
  cmd.set_command(ss.str());

  PROTOBUF(cmd, send, len);
  thisServer->sendCmdToSession(send, len);

  XLOG << "[GM-修改充值数据]" << pUser->accid << pUser->id << pUser->name << "cmd:" << cmd.command() << "发送至session处理" << XEND;
  return true;
}

bool GMCommandRuler::scene_gear(Scene* pScene, const xLuaData& params)
{
  if (pScene == nullptr)
    return false;
  
  if (params.has("id") == false)
    return false;
  
  DWORD gearId = params.getTableInt("id");
  DWORD state = params.getTableInt("state");

  pScene->m_oGear.set(gearId, state, nullptr);
  
  XLOG <<"[场景-GM] 设置机关状态 场景mapid"<<pScene->getMapID() <<"gearid"<< gearId <<"state" << state << XEND;
  return true;
}

bool GMCommandRuler::scene_kickalluser(Scene* pScene, const xLuaData& params)
{
  if (!pScene)
    return false;
  pScene->kickAllUser(true);
  XLOG << "[场景-GM] 踢掉场景内的所有玩家 场景mapid" << pScene->getMapID() << XEND;
  return true;
}

bool GMCommandRuler::scene_shakecreen(Scene* pScene, const xLuaData& params)
{
  if (!pScene) return false;
  
  Cmd::ShakeScreen cmd;
  if (params.has("amplitude"))
    cmd.set_maxamplitude(params.getTableInt("amplitude"));
  if (params.has("msec"))
    cmd.set_msec(params.getTableInt("msec"));
  if (params.has("shaketype"))
    cmd.set_shaketype(params.getTableInt("shaketype"));

  PROTOBUF(cmd, send, len);
  pScene->sendCmdToAll(send, len);
 
  XLOG << "[场景-GM] 震屏 场景mapid" << pScene->getMapID() << XEND;
  return true;
}

bool GMCommandRuler::scene_addquest(Scene* pScene, const xLuaData& params)
{
  if(!pScene)
    return false;

  xLuaData data = params;
  if(!data.has("quests"))
    return false;

  TVecDWORD vecQuests;
  auto addquest = [&](const string& key, xLuaData& data)
  {
    vecQuests.push_back(data.getInt());
  };
  data.getMutableData("quests").foreach(addquest);

  if(0 >= vecQuests.size())
    return false;

  return pScene->addQuestsAllUser(vecQuests);
}

bool GMCommandRuler::scene_delquest(Scene* pScene, const xLuaData& params)
{
  if(!pScene)
    return false;

  xLuaData data = params;
  if(!data.has("quests"))
    return false;

  TVecDWORD vecQuests;
  auto addquest = [&](const string& key, xLuaData& data)
  {
    vecQuests.push_back(data.getInt());
  };
  data.getMutableData("quests").foreach(addquest);

  if(0 >= vecQuests.size())
    return false;

  return pScene->delQuestsAllUser(vecQuests);
}

bool GMCommandRuler::inviteteammates(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  std::set<SceneUser*> setUsers = pUser->getTeamSceneUserInPvpGvg();
  for(auto &user : setUsers)
  {
    if (user == nullptr || user->getScene() == nullptr)
      continue;

    InviteWithMeUserCmd cmd;
    cmd.set_sendid(pUser->id);
    DWORD dwOverTime = now() + MiscConfig::getMe().getTeamCFG().dwInviteTime;
    cmd.set_time(dwOverTime);

    char sign[1024];
    bzero(sign, sizeof(sign));
    std::stringstream ss;
    ss << pUser->id << "_" << user->id << "_" << dwOverTime << "_" << "#$%^&";
    upyun_md5(ss.str().c_str(), ss.str().size(), sign);

    cmd.set_sign(sign);

    PROTOBUF(cmd, send, len);
    user->sendCmdToMe(send, len);
  }

  return true;
}

bool GMCommandRuler::lockquota(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  QWORD num = params.getTableQWORD("num");
  if (num == 0)
    return false;
  DWORD type = params.getTableQWORD("type");

  pUser->getDeposit().lockQuota(num, type == 0 ? Cmd::EQuotaType_G_Charge : static_cast<Cmd::EQuotaType>(type));

  return true;
}

bool GMCommandRuler::unlockquota(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  QWORD num = params.getTableQWORD("num");
  if (num == 0)
    return false;
  DWORD type = params.getTableQWORD("type");

  pUser->getDeposit().unlockQuota(num, type == 0 ? Cmd::EQuotaType_G_Charge : static_cast<Cmd::EQuotaType>(type), params.getTableInt("sub") == 1);

  return true;
}

bool GMCommandRuler::rewardsafety(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if (params.getTableInt("clear") == 1)
  {
    DWORD id = params.getTableInt("id");
    if (pUser->getPackage().clearRewardSafetyItem(id))
      XLOG << "[GM-保底奖励]" << pUser->accid << pUser->id << pUser->name << "reward:" << id << "清除保底奖励记录成功";
    return true;
  }

  const RewardSafetyItem* item = pUser->getPackage().getRewardSafetyItem(params.getTableInt("id"));
  if (item == nullptr)
  {
    MsgManager::sendDebugMsg(pUser->id, "无保底");
    return true;
  }

  stringstream ss;
  ss << "rewardid:" << item->id() << " 摇奖次数:" << item->rollcount() << " 摇中次数:" << item->rewardcount() << " 下次保底次数:" << item->nextsafetycount() << " 版本号:" << item->version();

  MsgManager::sendDebugMsg(pUser->id, ss.str());

  return true;
}

bool GMCommandRuler::setshopgotcount(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  DWORD shopid = params.getTableInt("id");
  DWORD count = params.getTableInt("count");
  if (!shopid)
    return false;

  pUser->getSceneShop().setShopItemCount(shopid, count, true);
  XLOG << "[GM-设置商品购买次数]" << pUser->accid << pUser->id << pUser->name << "shopid:" << shopid << "count:" << count << "设置成功" << XEND;
  return true;
}

bool GMCommandRuler::activatetransfer(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  if(!params.has("npcId")){
    return false;
  }
  DWORD npcid = params.getTableInt("npcId");

  return pUser->getTransfer().activateTransfer(npcid);
}

bool GMCommandRuler::unlockpetwear(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);
  if (params.has("itemid") == false)
    return false;
  DWORD itemid = params.getTableInt("itemid");
  if (params.has("body"))
  {
    DWORD bodyid = params.getTableInt("body");

    EEquipPos epos = EEQUIPPOS_MIN;
    if (params.has("pos"))
    {
      DWORD pos = params.getTableInt("pos");
      if (EEquipPos_IsValid(pos) == false)
        return false;
      epos = static_cast<EEquipPos>(pos);
    }

    if (pUser->getUserPet().unlockWear(itemid, bodyid, epos) == false)
      return false;
  }
  else
  {
    if (pUser->getUserPet().unlockWear(itemid) == false)
      return false;
  }
  XLOG << "[GM-unlockpetwear], 解锁成功, 装备id:" << itemid << "玩家:" << pUser->name << pUser->id;
  return true;
}

bool GMCommandRuler::boss(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  const string& action = params.getTableString("action");

  if (action == "start")
  {
    DWORD dwMapID = params.getTableInt("mapid");
    DWORD dwBossID = params.getTableInt("bossid");
    DWORD dwActID = params.getTableInt("actid");
    DWORD dwLv = params.getTableInt("lv");

    if (MapConfig::getMe().getMapCFG(dwMapID) == nullptr)
    {
      XERR << "[GM指令-boss相关]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行action :" << action << "失败,mapid :" << dwMapID << "未在 Table_Map.txt 表中找到" << XEND;
      return false;
    }
    if (BossConfig::getMe().getBossCFG(dwBossID) == nullptr)
    {
      XERR << "[GM指令-boss相关]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行action :" << action << "失败,bossid :" << dwBossID << "未在 Table_Boss.txt 表中找到" << XEND;
      return false;
    }

    Cmd::SummonBossBossSCmd scmd;
    scmd.set_npcid(dwBossID);
    scmd.set_mapid(dwMapID);
    scmd.set_lv(dwLv);
    if (BossMgr::getMe().onSummonBoss(scmd, dwActID) == false)
    {
      XERR << "[GM指令-boss相关]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行action :" << action << "mapid :" << dwMapID << "bossid :" << dwBossID << "失败" << XEND;
      return false;
    }
  }
  else if (action == "clear")
  {
    BossMgr::getMe().clear();
  }
  else if (action == "open")
  {
    GET_USER(entry);

    const DeadBossInfo& rInfo = GlobalManager::getMe().getGlobalBoss();
    if (rInfo.charid() != 0)
    {
      XDBG << "[GM指令-boss相关]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "尝试开启世界boss功能被忽略,已有信息" << rInfo.ShortDebugString() << XEND;
      return true;
    }

    DeadBossOpenBossSCmd scmd;
    DeadBossInfo* pInfo = scmd.mutable_info();
    pInfo->set_charid(pUser->id);
    pInfo->set_zoneid(thisServer->getZoneID());
    pInfo->set_time(now());
    pInfo->set_name(pUser->name);

    PROTOBUF(scmd, ssend, slen);
    thisServer->sendCmdToSession(ssend, slen);

    XLOG << "[GM指令-boss相关]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "尝试开启世界boss功能" << XEND;
  }
  else if (action == "show")
  {
    if (pUser->getScene() == nullptr)
    {
      XERR << "[GM指令-boss相关]" << pUser->accid << pUser->id << pUser->getProfession() << pUser->name << "执行action :" << action << "失败,玩家未在正确的场景中" << XEND;
      return false;
    }
    MsgManager::sendDebugMsg(pUser->id, BossMgr::getMe().debugInfo(pUser->getScene()->getMapID()));
  }
  else if (action == "set")
  {
    BossSetBossSCmd scmd;
    PROTOBUF(scmd, ssend, slen);
    thisServer->sendCmdToSession(ssend, slen);
  }
  else
  {
    return false;
  }

  return true;
}


bool GMCommandRuler::addresist(xSceneEntryDynamic* entry, const xLuaData& params)
{
  GET_USER(entry);

  const SQuestMiscCFG& rCFG = MiscConfig::getMe().getQuestCFG();
  DWORD dwDailyTCount = pUser->getQuest().getDailyTCount();
  DWORD dwDailyCount = pUser->getQuest().getDailyCount();
  if(dwDailyTCount >= rCFG.dwMaxDailyCount + dwDailyCount)
    return false;

  DWORD dwTimes = params.getTableInt("count");
  pUser->getQuest().setDailyTCount(dwTimes + dwDailyTCount);
  DWORD dwAddTimes = pUser->getQuest().getDailyTCount() - dwDailyTCount;
  MsgManager::sendMsg(pUser->id, 26016, MsgParams(dwAddTimes));
  XLOG << "[GM-抗击魔潮] 添加次数" << dwAddTimes << "总次数 " << pUser->getQuest().getDailyTCount() << "完成次数 " << pUser->getQuest().getDailyCount() << XEND;
  return true;
}


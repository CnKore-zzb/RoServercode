--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--公会福利
ServerGame.Artifact= {
    retrieve_cd = 7200,                     --神器的回收时间2小时
	building = {
		[5] = {
    		produce_npc_id = 7820,            --打造成功说话npcid

    		produce_msg_id = 3790,           --打造成功说的msgid
		},
		[6] = {                                                                              
    		produce_npc_id = 7823,            --头饰神器的打造成功说话npcid
    		produce_msg_id = 3790,           --打造成功说的msgid
		},
	}
}	
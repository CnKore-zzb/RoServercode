import os
import sys

def Dir_Traversal(rootDir):
	for root,dirs,files in os.walk(rootDir):
		for file in files:
			file_path = os.path.join(file)
			file_path = file_path.split(".")[0]
			if file_path.find("Table_Quest_")!=-1 or file_path == "Table_Quest":
				print(file_path)
			for dir in dirs:
				Dir_Traversal(dir)

rootDir = sys.argv[0].replace("OutPutQuestFileNames.py", "") + "../Cehua/Lua/Table/"

Dir_Traversal(rootDir)
/*
 * SceneObjectTest.cpp
 *
 *  Created on: Jul 16, 2018
 *      Author: rick
 */

#include <memory>
#include "gtest/gtest.h"

#include "SceneServer/Scene.h"

class SceneOjectTest : public :: testing::Test
{
};

TEST(SceneObjectTest, getBornPoint)
{
	std::string str = ""
	"Root={"
	"	BornPoints={"
	"		{"
	"			ID=2,"
	"			position={"
	"				55.1910018920898,"
	"				4.59999990463257,"
	"				14.5089874267578"
	"			},"
	"			range=0"
	"		},"
	"	},"
	"}";

	xLuaData data;
	data.parseFromString(str, "Root");

	auto so = std::unique_ptr<SceneObject>(new SceneObject());
	so->load(data);

	auto res = so->getBornPoint(DWORD(2));

	ASSERT_NE(nullptr, res);
	ASSERT_NEAR(55.1910018920898, res->x, 1.0E20);
	ASSERT_NEAR(4.59999990463257, res->y, 1.0E20);
	ASSERT_NEAR(14.5089874267578, res->z, 1.0E20);
}







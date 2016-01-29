#include "GameScene.h"
#include "VisibleRect.h"
#include "CheckerboardLayer.h"
using namespace cocos2d;


Scene* GameScene::createScene()
{
    auto scene = Scene::create();
	auto layer = GameScene::create();
    scene->addChild(layer);
    return scene;
}

bool GameScene::init()
{
    if ( !Layer::init() )
    {
        return false;
    }

	auto background = Sprite::create("background.png");
	background->setPosition(VisibleRect::center());
	addChild(background);

	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/elements.plist");

	auto checkerboard = CheckerboardLayer::create();
	addChild(checkerboard);

	CheckerboardCache::Config config;
	CheckerboardCache::instance()->add_checkerboard_config("map/01.tmx");
	if (CheckerboardCache::instance()->get_checkerboard_config("map/01.tmx", config))
	{
		checkerboard->start_game(config);
	}

	return true;
}
#pragma once

#include "cocos2d.h"
#include "GameLogic.h"
#include "CheckerboardCache.h"

class CheckerboardLayer : public cocos2d::Layer
{
	static const int kElementWidth = 74;
	static const int kElementHeight = 73;

public:
	CheckerboardLayer();

	~CheckerboardLayer();

	virtual bool init() override;

	CREATE_FUNC(CheckerboardLayer);

public:
	/**
	 * 开始游戏
	 */
	void start_game(const CheckerboardCache::Config &config);

	/**
	 * 获取起点坐标
	 */
	cocos2d::Vec2 get_start_point() const;

	/**
	 * 转换到世界坐标
	 */
	cocos2d::Vec2 convert_to_world_pos(const GameLogic::Vec2 &pos) const;

	/**
	 * 转换到棋盘坐标
	 */
	GameLogic::Vec2 convert_to_checkerboard_pos(const cocos2d::Vec2 &position) const;

public:
	virtual bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

	virtual void onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event) override;

private:
	/**
	 * 更新动作
	 */
	void on_update_action();

	/**
	 * 执行动作
	 */
	void run_action_group();

	/**
	 * 消除元素
	 */
	void on_element_eliminate(const GameLogic::Vec2 &pos);

	/**
	 * 生成元素
	 */
	void on_element_generate(const GameLogic::Vec2 &pos, int type);

	/**
	 * 刷新棋盘
	 */
	void on_refresh_checkerboard(const GameLogic::Vec2 &pos, int type);

	/**
	 * 自动填充
	 */
	void on_element_autofill(const GameLogic::Vec2 &source, const GameLogic::Vec2 &target);

	/**
	 * 完成动作
	 */
	void on_finish_action();

private:
	/**
	 * 初始化地板
	 */
	void init_floors();

	/**
	 * 初始化元素
	 */
	void init_elements();

	/**
	 * 交换元素
	 */
	void swap_element(const GameLogic::Vec2 &a, const GameLogic::Vec2 &b);

	/**
	 * 完成交换元素
	 */
	void swap_element_finished(const GameLogic::Vec2 &a, const GameLogic::Vec2 &b);
	
private:
	bool										touch_lock_;
	unsigned int								handle_num_;
	GameLogic									logic_;
	std::vector<cocos2d::Sprite*>				floors_;
	std::map<GameLogic::Vec2, cocos2d::Sprite*>	elements_;
	std::vector<cocos2d::Sprite *>				free_elements_;
	cocos2d::SpriteBatchNode*					batch_node_;
	GameLogic::Vec2								previous_selected_;
};
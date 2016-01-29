#ifndef __GAMELOGIC_H__
#define __GAMELOGIC_H__

#include <queue>
#include <vector>
#include <random>
#include <functional>
#include "AStar/AStar.h"
#include "CheckerboardCache.h"

class GameLogic
{
public:
	/**
	 * 地板类型
	 */
	enum FloorType
	{
		NOTHING = -1,
		NOELEMENT = 0,
	};

	/**
	 * 动作类型枚举
	 */
	enum class ActionType
	{
		NONE,
		START,				// 开始
		MOVE,				// 移动
		REMOVE,				// 消除
		GENERATE,			// 生成
		AUTOFILL,			// 自动填充
	};

	/**
	 * 二维坐标
	 */
	struct Vec2
	{
		int x;
		int y;

		Vec2() : x(0), y(0) {}
		Vec2(int _x, int _y) : x(_x), y(_y) {}

		static Vec2 invalid()
		{
			return Vec2(-1, -1);
		}

		bool operator!= (const Vec2 &that) const
		{
			return x != that.x || y != that.y;
		}

		bool operator== (const Vec2 &that) const
		{
			return x == that.x && y == that.y;
		}

		bool operator< (const Vec2 &that) const
		{
			return y < that.y ? true : y == that.y ? x < that.x : false;
		}
	};

	/**
	 * 搜索范围
	 */
	struct SearchScope
	{
		int min_row;
		int min_col;
		int max_row;
		int max_col;
	};

	/**
	 * 移动路径
	 */
	struct MoveTrack
	{
		GameLogic::Vec2 source;
		GameLogic::Vec2 target;

		bool operator== (const MoveTrack &that) const
		{
			return source == that.source && target == that.target;
		}
	};

	/**
	 * 动作信息
	 */
	struct Action
	{
		ActionType		type;						// 动作类型
		int				element_type;				// 元素类型
		Vec2			source;						// 来源位置
		Vec2			target;						// 目标位置
	};

public:
	GameLogic();

	~GameLogic();

	/**
	 * 开始游戏
	 */
	void start_game(const CheckerboardCache::Config &config);

	/**
	 * 获取棋盘宽度
	 */
	int get_checkerboard_width() const;

	/**
	 * 获取棋盘高度
	 */
	int get_checkerboard_height() const;

	/**
	 * 获取动作组数量
	 */
	size_t get_action_group_num() const;

	/**
	 * 取出动作组信息
	 */
	std::vector<Action> take_action_group_from_queue();

	/**
	 * 添加动作更新回调
	 */
	void add_action_update_callback(std::function<void()> &&callback);

	/**
	 * 获取棋盘数据
	 */
	const std::vector<int>& get_checkerboard() const;

	/**
	 * 浏览棋盘
	 */
	void visit_checkerboard(const std::function<void(const Vec2&, int type)> &callback);

	/**
	 * 坐标转索引
	 * @return 失败返回-1
	 */
	int vec2_to_index(const Vec2 &pos) const;

	/**
	 * 是否在棋盘
	 */
	bool is_in_checkerboard(const Vec2 &pos) const;

	/**
	 * 元素是否有效
	 */
	bool is_valid_element(const Vec2 &pos) const;

	/**
	 * 是否相邻
	 */
	bool is_adjacent(const Vec2 &a, const Vec2 &b) const;

	/**
	 * 交换位置并消除
	 */
	bool swap_and_eliminate(const Vec2 &a, const Vec2 &b);

protected:
	GameLogic(const GameLogic&) = delete;
	GameLogic& operator= (const GameLogic&) = delete;

private:
	/**
	 * 初始化搜索范围
	 */
	void init_search_scope();

	/**
	 * 更新搜索范围
	 */
	void update_search_scope(const Vec2 &pos);

	/**
	 * 求最短距离
	 */
	int shortest_distance_to_top(const Vec2 &pos);

	/**
	 * 是否可消除
	 */
	std::set<Vec2> can_eliminate(const Vec2 &pos) const;

	/**
	 * 获取顶行
	 */
	int get_top_line() const;

	/**
	 * 生成顶行元素
	 * @return 生成数量
	 */
	std::set<Vec2> generate_element_to_top_line();

	/**
	 * 自动填充
	 * @return 填充数量
	 */
	std::set<Vec2> autofill_element();

	/**
	 * 消除元素
	 */
	void eliminate(const std::set<Vec2> &eliminate_set);

	/**
	 * 执行消除
	 */
	void run_eliminate(const std::set<Vec2> &eliminate_set);

	/**
	 * 添加动作组
	 */
	void add_action_group(std::vector<Action> &&action_group);

	/**
	 * 添加动作到组
	 */
	void add_action_to_group(ActionType type, int element_type, const Vec2 &source, const Vec2 &target, std::vector<Action> &ret_action_group);

private:
	AStar									a_star_;
	SearchScope								scope_;
	CheckerboardCache::Config				config_;
	std::default_random_engine				generator_;
	std::vector<int>						checkerboard_;
	std::queue< std::vector<Action> >		action_group_queue_;
	std::vector< std::function<void()> >	action_callback_list_;
};

#endif
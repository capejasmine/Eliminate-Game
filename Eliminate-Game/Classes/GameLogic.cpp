#include "GameLogic.h"

#include <ctime>
#include <limits>
#include <cassert>


GameLogic::GameLogic()
	: generator_(time(nullptr))
{

}

GameLogic::~GameLogic()
{

}

// 开始游戏
void GameLogic::start_game(const CheckerboardCache::Config &config)
{
	assert(config.type_num > 1);
	assert(config.width > 0 && config.height > 0);
	assert(config.width * config.height == config.layout.size());

	config_ = config;
	size_t max_size = config_.width * config_.height;
	std::uniform_int_distribution<int> dis(1, config.type_num);
	for (size_t idx = 0; idx < max_size; ++idx)
	{
		if (config.layout[idx] != 0)
		{
			checkerboard_.push_back(dis(generator_));
		}
		else
		{
			checkerboard_.push_back(FloorType::NOTHING);
		}
	}

	init_search_scope();
	std::vector<Action> action_group;
	add_action_to_group(ActionType::START, FloorType::NOTHING, Vec2::invalid(), Vec2::invalid(), action_group);
	add_action_group(std::move(action_group));
}

// 获取棋盘宽度
int GameLogic::get_checkerboard_width() const
{
	return config_.width;
}

// 获取棋盘高度
int GameLogic::get_checkerboard_height() const
{
	return config_.height;
}

// 获取动作组数量
size_t GameLogic::get_action_group_num() const
{
	return action_group_queue_.size();
}

// 取出动作组信息
std::vector<GameLogic::Action> GameLogic::take_action_group_from_queue()
{
	assert(!action_group_queue_.empty());
	std::vector<GameLogic::Action> ret;
	if (!action_group_queue_.empty())
	{
		ret = std::move(action_group_queue_.front());
		action_group_queue_.pop();
	}
	return ret;
}

// 添加动作更新回调
void GameLogic::add_action_update_callback(std::function<void()> &&callback)
{
	if (callback != nullptr)
	{
		action_callback_list_.push_back(callback);
	}
}

// 添加动作组
void GameLogic::add_action_group(std::vector<Action> &&action_group)
{
	if (!action_group.empty())
	{
		action_group_queue_.push(action_group);
		for (auto &func : action_callback_list_)
		{
			func();
		}
	}
}

// 添加动作到组
void GameLogic::add_action_to_group(ActionType type, int element_type, const Vec2 &source, const Vec2 &target, std::vector<Action> &ret_action_group)
{
	Action action;
	action.type = type;
	action.source = source;
	action.target = target;
	action.element_type = element_type;
	ret_action_group.push_back(action);
}

// 获取棋盘数据
const std::vector<int>& GameLogic::get_checkerboard() const
{
	return checkerboard_;
}

// 浏览棋盘
void GameLogic::visit_checkerboard(const std::function<void(const Vec2&, int type)> &callback)
{
	assert(callback != nullptr);
	if (callback != nullptr)
	{
		for (size_t i = 0; i < checkerboard_.size(); ++i)
		{
			callback(Vec2(i % config_.width, i / config_.width), checkerboard_[i]);
		}
	}
}

// 坐标转索引
int GameLogic::vec2_to_index(const Vec2 &pos) const
{
	if (!is_in_checkerboard(pos))
	{
		return -1;
	}
	return pos.y * config_.width + pos.x;
}

// 是否在棋盘
bool GameLogic::is_in_checkerboard(const Vec2 &pos) const
{
	return pos.x >= 0 && pos.x < config_.width && pos.y >= 0 && pos.y < config_.height;
}

// 元素是否有效
bool GameLogic::is_valid_element(const Vec2 &pos) const
{
	return is_in_checkerboard(pos) && checkerboard_[vec2_to_index(pos)] > FloorType::NOELEMENT;
}

// 是否相邻
bool GameLogic::is_adjacent(const Vec2 &a, const Vec2 &b) const
{
	if (is_in_checkerboard(a) && is_in_checkerboard(b))
	{
		return abs(a.y + a.x - b.y - b.x) == 1;
	}
	return false;
}

// 初始化搜索范围
void GameLogic::init_search_scope()
{
	scope_.min_col = scope_.min_row = std::numeric_limits<int>::max();
	scope_.max_col = scope_.max_row = std::numeric_limits<int>::min();
}

// 更新搜索范围
void GameLogic::update_search_scope(const Vec2 &pos)
{
	if (pos.x >= 0 && pos.x < config_.width)
	{
		if (pos.x <= scope_.min_col)
		{
			scope_.min_col = pos.x > 0 ? pos.x - 1 : pos.x;
		}

		if (pos.x >= scope_.max_col)
		{
			scope_.max_col = pos.x + 1 < config_.width ? pos.x + 1 : pos.x;
		}
	}
	else
	{
		assert(false);
	}

	if (pos.y >= 0 && pos.y < config_.height)
	{
		if (pos.y <= scope_.min_row)
		{
			scope_.min_row = pos.y > 0 ? pos.y - 1 : pos.y;
		}

		if (pos.y >= scope_.max_row)
		{
			scope_.max_row = pos.y + 1 < config_.height ? pos.y + 1 : pos.y;
		}
	}
	else
	{
		assert(false);
	}
}

// 求最短距离
int GameLogic::shortest_distance_to_top(const Vec2 &pos)
{
	std::vector<int> heap;
	for (int col = 0; col < config_.width; ++col)
	{
		if (config_.layout[col] != 0)
		{
			AStar::Param param;
			param.start.x = col;
			param.start.y = config_.height - 1;
			param.end.x = pos.x;
			param.end.y = pos.y;
			param.width = config_.width;
			param.height = config_.height;
			param.is_canreach = [&](const AStar::Vec2 &point)
			{
				return config_.layout[point.y * config_.width + point.x] != 0;
			};

			size_t distance = a_star_.search(param).size();
			if (distance > 0)
			{
				heap.push_back(distance);
				std::push_heap(heap.begin(), heap.end(), [](int a, int b)->bool
				{
					return a > b;
				});
			}
		}
	}
	return heap.size() > 0 ? heap[0] : std::numeric_limits<int>::max();
}

// 是否可消除
std::set<GameLogic::Vec2> GameLogic::can_eliminate(const Vec2 &pos) const
{
	std::set<Vec2> vertical;
	std::set<Vec2> horizontal;
	std::set<Vec2> eliminate_set;
	if (!is_valid_element(pos))
	{
		return eliminate_set;
	}
	const int type = checkerboard_[vec2_to_index(pos)];

	// 纵向遍历
	{
		for (int row = 0; row < config_.height; ++row)
		{
			const int idx = row * config_.width + pos.x;
			if (checkerboard_[idx] == type)
			{
				vertical.insert(Vec2(pos.x, row));
			}
			else
			{
				// 满足消除条件
				if (vertical.size() >= 3 && (vertical.find(pos) != vertical.end()))
				{
					break;
				}
				vertical.clear();
			}
		}
	}

	// 横向遍历
	{
		for (int col = 0; col < config_.width; ++col)
		{
			const int idx = pos.y * config_.width + col;
			if (checkerboard_[idx] == type)
			{
				horizontal.insert(Vec2(col, pos.y));
			}
			else
			{
				// 满足消除条件
				if (horizontal.size() >= 3 && (horizontal.find(pos) != horizontal.end()))
				{
					break;
				}
				horizontal.clear();
			}
		}
	}

	// 插入数据
	if (vertical.size() < 3 || vertical.find(pos) == vertical.end())
	{
		vertical.clear();
	}
	else
	{
		eliminate_set.insert(vertical.begin(), vertical.end());
	}

	if (horizontal.size() < 3 || horizontal.find(pos) == horizontal.end())
	{
		horizontal.clear();
	}
	else
	{
		eliminate_set.insert(horizontal.begin(), horizontal.end());
	}

	return eliminate_set;
}

// 获取顶行
int GameLogic::get_top_line() const
{
	for (int row = config_.height - 1; row >= 0; --row)
	{
		for (int col = 0; col < config_.width; ++col)
		{
			int idx = row * config_.height + col;
			if (config_.layout[idx] != 0)
			{
				return row;
			}
		}
	}
	return -1;
}

// 生成顶行元素
std::set<GameLogic::Vec2> GameLogic::generate_element_to_top_line()
{
	int num = 0;
	int row = 0;
	std::set<Vec2> ret;
	const size_t max_size = config_.width * config_.height;
	std::uniform_int_distribution<int> dis(1, config_.type_num);
	if ((row = get_top_line()) > 0)
	{
		std::vector<Action> action_group;
		for (int col = 0; col < config_.width; ++col)
		{
			int idx = row * config_.height + col;
			if (checkerboard_[idx] == FloorType::NOELEMENT)
			{
				ret.insert(Vec2(col, row));
				int type = dis(generator_);
				checkerboard_[idx] = type;
				add_action_to_group(ActionType::GENERATE, type, Vec2(col, row), Vec2::invalid(), action_group);
			}
		}
		if (!action_group.empty())
		{
			num = action_group.size();
			add_action_group(std::move(action_group));
		}
	}
	return ret;
}

// 自动填充
std::set<GameLogic::Vec2> GameLogic::autofill_element()
{
	// 生成元素
	std::set<Vec2> ret = generate_element_to_top_line();

	// 填充空位
	std::set<Vec2> moved_set;
	size_t last_move_set_size = 0;
	std::vector<Action> action_group;
	const int top_line = get_top_line();
	do
	{
		last_move_set_size = moved_set.size();
		for (int row = scope_.min_row; row <= scope_.max_row; ++row)
		{
			for (int col = scope_.min_col; col <= scope_.max_col; ++col)
			{
				const int idx = vec2_to_index(Vec2(col, row));
				if (checkerboard_[idx] > FloorType::NOELEMENT && moved_set.find(Vec2(col, row)) == moved_set.end())
				{
					// 向下移动
					if (row - 1 >= 0 &&
						checkerboard_[vec2_to_index(Vec2(col, row - 1))] == FloorType::NOELEMENT)
					{
						moved_set.insert(Vec2(col, row - 1));
						add_action_to_group(ActionType::AUTOFILL, checkerboard_[idx], Vec2(col, row), Vec2(col, row - 1), action_group);
						std::swap(checkerboard_[idx], checkerboard_[vec2_to_index(Vec2(col, row - 1))]);
						continue;
					}
					// 横向移动
					else if (top_line > row)
					{
						// 左移
						if (col - 1 >= 0 &&
							config_.layout[vec2_to_index(Vec2(col - 1, row + 1))] == 0 &&
							checkerboard_[vec2_to_index(Vec2(col - 1, row))] == FloorType::NOELEMENT)
						{
							if (col - 2 >= 0 && config_.layout[vec2_to_index(Vec2(col - 2, row + 1))] != 0)
							{
								if (shortest_distance_to_top(Vec2(col, row)) <= shortest_distance_to_top(Vec2(col - 2, row)))
								{
									moved_set.insert(Vec2(col - 1, row));
									add_action_to_group(ActionType::AUTOFILL, checkerboard_[idx], Vec2(col, row), Vec2(col - 1, row), action_group);
									std::swap(checkerboard_[idx], checkerboard_[vec2_to_index(Vec2(col - 1, row))]);
									continue;
								}
								else if (checkerboard_[vec2_to_index(Vec2(col - 2, row))] > FloorType::NOELEMENT &&
										 moved_set.find(Vec2(col - 2, row)) == moved_set.end())
								{
									moved_set.insert(Vec2(col - 1, row));
									add_action_to_group(ActionType::AUTOFILL, checkerboard_[vec2_to_index(Vec2(col - 2, row))], Vec2(col - 2, row), Vec2(col - 1, row), action_group);
									std::swap(checkerboard_[vec2_to_index(Vec2(col - 2, row))], checkerboard_[vec2_to_index(Vec2(col - 1, row))]);
									continue;
								}
							}
							else
							{
								moved_set.insert(Vec2(col - 1, row));
								add_action_to_group(ActionType::AUTOFILL, checkerboard_[idx], Vec2(col, row), Vec2(col - 1, row), action_group);
								std::swap(checkerboard_[idx], checkerboard_[vec2_to_index(Vec2(col - 1, row))]);
								continue;
							}
						}

						// 右移
						if (col + 1 < config_.width &&
							config_.layout[vec2_to_index(Vec2(col + 1, row + 1))] == 0 &&
							checkerboard_[vec2_to_index(Vec2(col + 1, row))] == FloorType::NOELEMENT)
						{
							if (col + 2 < config_.width && config_.layout[vec2_to_index(Vec2(col + 2, row + 1))] != 0)
							{
								if (shortest_distance_to_top(Vec2(col, row)) <= shortest_distance_to_top(Vec2(col + 2, row)))
								{
									moved_set.insert(Vec2(col + 1, row));
									add_action_to_group(ActionType::AUTOFILL, checkerboard_[idx], Vec2(col, row), Vec2(col + 1, row), action_group);
									std::swap(checkerboard_[idx], checkerboard_[vec2_to_index(Vec2(col + 1, row))]);
									continue;
								}
								else if (checkerboard_[vec2_to_index(Vec2(col + 2, row))] > FloorType::NOELEMENT &&
										 moved_set.find(Vec2(col + 2, row)) == moved_set.end())
								{
									moved_set.insert(Vec2(col + 1, row));
									add_action_to_group(ActionType::AUTOFILL, checkerboard_[vec2_to_index(Vec2(col + 2, row))], Vec2(col + 2, row), Vec2(col + 1, row), action_group);
									std::swap(checkerboard_[vec2_to_index(Vec2(col + 2, row))], checkerboard_[vec2_to_index(Vec2(col + 1, row))]);
									continue;
								}
							}
							else
							{
								moved_set.insert(Vec2(col + 1, row));
								add_action_to_group(ActionType::AUTOFILL, checkerboard_[idx], Vec2(col, row), Vec2(col + 1, row), action_group);
								std::swap(checkerboard_[idx], checkerboard_[vec2_to_index(Vec2(col + 1, row))]);
								continue;
							}
						}
					}
				}
			}
		}

	} while (moved_set.size() > last_move_set_size);

	// 记录被补充的位置
	ret.insert(moved_set.begin(), moved_set.end());

	// 更新搜索范围
	for (auto &item : action_group)
	{
		update_search_scope(item.source);
	}

	// 添加动作组
	if (!action_group.empty())
	{
		add_action_group(std::move(action_group));
	}

	return ret;
}

// 消除元素
void GameLogic::eliminate(const std::set<Vec2> &eliminate_set)
{
	std::vector<Action> action_group;
	for (auto &pos : eliminate_set)
	{
		if (is_valid_element(pos))
		{
			update_search_scope(pos);
			add_action_to_group(ActionType::REMOVE, checkerboard_[vec2_to_index(pos)], pos, Vec2::invalid(), action_group);
			checkerboard_[vec2_to_index(pos)] = FloorType::NOELEMENT;
		}
		else
		{
			assert(false);
		}
	}
	add_action_group(std::move(action_group));
}

// 执行消除
void GameLogic::run_eliminate(const std::set<Vec2> &eliminate_set)
{
	// 消除元素
	eliminate(eliminate_set);

	// 循环填充，直到无法消除
	std::set<Vec2> all_moved_set;
	std::set<Vec2> new_eliminate_set;
	while (true)
	{
		// 填充所有空位
		std::set<Vec2> moved_set;
		do
		{
			moved_set = autofill_element();
			all_moved_set.insert(moved_set.begin(), moved_set.end());
		} while (!moved_set.empty());

		// 初始化搜索范围
		init_search_scope();

		// 检查填充后是否可消除
		for (auto &pos : all_moved_set)
		{
			auto ret = can_eliminate(pos);
			new_eliminate_set.insert(ret.begin(), ret.end());
		}
		all_moved_set.clear();

		// 无法消除
		if (new_eliminate_set.empty())
		{
			break;
		}

		// 消除元素
		eliminate(new_eliminate_set);
		new_eliminate_set.clear();
	}
}

// 交换位置并消除
bool GameLogic::swap_and_eliminate(const Vec2 &a, const Vec2 &b)
{
	// 两者是否是有效的元素
	if (!is_valid_element(a) || !is_valid_element(b))
	{
		return false;
	}

	// 交换元素
	std::swap(checkerboard_[vec2_to_index(a)], checkerboard_[vec2_to_index(b)]);
	{
		std::vector<Action> action_group;
		add_action_to_group(ActionType::MOVE, checkerboard_[vec2_to_index(b)], a, b, action_group);
		add_action_to_group(ActionType::MOVE, checkerboard_[vec2_to_index(a)], b, a, action_group);
		add_action_group(std::move(action_group));
	}

	// 检测是否可消除
	std::set<Vec2> eliminate_set;
	{
		std::set<Vec2> temp = can_eliminate(a);
		eliminate_set.insert(temp.begin(), temp.end());
		temp = can_eliminate(b);
		eliminate_set.insert(temp.begin(), temp.end());
	}

	if (eliminate_set.empty())
	{
		// 交换失败，恢复原状
		std::swap(checkerboard_[vec2_to_index(a)], checkerboard_[vec2_to_index(b)]);
		{
			std::vector<Action> action_group;
			add_action_to_group(ActionType::MOVE, checkerboard_[vec2_to_index(b)], a, b, action_group);
			add_action_to_group(ActionType::MOVE, checkerboard_[vec2_to_index(a)], b, a, action_group);
			add_action_group(std::move(action_group));
		}
		return false;
	}
	else
	{
		assert(eliminate_set.size() >= 3);
	}

	// 消除元素
	run_eliminate(eliminate_set);

	return true;
}
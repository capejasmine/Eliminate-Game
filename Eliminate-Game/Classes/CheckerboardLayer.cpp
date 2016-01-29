#include "CheckerboardLayer.h"

#include "Element.h"
#include "VisibleRect.h"
using namespace cocos2d;


CheckerboardLayer::CheckerboardLayer()
	: handle_num_(0)
	, touch_lock_(false)
	, batch_node_(nullptr)
	, previous_selected_(GameLogic::Vec2::invalid())
{
	
}

CheckerboardLayer::~CheckerboardLayer()
{

}

bool CheckerboardLayer::init()
{
	if (!Layer::init())
	{
		return false;
	}

	batch_node_ = SpriteBatchNode::create("img/elements.png");
	addChild(batch_node_);

	// 添加触摸事件
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(false);
	listener->onTouchBegan = CC_CALLBACK_2(CheckerboardLayer::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(CheckerboardLayer::onTouchMoved, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	// 游戏逻辑更新通知
	logic_.add_action_update_callback(std::bind(&CheckerboardLayer::on_update_action, this));

	return true;
}

// 初始化地板
void CheckerboardLayer::init_floors()
{
	for (auto floor_ptr : floors_)
	{
		floor_ptr->setVisible(false);
	}
}

// 初始化元素
void CheckerboardLayer::init_elements()
{
	for (auto elment : elements_)
	{
		elment.second->setVisible(false);
		free_elements_.push_back(elment.second);
	}
	elements_.clear();
}

// 开始游戏
void CheckerboardLayer::start_game(const CheckerboardCache::Config &config)
{
	init_floors();
	init_elements();
	handle_num_ = 0;
	logic_.start_game(config);
}

// 获取起点坐标
inline Vec2 CheckerboardLayer::get_start_point() const
{
	return VisibleRect::center() - Vec2((kElementWidth * logic_.get_checkerboard_width()) / 2, (kElementHeight * logic_.get_checkerboard_height()) / 2);
}

// 转换到世界坐标
Vec2 CheckerboardLayer::convert_to_world_pos(const GameLogic::Vec2 &pos) const
{
	Vec2 start_point = get_start_point();
	return Vec2(start_point.x + kElementWidth / 2 + kElementWidth * pos.x, start_point.y + kElementHeight / 2 + kElementHeight * pos.y);
}

// 转换到棋盘坐标
GameLogic::Vec2 CheckerboardLayer::convert_to_checkerboard_pos(const cocos2d::Vec2 &position) const
{
	Vec2 start_point = get_start_point();
	for (int row = 0; row < logic_.get_checkerboard_height(); ++row)
	{
		for (int col = 0; col < logic_.get_checkerboard_width(); ++col)
		{
			float frist_col = start_point.x + col * kElementWidth;
			float last_col = start_point.x + (col + 1) * kElementWidth;
			float frist_row = start_point.y + row * kElementHeight;
			float last_row = start_point.y + (row + 1) * kElementHeight;
			if (position.x >= frist_col && position.x < last_col && position.y >= frist_row && position.y < last_row)
			{
				return GameLogic::Vec2(col, row);
			}
		}
	}
	return GameLogic::Vec2::invalid();
}

// 更新动作
void CheckerboardLayer::on_update_action()
{
	assert(logic_.get_action_group_num() > 0);
	run_action_group();
}

// 执行动作
void CheckerboardLayer::run_action_group()
{
	if (handle_num_ == 0 && logic_.get_action_group_num() > 0)
	{
		std::vector<GameLogic::Action> action_group = logic_.take_action_group_from_queue();
		for (size_t i = 0; i < action_group.size(); ++i)
		{
			switch (action_group[i].type)
			{
				// 开始游戏
				case GameLogic::ActionType::START:
				{
					logic_.visit_checkerboard(std::bind(&CheckerboardLayer::on_refresh_checkerboard, this, std::placeholders::_1, std::placeholders::_2));
					break;
				}
				// 移动元素
				case GameLogic::ActionType::MOVE:
				{
					on_refresh_checkerboard(action_group[i].target, action_group[i].element_type);
					break;
				}
				// 生成元素
				case GameLogic::ActionType::GENERATE:
				{
					on_element_generate(action_group[i].source, action_group[i].element_type);
					break;
				}
				// 消除元素
				case GameLogic::ActionType::REMOVE:
				{
					on_element_eliminate(action_group[i].source);
					break;
				}
				// 元素填充
				case GameLogic::ActionType::AUTOFILL:
				{
					on_element_autofill(action_group[i].source, action_group[i].target);
					break;
				}
			}
		}
	}
}

// 刷新棋盘
void CheckerboardLayer::on_refresh_checkerboard(const GameLogic::Vec2 &pos, int type)
{
	// 更新地板显示
	{
		size_t idx = pos.y * logic_.get_checkerboard_width() + pos.x;
		while (idx >= floors_.size())
		{
			auto element = Sprite::createWithSpriteFrameName("gs_el_floor.png");
			element->setVisible(false);
			batch_node_->addChild(element);
			floors_.push_back(element);
		}

		auto element = floors_[idx];
		element->setPosition(convert_to_world_pos(pos));
		element->setVisible(type != GameLogic::FloorType::NOTHING);
		element->setLocalZOrder(0);
	}

	// 更新元素显示
	{
		if (type > GameLogic::FloorType::NOELEMENT)
		{
			char buffer[128];
			Sprite *element = nullptr;
			sprintf(buffer, "gs_el_%02d.png", type);

			auto itr = elements_.find(pos);
			if (itr != elements_.end())
			{
				element = (*itr).second;
			}
			else
			{
				if (!free_elements_.empty())
				{
					element = free_elements_.back();
					element->setSpriteFrame(buffer);
					free_elements_.pop_back();
				}
				else
				{
					element = Element::createWithSpriteFrameName(buffer);
					batch_node_->addChild(element);
				}
			}
			element->setVisible(true);
			element->setLocalZOrder(1);
			element->setPosition(convert_to_world_pos(pos));
			elements_.insert(std::make_pair(pos, element));
		}
	}
}

// 消除元素
void CheckerboardLayer::on_element_eliminate(const GameLogic::Vec2 &pos)
{
	auto itr = elements_.find(pos);
	CCAssert(itr != elements_.end(), "");
	if (itr != elements_.end())
	{
		Element *element = dynamic_cast<Element *>(itr->second);
		CCAssert(element != nullptr, "");
		if (element != nullptr)
		{
			++handle_num_;
			touch_lock_ = true;
			elements_.erase(pos);
			free_elements_.push_back(element);
			element->eliminate(std::bind(&CheckerboardLayer::on_finish_action, this));
		}
	}
}

// 生成元素
void CheckerboardLayer::on_element_generate(const GameLogic::Vec2 &pos, int type)
{
	auto itr = elements_.find(pos);
	CCAssert(itr == elements_.end(), "");
	if (itr == elements_.end())
	{
		++handle_num_;
		touch_lock_ = true;
		on_refresh_checkerboard(pos, type);
		runAction(CallFunc::create(std::bind(&CheckerboardLayer::on_finish_action, this)));
	}
}

// 自动填充
void CheckerboardLayer::on_element_autofill(const GameLogic::Vec2 &source, const GameLogic::Vec2 &target)
{
	auto itr = elements_.find(source);
	CCAssert(itr != elements_.end(), "");
	if (itr != elements_.end())
	{
		Element *element = dynamic_cast<Element *>(itr->second);
		CCAssert(element != nullptr, "");
		if (element != nullptr)
		{
			++handle_num_;
			touch_lock_ = true;
			elements_.erase(source);
			elements_[target] = element;
			element->auto_fill(0.1f, convert_to_world_pos(target), std::bind(&CheckerboardLayer::on_finish_action, this));
		}
	}
}

// 完成动作
void CheckerboardLayer::on_finish_action()
{
	assert(handle_num_ > 0);
	if (--handle_num_ == 0)
	{
		// 下一波动作
		touch_lock_ = false;
		run_action_group();
	}
}

// 交换元素
void CheckerboardLayer::swap_element(const GameLogic::Vec2 &a, const GameLogic::Vec2 &b)
{
	Element *current_element = dynamic_cast<Element *>((*elements_.find(a)).second);
	Element *previous_element = dynamic_cast<Element *>((*elements_.find(b)).second);
	assert(current_element != nullptr && previous_element != nullptr);

	elements_[a] = previous_element;
	elements_[b] = current_element;
	auto inside = [=]()->void
	{
		static int count = 0;
		if (++count == 2)
		{
			count = 0;
			swap_element_finished(a, b);
		}
	};
	current_element->move(0.25f, previous_element->getPosition(), inside);
	previous_element->move(0.25f, current_element->getPosition(), inside);
}

// 完成交换元素
void CheckerboardLayer::swap_element_finished(const GameLogic::Vec2 &a, const GameLogic::Vec2 &b)
{
	if (!logic_.swap_and_eliminate(a, b))
	{
		Element *current_element = dynamic_cast<Element *>((*elements_.find(a)).second);
		Element *previous_element = dynamic_cast<Element *>((*elements_.find(b)).second);
		assert(current_element != nullptr && previous_element != nullptr);

		elements_[a] = previous_element;
		elements_[b] = current_element;

		auto inside = [=]()->void
		{
			static int count = 0;
			if (++count == 2)
			{
				count = 0;
				touch_lock_ = false;
			}
		};
		current_element->move(0.25f, previous_element->getPosition(), inside);
		previous_element->move(0.25f, current_element->getPosition(), inside);
	}
}

bool CheckerboardLayer::onTouchBegan(Touch *touch, Event *event)
{
	return true;
}

void CheckerboardLayer::onTouchMoved(Touch *touch, Event *event)
{
	if (touch_lock_ || elements_.empty())
	{
		return;
	}

	GameLogic::Vec2 current_selected = convert_to_checkerboard_pos(touch->getLocation());
	auto itr = elements_.find(current_selected);
	if (itr != elements_.end() && itr->second != nullptr)
	{
		if (previous_selected_ != GameLogic::Vec2::invalid())
		{
			if (previous_selected_ != current_selected)
			{
				if (logic_.is_adjacent(current_selected, previous_selected_))
				{
					touch_lock_ = true;
					swap_element(current_selected, previous_selected_);
					previous_selected_ = GameLogic::Vec2::invalid();
				}
			}
		}
		else
		{
			previous_selected_ = current_selected;
		}
	}
	else
	{
		previous_selected_ = GameLogic::Vec2::invalid();
	}
}
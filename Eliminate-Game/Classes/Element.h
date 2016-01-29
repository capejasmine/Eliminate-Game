#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "cocos2d.h"

class Element final : public cocos2d::Sprite
{
public:
	static Element* create();
	static Element* create(const std::string& filename);
	static Element* create(const std::string& filename, const cocos2d::Rect& rect);
	static Element* createWithTexture(cocos2d::Texture2D *texture);
	static Element* createWithTexture(cocos2d::Texture2D *texture, const cocos2d::Rect& rect, bool rotated = false);
	static Element* createWithSpriteFrame(cocos2d::SpriteFrame *spriteFrame);
	static Element* createWithSpriteFrameName(const std::string& spriteFrameName);

public:
	/**
	 * 是否锁定
	 */
	bool is_locked() const;

	/**
	 * 移动
	 */
	void move(float duration, const cocos2d::Vec2& pos, const std::function<void()> &callback);

	/**
	 * 自动填充
	 */
	void auto_fill(float duration, const cocos2d::Vec2& pos, const std::function<void()> &callback);

	/**
	 * 消除
	 */
	void eliminate(const std::function<void()> &callback);

private:
	Element() = default;
	~Element() = default;
};

#endif
#ifndef __CHECKERBOARDCACHE_H__
#define __CHECKERBOARDCACHE_H__

#include <unordered_map>
#include "Singleton.h"

class CheckerboardCache : public Singleton < CheckerboardCache >
{
	SINGLETON(CheckerboardCache);

public:
	struct Config
	{
		int					height;		// 棋盘高度
		int					width;		// 棋盘宽度
		int					type_num;	// 类型数量
		std::vector<bool>	layout;		// 棋盘布局
	};

public:
	/**
	 * 添加棋盘配置
	 */
	void add_checkerboard_config(const std::string &filename);

	/**
	 * 获取棋盘配置
	 */
	bool get_checkerboard_config(const std::string &filename, Config &ret) const;

private:
	std::unordered_map<std::string, Config> checkerboard_;
};

#endif
#include "CheckerboardCache.h"

#include "cocos2d.h"
#include "json/document.h"
using namespace cocos2d;


CheckerboardCache::CheckerboardCache()
{

}

CheckerboardCache::~CheckerboardCache()
{

}

// 添加棋盘配置
void CheckerboardCache::add_checkerboard_config(const std::string &filename)
{
	auto itr = checkerboard_.find(filename);
	if (itr == checkerboard_.end())
	{
		Data data = FileUtils::getInstance()->getDataFromFile(filename);
		if (!data.isNull())
		{
			rapidjson::Document doc;
			doc.Parse<0>(std::string((const char *)data.getBytes(), data.getSize()).c_str());

			auto map_info = TMXMapInfo::create(filename);
			auto layers = map_info->getLayers();
			CCAssert(!layers.empty(), "");
			auto tiles = layers.front()->_tiles;
			auto layer_size = layers.front()->_layerSize;

			Config config;
			config.width = layer_size.width;
			config.height = layer_size.height;
			config.layout.resize(config.width * config.height);
			config.type_num = atoi(layers.front()->_name.c_str());

			// 坐标系转换
			for (int row = 0; row < config.height; ++row)
			{
				for (int col = 0; col < config.width; ++col)
				{
					config.layout[row * config.width + col] = tiles[(config.height - row - 1) * config.width + col] != 0;
				}
			}

			checkerboard_.insert(std::make_pair(filename, config));
		}
	}
}

// 获取棋盘配置
bool CheckerboardCache::get_checkerboard_config(const std::string &filename, Config &ret) const
{
	auto itr = checkerboard_.find(filename);
	if (itr != checkerboard_.end())
	{
		ret = itr->second;
	}
	return itr == checkerboard_.end() ? false : true;
}
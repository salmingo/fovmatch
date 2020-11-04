/**
 * @file ParamMatchShape.h
 * @brief 定义: 模型构建/匹配/拟合参数及文件访问接口
 * @note
 * - 文件以xml格式存储
 * - 基于boost::property_tree
 */

#ifndef _PARAM_MATCHSHAPE_H_
#define _PARAM_MATCHSHAPE_H_

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <string>
#include <string.h>

using namespace boost::property_tree;

/*!
 * @struct ParamMatchShape
 * @brief 定义: 模型匹配算法中, 构建/匹配/拟合模型的参数, 以及其存储文件的访问接口
 */
struct ParamMatchShape {
	/*------------- 参数: 模型匹配 -------------*/
	/*!
	 * @brief 两个模型坐标系的映射关系
	 * - -1: 旋转方向相同
	 * - +1: 旋转方向相反, 即沿X或Y轴镜像
	 * -  0: 不确定映射关系
	 */
	int parity;
	/*!
	 * @brief 像元张角下限, 量纲: 角秒/像元
	 * - 有效范围: [0.1, 324.0]
	  \verbatim
	  if scale_low < 0.1
	  	  scale_low = 0.1
	  elseif scale_low > 324.0
	  	  scale_low = 324.0
	  endif
	  \endverbatim
	 */
	double scale_low;
	/*!
	 * @brief 像元张角上限, 量纲: 角秒/像元
	 * - 有效范围: [0.1, 324.0]
	  \verbatim
	  if scale_high < 0.1
	  	  scale_high = 0.1
	  elseif scale_high > 324.0
	  	  scale_high = 324.0
	  endif
	  \endverbatim
	 */
	double scale_high;
	/*!
	 * @brief 星表文件路径
	 */
	std::string pathcat;

	/*------------- 参数: 输出结果 -------------*/
	/*!
	 * @brief 处理过程是否在标准输出设备打印
	 */
	bool use_stdprint;
	/*!
	 * @brief 使用结果输出目录
	 * - 启用该功能时, 处理结果输出至指定目录
	 * - 不启用该功能时, 处理结果输出至原始数据目录
	 */
	bool use_output_dir;
	/*!
	 * @brief 结果输出目录
	 */
	std::string output_dir;

protected:
	char errmsg[256];

protected:
	/*!
	 * @brief 创建缺省值文件
	 * @param filepath  文件路径
	 * @return
	 * 文件创建结果
	 */
	bool init_file(const char* filepath) {
		ptree pt;

		/* 参数: 模型匹配 */
		pt.add("ShapeMatch.<xmlattr>.parity", 0);
		pt.add("Scale.<xmlattr>.low",  8.3);
		pt.add("Scale.<xmlattr>.high", 8.5);
		pt.add("Catalog.<xmlattr>.pathname", "/data/catalog");

		/* 参数: 输出结果*/
		pt.add("StdPrint.<xmlattr>.use",    true);
		pt.add("Output.<xmlattr>.use",      false);
		pt.add("Output.<xmlattr>.pathname", "");

		try {
			xml_writer_settings<std::string> settings(' ', 4);
			write_xml(filepath, pt, std::locale(), settings);
			return true;
		}
		catch(xml_parser_error& ex) {
			strcpy (errmsg, ex.what());
			return false;
		}
	}

public:
	/*!
	 * @brief 加载约束参数
	 * @param filepath  文件路径
	 * @return
	 * 文件访问及参数加载结果.
	 */
	bool Load(const char* filepath) {
		try {
			ptree pt;
			read_xml(filepath, pt, boost::property_tree::xml_parser::trim_whitespace);

			/* 参数: 模型匹配 */
			parity     = pt.get("ShapeMatch.<xmlattr>.parity", 0);
			scale_low  = pt.get("Scale.<xmlattr>.low",         0.1);
			scale_high = pt.get("Scale.<xmlattr>.high",        324.0);
			pathcat    = pt.get("Catalog.<xmlattr>.pathname",  "");

			/* 参数: 输出结果*/
			use_stdprint   = pt.get("StdPrint.<xmlattr>.use",    true);
			use_output_dir = pt.get("Output.<xmlattr>.use",      false);
			output_dir     = pt.get("Output.<xmlattr>.pathname", "");

			return true;
		}
		catch(xml_parser_error& ex) {
			return init_file(filepath);
		}
	}

	/*!
	 * @brief 查看错误提示
	 * @return
	 * 错误提示
	 */
	const char* GetErrmsg() {
		return errmsg;
	}
};

#endif

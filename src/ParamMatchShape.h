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
	/*------------- 参数: 构建模型 -------------*/
	/*!
	 * @brief 参与构建模型样本数量的最大值
	 * @li 有效范围: [10, infinite)
	 * - 缺省值: 10
	 * - 超出范围时采用缺省值
	 */
	int count_sample_max_;
	/*!
	 * @brief 模型中样本数量的最小值
	 * - 不含中心和定向两点
	 * - 有效范围: [1, infinite)
	 */
	int count_per_shape_min_;
	/*!
	 * @brief 顶角, 量纲: 角度
	 * - 有效范围: (20, 340)
	 * - 超出该范围时, 由主程序自动判定顶角
	 */
	double apex_angle_;
	/*!
	 * @brief 样本与中心点的距离, 与坐标系维度长度的比值的最小值
	 * - 有效范围: [0.01, 0.2]
	 * - 缺省值: 0.1
	 * - 超出范围时, 采用缺省值
	 */
	double distance_min_;

	/*------------- 参数: 模型匹配 -------------*/


	/*------------- 参数: 模型拟合 -------------*/

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

		/* 参数: 构建模型 */
		pt.put("SampleCount.<xmlattr>.max",    count_sample_max_);
		pt.put("CountPerShape.<xmlattr>.min",  count_per_shape_min_);
		pt.put("Apex.<xmlattr>.angle",         apex_angle_);
		pt.put("DistanceRatio.<xmlattr>.min",  distance_min_);

		/* 参数: 模型匹配 */

		/* 参数: 模型拟合 */

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

			/* 参数: 构建模型 */
			count_sample_max_    = pt.get("SampleCount.<xmlattr>.max",   30);
			count_per_shape_min_ = pt.get("CountPerShape.<xmlattr>.min", 2);
			apex_angle_          = pt.get("Apex.<xmlattr>.angle",        60.0);
			distance_min_        = pt.get("DistanceRatio.<xmlattr>.min", 0.1);

			/* 参数: 模型匹配 */

			/* 参数: 模型拟合 */

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

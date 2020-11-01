/**
 * @file MatchShape.h
 * @brief 定义: 用于匹配的模型
 */

#ifndef _MATCHSHAPE_H_
#define _MATCHSHAPE_H_

#include <vector>

struct MatchShape {
	/*!
	 * @struct Point
	 * @brief 定义: 匹配模型内的样本提取特征
	 */
	struct Point {
		int id;				///< 样本ID
		double len_normal;	///< 归算长度. |id - idc|/len
		double incl_normal;	///< 归算倾角, 量纲: 角度. incl(id - idc)-incl
	};

	int idc;	///< 中心ID
	int ido;	///< 指向ID
	double len;	///< 距离: 指向-中心
	double incl;///< 倾角: 指向-中心相对'X'轴的倾角, 量纲: 角度
	std::vector<Point> pts;

public:
	/*!
	 * @brief 析构函数
	 */
	virtual ~MatchShape() {
		pts.clear();
	}

	/*!
	 * @brief 新增一个样本
	 * @param id    样本ID
	 * @param len   长度, |id - idc|
	 * @param incl  倾角, 量纲: 角度. incl(id - idc)
	 */
	void AddPoint(int id, double len, double incl) {
		Point pt;
		pt.id = id;
		pt.len_normal  = len / this->len;
		pt.incl_normal = incl - this->incl;
		pts.push_back(pt);
	}

	/*!
	 * @brief 清空匹配模型内的样本
	 */
	void Reset() {
		pts.clear();
	}

	/*!
	 * @brief 检查模型内是否有可用样本
	 * @return
	 * 检查结果
	 */
	bool IsEmpty() {
		return pts.size() == 0;
	}

	/*!
	 * @brief 查看模型内可用样本的数量
	 * @return
	 * 可用样本数量
	 * @note
	 * 不包含中心和指向
	 */
	int Count() {
		return pts.size();
	}
};
typedef std::vector<MatchShape> MatchShapeVec;

#endif

/**
 * @file MatchingShapePoint.h
 * @brief 定义: 参与模型匹配的样本
 */

#ifndef _MATCHINGSHAPE_POINT_H_
#define _MATCHINGSHAPE_POINT_H_

#include <string.h>
#include <math.h>
#include <vector>
#include <algorithm>

/*!
 * @struct MatchingShapePoint
 * @brief 定义: 用于模型匹配的样本
 */
typedef struct MatchingShapePoint {
	int id;		///< 编号
	double x, y;///< 坐标
	double z;	///< 亮度

public:
	MatchingShapePoint() {
		id = 0;
		x = y = z = 0.0;
	}

	MatchingShapePoint(int _id) {
		id = _id;
		x = y = z = 0.0;
	}

	MatchingShapePoint(int _id, double _x, double _y) {
		id = _id;
		x  = _x;
		y  = _y;
		z  =  0.0;
	}

	MatchingShapePoint(int _id, double _x, double _y, double _z) {
		id = _id;
		x  = _x;
		y  = _y;
		z  = _z;
	}

	/*!
	 * @brief 重定义操作符=, 复制对象数据
	 * @param other 待复制对象
	 * @return
	 * 实例对象
	 */
	MatchingShapePoint& operator=(const MatchingShapePoint& other) {
		if (this != &x) memcpy(this, &other, sizeof(MatchingShapePoint));
		return *this;
	}

	/*!
	 * @brief 重定义操作符-, 计算两点之间的距离
	 * @param other  被减数
	 * @return
	 * 两点之间距离
	 */
	double operator-(const MatchingShapePoint& other) {
		double dx = x - other.x;
		double dy = y - other.y;
		return sqrt(dx * dx + dy * dy);
	}

	/*!
	 * @brief 重定义操作符/, 计算两点连线相对'X'轴的倾角
	 * @param other 起点
	 * @return
	 * 倾角, 量纲: 弧度, 有效范围: \f$[-\pi, +\pi]\f$
	 */
	double operator/(const MatchingShapePoint& other) {
		double dx = x - other.x;
		double dy = y - other.y;
		return atan2(dy, dx);
	}
} PointMS;
typedef std::vector<PointMS> PtMSVec;

/*!
 * @struct MatchingShapePointSet
 * @brief 定义: 用于模型匹配的样本集合
 */
typedef struct MatchingShapePointSet {
	PtMSVec pts;

public:
	virtual ~MatchingShapePointSet() {
		pts.clear();
	}

	/*!
	 * @brief 按照数据结构中的z值升序排序
	 */
	void SortAscend() {
		std::stable_sort(pts.begin(), pts.end(), [](const PointMS& pt1, const PointMS& pt2) {
			return (pt1.z <= pt2.z);
		});
	}

	/*!
	 * @brief 按照数据中的z值降序排序
	 */
	void SortDescend() {
		std::stable_sort(pts.begin(), pts.end(), [](const PointMS& pt1, const PointMS& pt2) {
			return (pt1.z >= pt2.z);
		});
	}

	/*!
	 * @brief 清空集合内样本
	 */
	void Reset() {
		pts.clear();
	}

	/*!
	 * @brief 新增一个样本
	 * @param pt  样本
	 */
	void AddPoint(PointMS& pt) {
		pts.push_back(pt);
	}

	/*!
	 * @brief 查看集合内样本的数量
	 * @return
	 * 样本数量
	 */
	int Count() {
		return pts.size();
	}

	/*!
	 * @brief 重定义操作符(), 获得样本集合访问地址
	 * @return
	 * 样本集合实例地址
	 */
	PtMSVec& operator()() {
		return pts;
	}
} PointMSSet;

#endif

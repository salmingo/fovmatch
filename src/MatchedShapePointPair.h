/**
 * @file MatchedShapePointPair.h
 * @brief 定义: 模型匹配结果的样本对
 */

#ifndef _MATCHEDSHAPE_POINTPAIR_H_
#define _MATCHEDSHAPE_POINTPAIR_H_

#include <vector>

/*!
 * @struct MatchedShapeOptionalPointPair
 * @brief 疑似样本对集合
 */
typedef struct MatchedShapeOptionalPointPair {
	/* 数据类型 */
	struct HitPoint {
		int id;		///< 模型匹配集合2中的样本ID
		int hit;	///< 样本命中率

	public:
		HitPoint() {
			id = hit = 0;
		}

		HitPoint(int _id) {
			id  = _id;
			hit = 1;
		}

		void Inc() {
			++hit;
		}
	};
	typedef std::vector<HitPoint> PtHitVec;

	/* 成员变量 */
	int id;			///< 模型匹配集合1中样本ID
	PtHitVec pts;	///< 模型匹配集合2中样本集合

public:
	MatchedShapeOptionalPointPair() {
		id = 0;
	}

	MatchedShapeOptionalPointPair(int _id) {
		id = _id;
	}

	virtual ~MatchedShapeOptionalPointPair() {
		pts.clear();
	}

	void Reset() {
		pts.clear();
	}

	/*!
	 * @brief 标记匹配成功的集合2样本
	 * @param _id  集合2中样本ID
	 */
	void MarkHitPoint(int _id) {
		PtHitVec::iterator it;
		for (it = pts.begin(); it != pts.end() && it->id != _id; ++it);
		if (it != pts.end()) it->Inc();
		else {
			HitPoint pt(_id);
			pts.push_back(pt);
		}
	}

	/*!
	 * @brief 查找命中率最高的样本ID
	 * @param id2    样本ID
	 * @param ratio  命中率最高值与次高值的比值
	 * @return
	 * 查找结果
	 */
	bool GetHitPoint(int &id2, double &ratio) {
		int n(pts.size());
		if (!n) return false;
		int maxhit(pts[0].hit), sechit(1);

		id2 = pts[0].id;
		for (int i = 1; i < n; ++i) {
			if (maxhit < pts[i].hit) {
				sechit = maxhit;
				maxhit = pts[i].hit;
				id2    = pts[i].id;
			}
		}
		ratio = double(maxhit) / sechit;
		return true;
	}
} OptPointPairMS;
typedef std::vector<OptPointPairMS> OptPtPairMSVec;

typedef struct MatchedShapePointPair {
	int id1;	///< 模型匹配集合1中样本ID
	int id2;	///< 模型匹配集合2中样本ID. 有效范围是: [0, infinite)

public:
	MatchedShapePointPair() {
		id1 = id2 = -1;
	}

	MatchedShapePointPair(int _id) {
		id1 = _id;
		id2 = -1;
	}

	MatchedShapePointPair(int _id1, int _id2) {
		id1 = _id1;
		id2 = _id2;
	}
} PointPairMS;
typedef std::vector<PointPairMS> PtPairMSVec;

#endif

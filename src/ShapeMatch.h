/**
 * @file ShapeMatch.h
 * @brief 定义文件, 匹配基于两个坐标系建立的模型, 建立模型之间的样本对应关系
 * @version 0.1
 * @date 2020-11-02
 * @author 卢晓猛
 */

#ifndef SHAPEMATCH_H_
#define SHAPEMATCH_H_

#include "ParamMatchShape.h"
#include "MatchShape.h"
#include "MatchingShapePoint.h"
#include "MatchedShapePointPair.h"

/*!
 * @class ShapeMatch
 * @brief
 *
 */
class ShapeMatch {
public:
	ShapeMatch();
	virtual ~ShapeMatch();
};

#endif /* SHAPEMATCH_H_ */
